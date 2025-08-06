#pragma once

#include <Abstract/Base.h>

class OfcAccountStatementModule : public Base {
private:
    enum class FileType { NONE=0, OFC, OFX, Count } m_fileType;

public:
    static Cash::BankStatementModulePtr create();

public:
    virtual ~OfcAccountStatementModule() = default;
    std::string getFileExt() const override;
    Cash::AccountTransactions importBankStatement(const std::string& vFilePathName) final;
};