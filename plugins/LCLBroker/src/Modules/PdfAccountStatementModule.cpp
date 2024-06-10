#include <Modules/PdfAccountStatementModule.h>
#include <ImGuiPack.h>
#include <ctools/FileHelper.h>
#include <ctools/Logger.h>

#include <memory>
#include <vector>
#include <array>
#include <map>

#include <xpdf/goo/gmem.h>
#include <xpdf/goo/gmempp.h>
#include <xpdf/goo/parseargs.h>
#include <xpdf/goo/GString.h>
#include <xpdf/goo/GList.h>
#include <xpdf/xpdf/GlobalParams.h>
#include <xpdf/xpdf/Object.h>
#include <xpdf/xpdf/Stream.h>
#include <xpdf/xpdf/Array.h>
#include <xpdf/xpdf/Dict.h>
#include <xpdf/xpdf/XRef.h>
#include <xpdf/xpdf/Catalog.h>
#include <xpdf/xpdf/Page.h>
#include <xpdf/xpdf/PDFDoc.h>
#include <xpdf/xpdf/OutputDev.h>
#include <xpdf/xpdf/CharTypes.h>
#include <xpdf/xpdf/UnicodeMap.h>
#include <xpdf/xpdf/TextString.h>
#include <xpdf/xpdf/Error.h>
#include <xpdf/xpdf/config.h>
#include <xpdf/xpdf/GfxState.h>

// we can check the file pdftotext with -table option
// xpdf/xpdf/pdftotext.c
// xpdf/xpdf/TextOutputDev.cc

struct Token {
    std::string token;
    double px = 0.0;
    double py = 0.0;
    double sx = 0.0;
    double sy = 0.0;
    void print() {
        LogVarDebugInfo("p(%f,%f) s(%f,%f) => \"%s\"", px, py, sx, sy, token.c_str());
    }
    void merge(const Token &vToken) {
        #if 0
        if ((!token.empty()) && (token.back() != ' ')) {
            token += ' ';
        }
        token += vToken.token;
        #else
        if (token.empty()) {
            token = vToken.token;
        }
        #endif
    }
};

typedef int32_t PageIdx, PosX, PosY, SizeX, SizeY;
struct Fields {
    bool isHeader = false;
    std::vector<Token> fields;
    std::vector<double> hxs;  // the x of each header column
};

typedef std::vector<Fields> Table;
typedef std::map<PageIdx, std::map<PosY, std::map<PosX, Token>>> TokenContainer;

class TableExtractor : public OutputDev {
private:
    TokenContainer m_Tokens;
    bool m_IsFirstTokenChar = false;
    Token m_TempToken;
    PageIdx m_CurrentPageIdx = 0;

public:
    TableExtractor() = default;
    ~TableExtractor() = default;

    const TokenContainer &getTokenContainer() const {
        return m_Tokens;
    }

    //----- get info about output device

    // Does this device use upside-down coordinates?
    // (Upside-down means (0,0) is the top left corner of the page.)
    GBool upsideDown() override {
        return true;
    }

    // Does this device use drawChar() or drawString()?
    GBool useDrawChar() override {
        return true;
    }

    // Does this device use beginType3Char/endType3Char?  Otherwise,
    // text in Type 3 fonts will be drawn with drawChar/drawString.
    GBool interpretType3Chars() override {
        return true;
    }

    void endPage() override {
      //  LogVarDebugInfo("endPage()");
        ++m_CurrentPageIdx;
    }

    void beginString(GfxState *state, GString *s) override {
        m_TempToken = {};
        m_IsFirstTokenChar = true;
    }

    void endString(GfxState *state) override {
        m_TempToken.px = std::abs(m_TempToken.px);
        m_TempToken.py = std::abs(m_TempToken.py);
        m_TempToken.sx = std::abs(m_TempToken.sx);
        m_TempToken.sy = std::abs(m_TempToken.sy);
        int32_t ipy = static_cast<int32_t>(m_TempToken.py);
        int32_t ipx = static_cast<int32_t>(m_TempToken.px);
        m_Tokens[m_CurrentPageIdx][ipy][ipx] = m_TempToken;
        //m_TempToken.print();
    }

    void drawChar(GfxState *state,
                  double x,
                  double y,
                  double dx,
                  double dy,
                  double originX,
                  double originY,
                  CharCode c,
                  int nBytes,
                  Unicode *u,
                  int uLen,
                  GBool fill,
                  GBool stroke,
                  GBool makePath) override {
        if (m_IsFirstTokenChar) {
            m_TempToken.px = x + originX;
            m_TempToken.py = y + originY;
            m_IsFirstTokenChar = false;
        }
        m_TempToken.sx = x + dx - m_TempToken.px;
        m_TempToken.sy = y + dy - m_TempToken.py;
        m_TempToken.token += c;
    }

public:
};

class TableSolver {
private:
    typedef std::array<Token, 5> StatementFields;
    typedef std::array<int32_t, 5> ColumnSizes;
    typedef std::vector<StatementFields> StatementRows;
    ColumnSizes m_ColSizes = {};
    Fields m_RibHeader;
    Fields m_RibDatas;
    std::string m_StartDate;
    std::string m_EndDate;
    std::string m_Ledger;
    std::string m_DocNumber;
    Token m_StartSoldeToken;
    Token m_EndSoldeToken;
    double m_StartSolde = 0.0;
    double m_EndSolde = 0.0;

public:
    Cash::AccountStatements compute(const TokenContainer &vContainer, const std::string &vSourceName) {
        const auto &tbl = fillTable(vContainer);
        const auto &stms = solveTableColumns(tbl);
#if _DEBUG
        //printTable(stms);
#endif
        return extractStatements(stms, vSourceName);
    }

private:
    Table fillTable(const TokenContainer &vContainer) {
        Table ret;
        bool rib_started = false;
        Fields fields;
        bool date_range_found = false;
        bool tabled_started = false;
        bool doc_end = false;
        bool can_save_row = false;
        const auto &pages_count = (int32_t)vContainer.size();
        for (const auto &page : vContainer) {
            const auto &page_idx = page.first;
            const auto &page_footer = ct::toStr("  Page %u / %u", page_idx + 1, pages_count);
            for (const auto &row : page.second) {
                const auto &row_idx = row.first;
                const auto &cols = row.second;
                fields = {};
                for (const auto &col : cols) {
                    fields.fields.push_back(col.second);
                }
                can_save_row = true;
                if (rib_started) {
                    // bug du fichier pdf de LCL
                    // le numerod e compte est en deux fois.
                    // le 3er token est 000 et le 4 le reste du numero
                    if (fields.fields.size() == 5U && m_RibHeader.fields.size() == 4U) {
                        m_RibDatas = {};
                        m_RibDatas.fields.push_back(fields.fields.at(0));
                        m_RibDatas.fields.push_back(fields.fields.at(1));
                        auto tk0 = fields.fields.at(2);
                        auto tk1 = fields.fields.at(3);
                        tk0.token += tk1.token;
                        tk0.sx += tk1.sx;
                        m_RibDatas.fields.push_back(tk0);
                        m_RibDatas.fields.push_back(fields.fields.at(4));
                    } else {
                        m_RibDatas = fields;
                    }
                    rib_started = false;
                }
                if (fields.fields.size() == 5U) {
                    if (fields.fields.at(0).token == "DATE" &&     //
                        fields.fields.at(1).token == "LIBELLE" &&  //
                        fields.fields.at(2).token == "VALEUR" &&   //
                        fields.fields.at(3).token == "DEBIT" &&    //
                        fields.fields.at(4).token == "CREDIT") {
                        fields.isHeader = true;
                        tabled_started = true;  // start of a table
                        fields.hxs.push_back(0.0);
                        for (const auto &f : fields.fields) {
                            fields.hxs.push_back(f.px + f.sx);  // x coord of column separator
                        }
                    }
                } else if (fields.fields.size() == 1U) {
                    if (tabled_started) {
                        if (fields.fields.at(0).token == page_footer) {
                            tabled_started = false;  // end of a table
                        }
                    } else if (!date_range_found) {
                        const auto &tmp = fields.fields.at(0).token;
                        auto du_pos = tmp.find(" du ");
                        auto au_pos = tmp.find(" au ");
                        auto n_pos = tmp.find(" -  N° ");
                        if (du_pos != std::string::npos &&  //
                            au_pos != std::string::npos &&  //
                            n_pos != std::string::npos) {
                            // we have found the date range
                            du_pos += 4;
                            m_StartDate = tmp.substr(du_pos, au_pos - du_pos);
                            au_pos += 4;
                            m_EndDate = tmp.substr(au_pos, n_pos - au_pos);
                            n_pos += 7;
                            m_DocNumber = tmp.substr(n_pos);
                            ct::replaceString(m_DocNumber, " ", "");
                            date_range_found = true;
                        }
                    }
                } else if (fields.fields.size() == 2U) {
                    if (!tabled_started) {
                        if (fields.fields.at(0).token == "Indicatif" &&  //
                            fields.fields.at(1).token == "N° de compte") {
                            m_RibHeader = fields;
                            rib_started = true;
                        }
                    }
                } else if (fields.fields.size() == 3U) {
                    if (tabled_started) {
                        if (fields.fields.at(1).token.find("ANCIEN SOLDE") != std::string::npos) {
                            // ANCIEN SOLDE => on saute la ligne
                            can_save_row = false;  // end of a table
                            m_StartSoldeToken = fields.fields.at(2);
                        } else if( 
                            fields.fields.at(0).token.find("TOTAUX") != std::string::npos) {
                            // TOTAUX => on saute la ligne
                            can_save_row = false;  // end of a table
                        } else if (fields.fields.at(1).token.find("SOLDE EN EUROS") != std::string::npos) {
                            // fin du fichier
                            m_Ledger = fields.fields.at(2).token;
                            tabled_started = false;  // end of a table
                            m_EndSoldeToken = fields.fields.at(2);
                            return ret;
                        }
                    }
                } else if (fields.fields.size() == 4U) {
                    if (!tabled_started) {
                        if (fields.fields.at(0).token == "Banque" &&     //
                            fields.fields.at(1).token == "Indicatif" &&  //
                            fields.fields.at(2).token == "N° de compte" &&  //
                            fields.fields.at(3).token == "Clé") {
                            m_RibHeader = fields;
                            rib_started = true;
                        }
                    }
                }
                if (can_save_row && tabled_started && !doc_end) {
                    ret.push_back(fields);
                }
            }
        }
        return ret;
    }
    int32_t getColumnOfField(const Token &vToken, const std::vector<double> vColumnPoses) {
        // we need to find the column
        // from end to start, for find the last column
        int32_t c = static_cast<int32_t>(vColumnPoses.size()) - 1;
        for (; c >= 0; --c) {
            if (vToken.px > vColumnPoses.at(c)) {
                break;
            }
        }
        if (c < 0) {
            CTOOL_DEBUG_BREAK;
        }
        return c;
    }
    StatementRows solveTableColumns(const Table &vTable) {
        StatementRows ret;
        if (!vTable.empty()) {
            bool headerAlreadyAdded = false;
            Fields currentHeader;
            for (const auto &row : vTable) {
                /*if (row.fields.size() > 1 && row.fields.at(1).token == "CB RETRAIT DU 10/01") {
                    CTOOL_DEBUG_BREAK;
                }*/
                if (row.isHeader) {
                    currentHeader = row;
                } 
                StatementFields f;
                for (const auto &col : row.fields) {
                    const auto &c = getColumnOfField(col, currentHeader.hxs);
                    if (c < m_ColSizes.size()) {
                        f.at(c).merge(col);
                        // for table printing
                        if (col.token.size() > m_ColSizes.at(c)) {
                            if (col.token.size() > 100) {
                                CTOOL_DEBUG_BREAK;
                            }
                            m_ColSizes.at(c) = static_cast<int32_t>(col.token.size());  // the sx is the new max width of the column
                        }
                    }
                }
                if (row.isHeader) { // add only one header
                    if (!headerAlreadyAdded) {
                        ret.push_back(f);
                        headerAlreadyAdded = true;
                    }
                } else {
                    ret.push_back(f);
                }
            }
            {  // start solde
                const auto &c = getColumnOfField(m_StartSoldeToken, currentHeader.hxs);
                ct::replaceString(m_StartSoldeToken.token, " ", "");
                ct::replaceString(m_StartSoldeToken.token, ",", ".");
                if (c == 3) {
                    m_StartSoldeToken.token = "-" + m_StartSoldeToken.token;
                } 
            }
            {  // end solde
                const auto &c = getColumnOfField(m_EndSoldeToken, currentHeader.hxs);
                ct::replaceString(m_EndSoldeToken.token, " ", "");
                ct::replaceString(m_EndSoldeToken.token, ",", ".");
                if (c == 3) {
                    m_EndSoldeToken.token = "-" + m_EndSoldeToken.token;
                }
            }
        }
        return ret;
    }
    void printTable(const StatementRows &vStms) {
        LogVarDebugInfo("%s", "=========== INFOS ===================");
        LogVarDebugInfo("Start Date : %s", m_StartDate.c_str());
        LogVarDebugInfo("End Date : %s", m_EndDate.c_str());
        LogVarDebugInfo("Ledger : %s", m_Ledger.c_str());
        LogVarDebugInfo("%s", "============ RIB ====================");
        assert(m_RibHeader.fields.size() == m_RibDatas.fields.size());
        for (size_t idx = 0; idx < m_RibHeader.fields.size(); ++idx) {
            LogVarDebugInfo("%s : %s", m_RibHeader.fields.at(idx).token.c_str(), m_RibDatas.fields.at(idx).token.c_str());
        }
        LogVarDebugInfo("%s", "========= STATEMENT =================");
        bool isHeader = true;
        std::string h_line;
        std::string row_str;
        bool is_new_line = false;
        bool is_last_line_the_header = true;
        for (const auto &row : vStms) {
            row_str.clear();
            is_new_line = false;
            for (size_t idx = 0; idx < 5; ++idx) {
                const auto &tk = row.at(idx);
                const auto &colSize = m_ColSizes.at(idx);
                const auto w = colSize - tk.token.size();
                if (w < 0) {
                    CTOOL_DEBUG_BREAK;
                }
                if (idx == 0) {
                    if (!tk.token.empty()) {
                        is_new_line = true;
                    }
                }
                row_str += " | " + tk.token + std::string(w, ' ');
            }
            row_str += " | ";
            if (isHeader) {
                h_line = std::string(row_str.size(), '=');
                LogVarDebugInfo("%s", h_line.c_str());
            } else if (is_new_line && !is_last_line_the_header) {
                LogVarDebugInfo("%s", std::string(row_str.size(), '-').c_str());
            }        
            LogVarDebugInfo("%s", row_str.c_str());
            if (isHeader) {
                LogVarDebugInfo("%s", h_line.c_str());
                isHeader = false;
            } else {
                is_last_line_the_header = false;
            }
        }
        LogVarDebugInfo("%s", h_line.c_str());
    }
    Cash::AccountStatements extractStatements(const StatementRows &vStms, const std::string &vSourceName) {
        Cash::AccountStatements ret;

        struct TransDoublon {
            uint32_t doublons = 1U;
            Cash::Transaction trans;
        };
        std::map<std::string, TransDoublon> transactions;

        {  // dates
            ret.start_date = m_StartDate;
            ret.end_date = m_EndDate;
        }

        {  // ledger
            ret.ledger = ct::dvariant(m_Ledger).GetD();
        }

        {  // account infos
            std::string bank_id;
            std::string guichet;
            std::string number;
            if (m_RibDatas.fields.size() == 4U) {
                // RIB // Compte Depot
                bank_id = m_RibDatas.fields.at(0).token;
                guichet = m_RibDatas.fields.at(1).token;
                number = m_RibDatas.fields.at(2).token;
            } else if (m_RibDatas.fields.size() == 2U) {
                // NO RIB // Compte Epargne
                guichet = m_RibDatas.fields.at(0).token;
                number = m_RibDatas.fields.at(1).token;
            } else {
                LogVarError("Fail, No Bank Infos found");
                return {};
            }

            // remove spaces, because some numbers can be XXXXXX X
            ct::replaceString(guichet, " ", "");
            ct::replaceString(number, " ", "");

            // cut number like 000000XXXXXXX to XXXXXXX
            if (number.size() > 7U) {
                number = number.substr(number.size() - 7U);
            }

            ret.account.bank_id = bank_id;
            ret.account.number = guichet + " " + number;
        }

        {  // statements
            TransDoublon trans;

            bool is_new_line = false;
            m_StartSolde = ct::dvariant(m_StartSoldeToken.token).GetD();
            m_EndSolde = ct::dvariant(m_EndSoldeToken.token).GetD();
            double solde = m_StartSolde;
            double debit = 0.0, credit = 0.0;
            // 0 is the header, so we jump it
            for (size_t row_idx = 1; row_idx < vStms.size(); ++row_idx) {
                const auto &row = vStms.at(row_idx);
                is_new_line = false;
                for (size_t idx = 0; idx < 5; ++idx) {
                    const auto &tk = row.at(idx);
                    if (idx == 0) {
                        if (!tk.token.empty()) {
                            trans = {};
                            is_new_line = true;
                            trans.trans.date = tk.token;
                            auto arr = ct::splitStringToVector(trans.trans.date, ".");
                            if (arr.size() == 2U) {
                                trans.trans.date = arr.at(1) + "-" + arr.at(0);
                            } else {
                                CTOOL_DEBUG_BREAK;
                            }
                        }
                    } else if (idx == 1) {
                        if (is_new_line) {
                            parseDescription(tk.token, trans.trans.entity, trans.trans.operation, trans.trans.description);
                        } else {
                            trans.trans.comment += tk.token;
                        }
                    } else if (idx == 2) {
                        if (is_new_line) {
                            auto arr = ct::splitStringToVector(tk.token, ".");
                            if (arr.size() == 3U) {
                                trans.trans.date = "20" + arr.at(2) + "-" + trans.trans.date;
                            } else {
                                CTOOL_DEBUG_BREAK;
                            }
                        }
                    } else if (idx == 3) {
                        if (is_new_line) {
                            auto tok = tk.token;
                            ct::replaceString(tok, " ", "");
                            ct::replaceString(tok, ",", ".");
                            debit = ct::dvariant(tok).GetD();
                            if (debit > 0.0) {
                                trans.trans.amount = debit * -1.0;
                                solde += trans.trans.amount;
                            }
                        }
                    } else if (idx == 4) {
                        if (is_new_line) {
                            auto tok = tk.token;
                            ct::replaceString(tok, " ", "");
                            ct::replaceString(tok, ",", ".");
                            credit = ct::dvariant(tok).GetD();
                            if (credit > 0.0) {
                                trans.trans.amount = credit;
                                solde += trans.trans.amount;
                            }
                        }
                    }
                }
                if (is_new_line) {
                    trans.trans.source = vSourceName;
                    trans.trans.source_type = "pdf";
                    trans.trans.hash = ct::toStr("%s_%s_%f",  //
                                                 trans.trans.date.c_str(),
                                                 // un fichier ofc ne peut pas avoir des description de longueur > a 30
                                                 // alors on limite le hash a utiliser un description de 30
                                                 // comme cela un ofc ne rentrera pas un collision avec un autre type de fcihier comme les pdf par ex
                                                 trans.trans.description.substr(0, 30).c_str(),
                                                 trans.trans.amount);  // must be unique per oepration
                    if (transactions.find(trans.trans.hash) != transactions.end()) {
                        ++transactions.at(trans.trans.hash).doublons;
                    } else { 
                        transactions[trans.trans.hash] = trans;
                    }                    
                    trans = {};
                }
            }

            const auto &compute_solde_str = ct::round_n(solde, 2);
            const auto &final_solde_str = ct::round_n(m_EndSolde, 2);
            if (compute_solde_str != final_solde_str) {
                LogVarDebugError("Fail, the computed solde of %s not match the end solde of the file.some lines are badly parsed maybe",
                                 compute_solde_str.c_str(),
                                 final_solde_str.c_str());
            }

            for (const auto &t : transactions) {
                auto trans = t.second;
                for (uint32_t idx = 0U; idx < trans.doublons; ++idx) {
                    trans.trans.hash = t.second.trans.hash + ct::toStr("_%u", idx);
                    ret.statements.push_back(trans.trans);
                }
            }
        }

        return ret;
    }
};

Cash::BankStatementModulePtr PdfAccountStatementModule::create() {
    auto res = std::make_shared<PdfAccountStatementModule>();
    if (!res->init(nullptr)) {
        res.reset();
    }
    return res;
}

Cash::AccountStatements PdfAccountStatementModule::importBankStatement(const std::string& vFilePathName) {
    Cash::AccountStatements ret{};
    auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
    if (ps.isOk) {
        globalParams = new GlobalParams(nullptr);                                             // globalParams ios a extern and is used by pdf_doc_ptr...
        auto pdf_doc_ptr = new PDFDoc(new GString(vFilePathName.c_str()), nullptr, nullptr); // the GString is deleted by PDFDoc
        if (pdf_doc_ptr != nullptr) {
            if (!pdf_doc_ptr->isOk()) {
                LogVarError("Fail, cant read the pdf %s", vFilePathName.c_str());
            } else {
                auto pages_count = pdf_doc_ptr->getNumPages();
                if (pages_count == 0) {
                    LogVarError("Fail, no pages are available in the pdf %s", vFilePathName.c_str());
                } else {
                    //LogVarDebugInfo("LCl Table Extraction for %s", vFilePathName.c_str());

                    TableExtractor tbl;
                    pdf_doc_ptr->displayPages(&tbl, 1, pages_count, 72, 72, 0, gFalse, gTrue, gFalse);

                    TableSolver ts;
                    ret = ts.compute(tbl.getTokenContainer(), ps.GetFPNE_WithPath(""));
                }
            }
        }
        delete pdf_doc_ptr;
        delete globalParams;
    }
    return ret;
}
