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
typedef int64_t DateEpoch;

typedef std::string BankName;
typedef std::string BankUrl;
typedef std::string BankAgency;

typedef std::string SourceName;
typedef std::string SourceType;
typedef std::string SourceSha;

typedef std::string AccountType;
typedef std::string AccountName;
typedef std::string AccountNumber;
typedef double AccountBaseSolde;

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
typedef std::string TransactionSha;

typedef std::string DataBrokerName;
typedef std::string DataBrokerWay;

typedef std::string IncomeName;
typedef std::string IncomeDate;
typedef int64_t IncomeDateEpoch;
typedef double IncomeAmount;
typedef int32_t IncomeDay;
typedef std::string IncomeDescription;

typedef std::string BudgetDate;
typedef int64_t BudgetDateEpoch;
typedef uint32_t BudgetOffset;
typedef uint32_t BudgetProjectedDays;
typedef double MinValue;
typedef double MaxValue;

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

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
////////////// NOUVELLES STRUCTURES ///////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

struct Amounts {
    double debit = 0.0;
    double credit = 0.0;
    double amount = 0.0;
};

// BANK

struct BankInput {
    std::string name;
    std::string url;
    std::string sha;
};

struct BankOutput {
    RowID id = 0;
    BankInput datas;
    Amounts amounts;
    uint32_t count = 0;
};

// ACCOUNT

struct AccountInput {
    std::string number;
    std::string bank_agency;
    std::string type;
    std::string name;
    double base_solde = 0.0;
    std::string sha;
};

struct AccountOutput {
    RowID id = 0;
    std::string bankName;
    AccountInput datas;
    Amounts amounts;
    uint32_t count = 0;
};

// ENTITY

struct EntityInput {
    std::string name;
};

struct EntityOutput {
    RowID id = 0;
    EntityInput datas;
    Amounts amounts;
    uint32_t count = 0;
};

// CATEGORY

struct CategoryInput {
    std::string name;
};

struct CategoryOutput {
    RowID id = 0;
    CategoryInput datas;
    Amounts amounts;
    uint32_t count = 0;
};

// OPERATION

struct OperationInput {
    std::string name;
};

struct OperationOutput {
    RowID id = 0;
    OperationInput datas;
    Amounts amounts;
    uint32_t count = 0;
};

// INCOME

struct IncomeInput {
    RowID account_id = 0;
    std::string name;
    EntityInput entity;
    CategoryInput category;
    OperationInput operation;
    std::string startDate;
    std::string endDate;
    double minAmount = 0.0;
    double maxAmount = 0.0;
    int32_t minDay = 0U;
    uint32_t maxDay = 0U;
    std::string description;
    std::string comment;
    std::string sha;
    bool optional{false};
};

struct BudgetIncomeDatas {
    bool usedInMinCurve = false;
    bool usedInMaxCurve = false;
};

struct IncomeOutput {
    RowID id = 0;
    std::string accountNumber;
    IncomeInput datas;
    DateEpoch startEpoch;
    DateEpoch endEpoch;
    Amounts amounts;
    uint32_t count = 0;
    BudgetIncomeDatas budget;
};

// SOURCE

struct SourceInput {
    std::string name;
    std::string type;
    std::string sha;
};

struct SourceOutput {
    RowID id = 0;
    SourceInput datas;
};

// TRANSACTION

struct TransactionInput {
    EntityInput entity;
    CategoryInput category;
    OperationInput operation;
    SourceInput source;
    IncomeInput income;
    bool incomeConfirmed = false;
    std::string date;
    std::string description;
    std::string comment;
    double amount = 0.0;
    bool confirmed = 0;
    std::string sha;
};

struct TransactionOutput {
    RowID id = 0;
    std::string accountNumber;
    DateEpoch dateEpoch;
    TransactionInput datas;
    size_t comment_first_line_end_pos = 0;
    Amounts amounts;
    std::array<std::string, SearchColumns::SEARCH_COLUMN_Count> optimized;
    bool isOk() { return id != 0; }
};

// BUDGET

struct BudgetMinMax {
    MinValue min = 0.0;
    MaxValue max = 0.0;
};

struct BudgetOutput {
    RowID id = 0;
    std::string date;
    DateEpoch dateEpoch = 0;
    BudgetMinMax delta;
    BudgetMinMax solde;
    double soldeReal{}; // le vrai solde realisé par les vraies transactions
    std::string incomesMin;
    std::string incomesMinAmount;
    std::string incomesMax;
    std::string incomesMaxAmount;
};
