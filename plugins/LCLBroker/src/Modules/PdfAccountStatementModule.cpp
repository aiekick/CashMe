#include <Modules/PdfAccountStatementModule.h>
#include <ImGuiPack.h>

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
#include <xpdf/xpdf/TextOutputDev.h>
#include <xpdf/xpdf/CharTypes.h>
#include <xpdf/xpdf/UnicodeMap.h>
#include <xpdf/xpdf/TextString.h>
#include <xpdf/xpdf/Error.h>
#include <xpdf/xpdf/config.h>

// we can check the file pdftotext with -table option
// xpdf/xpdf/pdftotext.c
// xpdf/xpdf/TextOutputDev.cc

Cash::BankStatementModulePtr PdfAccountStatementModule::create() {
    auto res = std::make_shared<PdfAccountStatementModule>();
    if (!res->init(nullptr)) {
        res.reset();
    }
    return res;
}

Cash::AccountStatements PdfAccountStatementModule::importBankStatement(const std::string& vFilePathName) {
    return {};
}
