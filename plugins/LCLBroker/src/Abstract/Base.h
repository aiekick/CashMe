#pragma once

#include <apis/CashMePluginApi.h>

class Base : public Cash::BankStatementImportModule {
public:
    virtual ~Base() = default;
    bool init(Cash::PluginBridge* vBridgePtr) final;
    void unit() final;
    std::string getFileExt() const override;
};