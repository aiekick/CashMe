#pragma once

#include <set>
#include <array>
#include <vector>
#include <string>
#include <cstdint>

typedef std::string DBFile;
typedef uint32_t RowID;
typedef std::string BankName;
typedef std::string BankUrl;
typedef std::string BankAgency;
typedef std::string SourceName;
typedef std::string SourceType;
typedef std::string SourceSha1;
typedef std::string AccountType;
typedef std::string AccountName;
typedef std::string CategoryName;
typedef std::string OperationName;
typedef std::string AccountNumber;
typedef double AccounBaseSolde;
typedef std::string TransactionDate;
typedef std::string TransactionDescription;
typedef std::string TransactionComment;
typedef double TransactionAmount;
typedef double TransactionSolde;
typedef bool TransactionConfirmed;
typedef uint32_t TransactionsCount;
typedef std::string TransactionHash;

struct Bank {
    RowID id = 0;
    BankName name;
    BankUrl url;
};

struct Category {
    RowID id = 0;
    CategoryName name;
};

struct Operation {
    RowID id = 0;
    OperationName name;
};

struct Account {
    RowID id = 0;
    BankName bank;
    BankAgency agency;
    AccountType type;
    AccountName name;
    AccountNumber number;
    AccounBaseSolde base_solde = 0.0;
    TransactionsCount count = 0U;
};

struct Transaction {
    RowID id = 0;
    AccountNumber account;
    OperationName operation;
    CategoryName category;
    SourceName source;
    TransactionDate date;
    TransactionDescription description;
    TransactionComment comment;
    TransactionAmount amount = 0.0;
    TransactionSolde solde = 0.0;
    TransactionConfirmed confirmed = false;
    TransactionHash hash;
    // date, desc, comm, cat, op
    std::array<std::string, 5> optimized;  //
    bool isOk() {
        return id != 0;
    }
};

