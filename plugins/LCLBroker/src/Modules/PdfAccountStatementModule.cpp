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

    const TokenContainer const &getTokenContainer() const {
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

public:
    void compute(const TokenContainer &vContainer) {
        const auto &tbl = fillTable(vContainer);
        const auto &stms = solveTableColumns(tbl);
        printTable(stms);
    }

private:
    Table fillTable(const TokenContainer &vContainer) {
        Table ret;
        Fields fields;
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
                    }
                } else if (fields.fields.size() == 2U) {
                    if (tabled_started) {
                        // SOLDE INTERMEDIAIRE ou SOLDE EN EUROS pour la fin
                        // du doc en evitant les tables de fin de fichiers
                        if (fields.fields.at(0).token.find("SOLDE INTERMEDIAIRE") != std::string::npos) {
                            tabled_started = false;  // end of a table
                            return ret;
                        }
                    }
                } else if (fields.fields.size() == 3U) {
                    if (tabled_started) {
                        // ANCIEN SOLDE => on saute la ligne
                        if (fields.fields.at(1).token.find("ANCIEN SOLDE") != std::string::npos) {
                            can_save_row = false;  // end of a table
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
    StatementRows solveTableColumns(const Table &vTable) {
        StatementRows ret;
        if (!vTable.empty()) {
            bool headerAlreadyAdded = false;
            Fields currentHeader;
            for (const auto &row : vTable) {
                if (row.isHeader) {
                    currentHeader = row;
                } 
                StatementFields f;
                for (const auto &col : row.fields) {
                    // we need to find the column
                    // from end to styart, for find the last column
                    int32_t c = currentHeader.hxs.size() - 1;
                    for (; c >= 0; --c) {
                        if (col.px > currentHeader.hxs.at(c)) {
                            break;
                        }
                    }
                    if (c < 0) {
                        CTOOL_DEBUG_BREAK;
                    }
                    if (c < m_ColSizes.size()) {
                        f[c] = col;
                        // for table printing
                        if (col.token.size() > m_ColSizes.at(c)) {
                            if (col.token.size() > 100) {
                                CTOOL_DEBUG_BREAK;
                            }
                            m_ColSizes[c] = col.token.size();  // the sx is the new max width of the column
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
        }
        return ret;
    }
    void printTable(const StatementRows &vStms) {
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
        auto pdf_doc_ptr = new PDFDoc(new GString(vFilePathName.c_str()), nullptr, nullptr); // the GString is deleted by PDFDoc
        if (pdf_doc_ptr != nullptr) {
            if (!pdf_doc_ptr->isOk()) {
                LogVarError("Fail, cant read the pdf %s", vFilePathName.c_str());
            } else {
                auto pages_count = (size_t)pdf_doc_ptr->getNumPages();
                if (pages_count == 0) {
                    LogVarError("Fail, no pages are available in the pdf %s", vFilePathName.c_str());
                } else {
                    globalParams = new GlobalParams(nullptr);  // globalParams ios a extern and is used by pdf_doc_ptr...

                    LogVarDebugInfo("LCl Table Extraction for %s", vFilePathName.c_str());

                    TableExtractor tbl;
                    pdf_doc_ptr->displayPages(&tbl, 1, pages_count, 72, 72, 0, gFalse, gTrue, gFalse);

                    TableSolver ts;
                    ts.compute(tbl.getTokenContainer());

                    delete globalParams;
                }
            }
        }
        delete pdf_doc_ptr;
    }
    return ret;
}
