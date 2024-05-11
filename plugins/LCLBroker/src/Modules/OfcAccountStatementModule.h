#pragma once

#include <Abstract/Base.h>

class OfcAccountStatementModule : public Base {
public:
    static Cash::BankStatementModulePtr create();

public:
    virtual ~OfcAccountStatementModule() = default;
    Cash::BankStatement importBankStatement(const std::string& vFilePathName) final;
    std::string getFileExt() const override {
        return ".ofc";
    }
};