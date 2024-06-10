#pragma once

#include <apis/CashMePluginApi.h>

bool parseDescription(const std::string& vDesc,  //
                      std::string& vOutEntity,
                      std::string& vOutOperation,
                      std::string& vOutDescription);

class Base : public Cash::BankStatementImportModule {
public:
    virtual ~Base() = default;
    bool init(Cash::PluginBridge* vBridgePtr) final;
    void unit() final;
    std::string getFileExt() const override;
};