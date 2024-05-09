#pragma once

#include <Abstract/Base.h>

class PdfAccountStatementModule : public Base {
public:
    static Cash::BankStatementModulePtr create();

public:
    virtual ~PdfAccountStatementModule() = default;
    Cash::BankStatement importBankStatement(const std::string& vFilePathName) final;
};