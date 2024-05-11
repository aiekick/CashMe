#pragma once

#include <Abstract/Base.h>

class PdfAccountStatementModule : public Base {
public:
    static Cash::BankStatementModulePtr create();

public:
    virtual ~PdfAccountStatementModule() = default;
    Cash::AccountStatements importBankStatement(const std::string& vFilePathName) final;
    std::string getFileExt() const override {
        return ".pdf";
    }
};