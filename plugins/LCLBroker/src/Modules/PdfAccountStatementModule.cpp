#include <Modules/PdfAccountStatementModule.h>
#include <ImGuiPack.h>
#include <ctools/FileHelper.h>
#include <ctools/Logger.h>

#include <memory>
#include <vector>
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

typedef int32_t PageIdx, PosX, PosY;
struct Field {
    PosX px;
    PosY py;
    std::string field;
};
typedef std::vector<Field> Fields;
typedef std::vector<Fields> Table;

class TableExtractor : public OutputDev {
private:
    typedef std::map<PageIdx, std::map<PosY, std::map<PosX, std::string>>> TokenContainer;
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

    void startPage(int pageNum, GfxState *state) override {
      //  LogVarDebugInfo("startPage(%i)", pageNum);
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
        int32_t ipy = std::abs((int32_t)m_TempToken.py);
        int32_t ipx = std::abs((int32_t)m_TempToken.px);
        m_Tokens[m_CurrentPageIdx][ipy][ipx] = m_TempToken.token;
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

    void drawString(GfxState *state, GString *s, GBool fill, GBool stroke, GBool makePath) override {
       // LogVarDebugInfo("drawString(\"%s\")", s->getCString());
    }

    void incCharCount(int nChars) override {
      //  LogVarDebugInfo("incCharCount(%i)", nChars);
    }

    void beginActualText(GfxState *state, Unicode *u, int uLen) override {
     //   LogVarDebugInfo("beginActualText(%u,%i,%i)", (uintptr_t)state, (u != nullptr ? *u : -1), uLen);
    }

    void endActualText(GfxState *state) override {
     //   LogVarDebugInfo("endActualText(%u)", (uintptr_t)state);
    }

    void stroke(GfxState *state) override  {
        GfxPath *path;
        GfxSubpath *subpath;
        double x[2], y[2], t;
        path = state->getPath();
        if (path->getNumSubpaths() != 1) {
            return;
        }
        subpath = path->getSubpath(0);
        if (subpath->getNumPoints() != 2) {
            return;
        }
        state->transform(subpath->getX(0), subpath->getY(0), &x[0], &y[0]);
        state->transform(subpath->getX(1), subpath->getY(1), &x[1], &y[1]);
        // look for a vertical or horizontal line
        if (x[0] == x[1] || y[0] == y[1]) {
            if (x[0] > x[1]) {
                t = x[0];
                x[0] = x[1];
                x[1] = t;
            }
            if (y[0] > y[1]) {
                t = y[0];
                y[0] = y[1];
                y[1] = t;
            }
            if (x[0] == x[1]) {
                LogVarDebugInfo("stroke Vertical : %f,%f,%f,%f", x[0], y[0], x[1], y[1]);
            }
            if (y[0] == y[1]) {
                LogVarDebugInfo("stroke Horizontal : %f,%f,%f,%f", x[0], y[0], x[1], y[1]);
            }
        } else {
        
        }
        LogVarDebugInfo("stroke : %f,%f,%f,%f", x[0], y[0], x[1], y[1]);
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
        auto pdf_doc_ptr = new PDFDoc(new GString(vFilePathName.c_str()), nullptr, nullptr); // the GString is delete by PDFDoc
        if (pdf_doc_ptr != nullptr) {
            if (!pdf_doc_ptr->isOk()) {
                LogVarError("Fail, cant read the pdf %s", vFilePathName.c_str());
            } else {
                auto pages_count = (size_t)pdf_doc_ptr->getNumPages();
                if (pages_count == 0) {
                    LogVarError("Fail, no pages are available in the pdf %s", vFilePathName.c_str());
                } else {
                    TableExtractor tbl;
                    globalParams = new GlobalParams(nullptr); // globalParams ios a extern and is used by pdf_doc_ptr...
                    pdf_doc_ptr->displayPages(&tbl, 1, pages_count, 72, 72, 0, gFalse, gTrue, gFalse);
                    delete globalParams;

                    // fill Table
                    Table _table;
                    {
                        Fields fields;
                        Field f;
                        bool tabled_started = false;
                        const auto &tokens = tbl.getTokenContainer();
                        pages_count = (int32_t)tokens.size();
                        for (const auto &page : tokens) {
                            const auto &page_idx = page.first;
                            const auto &page_footer = ct::toStr("  Page %u / %u", page_idx + 1, pages_count);
                            for (const auto &row : page.second) {
                                const auto &row_idx = row.first;
                                const auto &cols = row.second;
                                fields = {};
                                for (const auto &col : cols) {
                                    f = {};
                                    f.px = col.first;
                                    f.py = row.first;
                                    f.field = col.second;
                                    fields.push_back(f);
                                }
                                if (fields.size() == 5U) {
                                    if (fields.at(0).field == "DATE" &&     //
                                        fields.at(1).field == "LIBELLE" &&  //
                                        fields.at(2).field == "VALEUR" &&   //
                                        fields.at(3).field == "DEBIT" &&    //
                                        fields.at(4).field == "CREDIT") {
                                        tabled_started = true;
                                    }
                                } else if (fields.size() == 1U) {
                                    if (tabled_started) {
                                        if (fields.at(0).field == page_footer) {
                                            tabled_started = false;
                                        }
                                    }
                                }
                                if (tabled_started) {
                                    _table.push_back(fields);
                                }
                            }
                        }
                    }

                    // print table
                    {
                        for (const auto &row : _table) {
                            std::string row_str;
                            for (const auto &col : row) {
                                row_str += " \t\"" + col.field + "\"";
                            }
                            LogVarDebugInfo("%s", row_str.c_str());
                        }
                    }
                }
            }
        }
        delete pdf_doc_ptr;
    }
    return ret;
}
