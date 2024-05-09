#include <Modules/PdfAccountStatementModule.h>
#include <ImGuiPack.h>

Cash::BankStatementModulePtr PdfAccountStatementModule::create() {
    auto res = std::make_shared<PdfAccountStatementModule>();
    if (!res->init(nullptr)) {
        res.reset();
    }
    return res;
}

Cash::BankStatement PdfAccountStatementModule::importBankStatement(const std::string& vFilePathName) {
    return {};
}
