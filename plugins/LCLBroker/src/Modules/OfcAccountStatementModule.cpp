#include <Modules/OfcAccountStatementModule.h>

#include <sstream>
#include <string>

#include <ezlibs/ezFile.hpp>
#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezSha.hpp>

#include <ImGuiPack.h>

#include <Utils/Utils.h>

Cash::BankStatementModulePtr OfcAccountStatementModule::create() {
    auto res = std::make_shared<OfcAccountStatementModule>();
    if (!res->init(nullptr)) {
        res.reset();
    }
    return res;
}
std::string OfcAccountStatementModule::getFileExt() const {
    return "Bank Account files (*.ofc *.ofx){.ofc,.ofx}";
}

Cash::AccountStatements OfcAccountStatementModule::importBankStatement(const std::string& vFilePathName) {
    Cash::AccountStatements ret;

    struct TransDoublon {
        uint32_t doublons = 1U;
        Cash::Transaction trans;
    };
    std::map<std::string, TransDoublon> transactions;

    auto ps = ez::file::parsePathFileName(vFilePathName);
    if (ps.isOk) {
        auto lines = ez::str::splitStringToVector(ez::file::loadFileToString(vFilePathName), '\n');
        if (lines.empty()) {
            LogVarError("Fail, %s is empty", vFilePathName.c_str());
            return {};
        }

        m_fileType = FileType::NONE;

        if (lines.at(0).find("<OFC>") != std::string::npos) {
            m_fileType = FileType::OFC;
        } else if (lines.at(0).find("application/x-ofx") != std::string::npos) {
            m_fileType = FileType::OFX;
        }else {
            LogVarError("Fail, %s is not a OFC or OFX File", vFilePathName.c_str());
            return {};
        }

        // Account detections
        // OFC give
        // <ACCTID> 000XX 999999X
        // OFX give :
        // <BRANCHID> 000XX
        // <ACCTID> 999999X

        bool is_a_stmt = false;
        TransDoublon trans;
        // we start at 1, since 0 is the header
        for (size_t idx = 1; idx < lines.size(); ++idx) {
            auto line = lines.at(idx);
            if (line.find("<BANKID>") != std::string::npos) {
                ez::str::replaceString(line, "<BANKID>", "");
                ret.account.bank_id = line;
            } else if (line.find("<BRANCHID>") != std::string::npos) {
                ez::str::replaceString(line, "<BRANCHID>", "");
                ret.account.number = line;
            } else if (line.find("<ACCTID>") != std::string::npos) {
                ez::str::replaceString(line, "<ACCTID>", "");
                if (m_fileType == FileType::OFC) { // OFC
                    ret.account.number = line;
                } else { // OFX
                    ret.account.number.append(" ").append(line);
                }
            } else if (line.find("<DTSTART>") != std::string::npos) {
                ez::str::replaceString(line, "<DTSTART>", "");
                ret.start_date = line;
            } else if (line.find("<DTEND>") != std::string::npos) {
                ez::str::replaceString(line, "<DTEND>", "");
                ret.end_date = line;
            } else if (line.find("<LEDGER>") != std::string::npos) {
                ez::str::replaceString(line, "<LEDGER>", "");
                ret.ledger = ez::dvariant(line).GetD();
            } else if (line.find("<STMTTRN>") != std::string::npos) {
                is_a_stmt = true;
                trans = {};
            } else if (line.find("</STMTTRN>") != std::string::npos) {
                if (is_a_stmt) {
                    trans.trans.source = ps.GetFPNE_WithPath("");
                    if (m_fileType == FileType::OFC) {
                        trans.trans.source_type = "ofc";
                    } else if (m_fileType == FileType::OFX) {
                        trans.trans.source_type = "ofx";
                    }
                    trans.trans.source_sha1 =         //
                        ez::sha1()                    //
                            .add(trans.trans.source)  //
                            .add(trans.trans.source_type)
                            .finalize()
                            .getHex();
                    trans.trans.hash =  //
                        ez::sha1()
                            .add(trans.trans.date)
                            // un fichier ofc ne peut pas avoir des description de longueur > a 30
                            // alors on limite le hash a utiliser un description de 30
                            // comme cela un ofc ne rentrera pas un collision avec un autre type de fcihier comme les pdf par ex
                            .add(trans.trans.description.substr(0, 30))
                            // must be unique per oepration
                            .addValue(trans.trans.amount)
                            .finalize()
                            .getHex();
                    if (transactions.find(trans.trans.hash) != transactions.end()) {
                        ++transactions.at(trans.trans.hash).doublons;
                    } else {
                        transactions[trans.trans.hash] = trans;
                    }
                    trans = {};
                    is_a_stmt = false;
                }
            }
            if (is_a_stmt) {
                if (line.find("<DTPOSTED>") != std::string::npos) {
                    ez::str::replaceString(line, "<DTPOSTED>", "");
                    trans.trans.date = line;
                    if (trans.trans.date.size() == 8U) {
                        trans.trans.date.insert(6, "-");
                        trans.trans.date.insert(4, "-");
                    }
                } else if (line.find("<TRNAMT>") != std::string::npos) {
                    ez::str::replaceString(line, "<TRNAMT>", "");
                    trans.trans.amount = ez::dvariant(line).GetD();
                } else if (line.find("<NAME>") != std::string::npos) {
                    ez::str::replaceString(line, "<NAME>", "");
                    parseDescription(line, trans.trans.entity, trans.trans.operation, trans.trans.description);
                } else if (line.find("<MEMO>") != std::string::npos) {
                    ez::str::replaceString(line, "<MEMO>", "");
                    trans.trans.comment = line;
                } else if (line.find("<FITID>") != std::string::npos) {
                    ez::str::replaceString(line, "<FITID>", "");
                    //trans.hash = line;
                } else if (line.find("<CHKNUM>") != std::string::npos) {
                    ez::str::replaceString(line, "<CHKNUM>", "");
                    trans.trans.operation = "CHEQUE";
                    trans.trans.description = "CHEQUE " + line;
                    trans.trans.entity = "LCL";
                }
            }            
        }

        for (const auto& t : transactions) {
            auto trans = t.second;
            for (uint32_t idx = 0U; idx < trans.doublons; ++idx) {
                trans.trans.hash = t.second.trans.hash + ez::str::toStr("_%u", idx);
                ret.statements.push_back(trans.trans);
            }
        }
    }

    return ret;
}
