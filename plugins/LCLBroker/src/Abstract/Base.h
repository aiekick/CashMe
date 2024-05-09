#pragma once

#include <apis/CashMePluginApi.h>

class Base : public Cash::BankStatementImportModule {
public:
    virtual ~Base() = default;
    bool init(Cash::PluginBridge* vBridgePtr) final {
        return true;
    };
    void unit() final{};
};