#include <Modules/OfcAccountStatementModule.h>
#include <ImGuiPack.h>
#include <ctools/FileHelper.h>
#include <ctools/Logger.h>
#include <sstream>
#include <string>

Cash::BankStatementModulePtr OfcAccountStatementModule::create() {
    auto res = std::make_shared<OfcAccountStatementModule>();
    if (!res->init(nullptr)) {
        res.reset();
    }
    return res;
}

Cash::AccountStatements OfcAccountStatementModule::importBankStatement(const std::string& vFilePathName) {
    Cash::AccountStatements ret;
    auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
    if (ps.isOk) {
        auto lines = ct::splitStringToVector(FileHelper::Instance()->LoadFileToString(vFilePathName), '\n');
        if (lines.empty()) {
            LogVarError("Fail, %s is empty", vFilePathName.c_str());
            return {};
        }
        if (lines.at(0).find("<OFC>") == std::string::npos) {
            LogVarError("Fail, %s is not a OFC File", vFilePathName.c_str());
            return {};
        }

        bool is_a_stmt = false;
        Cash::Transaction trans;

        // we start at 1, since 0 is the header
        for (size_t idx = 1; idx < lines.size(); ++idx) {
            auto line = lines.at(idx);
            if (line.find("<ACCTID>") != std::string::npos) {
                ct::replaceString(line, "<ACCTID>", "");
                ret.account.number = line;
            } else if (line.find("<DTSTART>") != std::string::npos) {
                ct::replaceString(line, "<DTSTART>", "");
                ret.start_date = line;
            } else if (line.find("<DTEND>") != std::string::npos) {
                ct::replaceString(line, "<DTEND>", "");
                ret.end_date = line;
            } else if (line.find("<LEDGER>") != std::string::npos) {
                ct::replaceString(line, "<LEDGER>", "");
                ret.ledger = ct::dvariant(line).GetD();
            } else if (line.find("<STMTTRN>") != std::string::npos) {
                is_a_stmt = true;
                trans = {};
            } else if (line.find("</STMTTRN>") != std::string::npos) {
                if (is_a_stmt) {
                    ret.statements.push_back(trans);
                    is_a_stmt = false;
                }
            }
            if (is_a_stmt) {
                if (line.find("<DTPOSTED>") != std::string::npos) {
                    ct::replaceString(line, "<DTPOSTED>", "");
                    trans.date = line;
                    if (trans.date.size() == 8U) {
                        trans.date.insert(6, "-");
                        trans.date.insert(4, "-");
                    }
                } else if (line.find("<TRNAMT>") != std::string::npos) {
                    ct::replaceString(line, "<TRNAMT>", "");
                    trans.amount = ct::dvariant(line).GetD();
                } else if (line.find("<NAME>") != std::string::npos) {
                    ct::replaceString(line, "<NAME>", "");
                    trans.label = line;
                    const auto& first_not_space = trans.label.find_first_not_of(' ');
                    if (first_not_space != std::string::npos) {
                        const auto& space_pos = trans.label.find(' ', first_not_space);
                        if (space_pos != std::string::npos) {
                            trans.operation = trans.label.substr(first_not_space, space_pos - first_not_space);
                        }
                    }
                } else if (line.find("<MEMO>") != std::string::npos) {
                    ct::replaceString(line, "<MEMO>", "");
                    trans.comment = line;
                } else if (line.find("<FITID>") != std::string::npos) {
                    ct::replaceString(line, "<FITID>", "");
                    trans.hash = line;
                } else if (line.find("<CHKNUM>") != std::string::npos) {
                    ct::replaceString(line, "<CHKNUM>", "");
                    trans.operation = "CHEQUE";
                    trans.label = "CHEQUE " + line;
                }
            }
            
        }
    }
    return ret;
}
