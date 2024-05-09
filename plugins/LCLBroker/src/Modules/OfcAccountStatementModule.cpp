#include <Modules/OfcAccountStatementModule.h>
#include <ImGuiPack.h>

Cash::BankStatementModulePtr OfcAccountStatementModule::create() {
    auto res = std::make_shared<OfcAccountStatementModule>();
    if (!res->init(nullptr)) {
        res.reset();
    }
    return res;
}

Cash::BankStatement OfcAccountStatementModule::importBankStatement(const std::string& vFilePathName) {
    return {};
}
