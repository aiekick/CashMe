#pragma once

#include <Abstract/Base.h>

class PdfAccountStatementModule : public Base {
public:
    static Cash::BankStatementModulePtr create();

public:
    virtual ~PdfAccountStatementModule() = default;
    std::string getFileExt() const override;
    Cash::AccountTransactions importBankStatement(const std::string& vFilePathName) final;
};