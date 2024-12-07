#pragma once

#include <set>
#include <map>
#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <apis/CashMePluginApi.h>

typedef std::string DBFile;

typedef uint32_t RowID;

typedef std::string BankName;
typedef std::string BankUrl;
typedef std::string BankAgency;

typedef std::string SourceName;
typedef std::string SourceType;
typedef std::string SourceSha;

typedef std::string AccountType;
typedef std::string AccountName;
typedef std::string AccountNumber;
typedef double AccounBaseSolde;

typedef std::string EntityName;

typedef std::string CategoryName;

typedef std::string OperationName;

typedef std::string TransactionDate;
typedef int64_t TransactionDateEpoch;
typedef std::string TransactionDescription;
typedef std::string TransactionComment;
typedef double TransactionAmount;
typedef double TransactionDebit;
typedef double TransactionCredit;
typedef double TransactionSolde;
typedef bool TransactionConfirmed;
typedef uint32_t TransactionsCount;
typedef std::string TransactionHash;

typedef std::string DataBrokerName;
typedef std::string DataBrokerWay;

typedef std::string IncomeName;
typedef std::string IncomeDate;
typedef int64_t IncomeDateEpoch;
typedef double IncomeAmount;
typedef int32_t IncomeDay;
typedef std::string IncomeDescription;

typedef std::map<DataBrokerName, std::map<DataBrokerWay, Cash::BankStatementModulePtr>> DataBrockerContainer;

enum FilteringMode {  //
    FILTERING_MODE_BY_SEARCH = 0,
    FILTERING_MODE_BY_SELECTED_ROW_IDS
};

enum GroupingMode {  //
    GROUPING_MODE_TRANSACTIONS = 0, // defaut per transation
    GROUPING_MODE_DAYS,
    GROUPING_MODE_MONTHS,
    GROUPING_MODE_YEARS
};

enum SearchColumns {  //
    SEARCH_COLUMN_DATE = 0,
    SEARCH_COLUMN_DESCRIPTION,
    SEARCH_COLUMN_COMMENT,
    SEARCH_COLUMN_ENTITY,
    SEARCH_COLUMN_CATEGORY,
    SEARCH_COLUMN_OPERATION,
    SEARCH_COLUMN_Count
};

struct Bank {
    RowID id = 0;
    BankName name;
    BankUrl url;
};

struct AmountStats {
    TransactionDebit debit = 0.0;
    TransactionCredit credit = 0.0;
    TransactionAmount amount = 0.0;
};

struct Entity : public AmountStats {
    RowID id = 0;
    EntityName name;
};

struct Category : public AmountStats {
    RowID id = 0;
    CategoryName name;
};

struct Operation : public AmountStats {
    RowID id = 0;
    OperationName name;
};

struct Account : public AmountStats {
    RowID id = 0;
    BankName bank;
    BankAgency agency;
    AccountType type;
    AccountName name;
    AccountNumber number;
    AccounBaseSolde base_solde = 0.0;
    TransactionsCount count = 0U;
};

struct Income {
    RowID id = 0;
    AccountNumber account;
    EntityName entity;
    CategoryName category;
    OperationName operation;
    IncomeName name;
    IncomeDate startDate;
    IncomeDateEpoch startDateEpoch = 0;
    IncomeDate endDate;
    IncomeDateEpoch endDateEpoch = 0;
    IncomeAmount minAmount = 0.0;
    IncomeAmount maxAmount = 0.0;
    IncomeDay minDay = 0U;
    IncomeDay maxDay = 0U;
    IncomeDescription description;
};

struct Transaction {
    RowID id = 0;
    AccountNumber account;
    EntityName entity;
    CategoryName category;
    OperationName operation;
    SourceName source;
    TransactionDate date;
    TransactionDateEpoch epoch = 0;
    TransactionDescription description;
    TransactionComment comment;
    TransactionDebit debit = 0.0;
    TransactionCredit credit = 0.0;
    TransactionAmount amount = 0.0;
    TransactionSolde solde = 0.0;
    TransactionConfirmed confirmed = false;
    TransactionHash hash;
    std::array<std::string, SearchColumns::SEARCH_COLUMN_Count> optimized;  //
    bool isOk() {
        return id != 0;
    }
};

