/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <set>
#include <memory>
#include <string>
#include <functional>
#include <sqlite3.hpp>
#include <Headers/DatasDef.h>
#include <ezlibs/ezSingleton.hpp>

enum class DateFormat { DAYS = 0, MONTHS, YEARS, Count };
enum class GroupBy { DATES = 0, ENTITIES, OPERATIONS, CATEGORIES, DESCRIPTIONS, Count };

struct sqlite3;
class DataBase {
    IMPLEMENT_SINGLETON(DataBase)
private:
    sqlite3* m_SqliteDB = nullptr;
    std::string m_DataBaseFilePathName = "datas.db3";
    bool m_TransactionStarted = false;
    char* m_LastErrorMsg = nullptr;

public:
    // DATABASE FILE

    bool IsFileASqlite3DB(const DBFile& vDBFilePathName);
    bool CreateDBFile(const DBFile& vDBFilePathName);
    bool OpenDBFile();
    bool OpenDBFile(const DBFile& vDBFilePathName);
    void CloseDBFile();

    // TRANSACTIONS

    bool BeginTransaction();
    void CommitTransaction();
    void RollbackTransaction();

    // MISC

    void ClearDataTables();
    std::string GetLastErrorMesg();

    // SETTINGS

    bool SetSettingsXMLDatas(const std::string& vXMLDatas);
    std::string GetSettingsXMLDatas();

    // BANK

    bool AddBank(const BankInput& vBankInput);
    bool GetBank(const BankName& vBankName, RowID& vOutRowID);
    bool GetBanks(std::function<void(const BankOutput&)> vCallback);
    bool GetBanksStats(std::function<void(const BankOutput&)> vCallback);
    bool UpdateBank(const RowID& vRowID, const BankInput& vBankInput);
    bool DeleteBanks();

    // ACCOUNT

    bool AddAccount(const BankName& vBankName, const AccountInput& vAccountInput);
    bool GetAccount(const AccountNumber& vAccountNumber, RowID& vOutRowID);
    bool GetAccounts(std::function<void(const AccountOutput&)> vCallback);
    bool GetAccountsStats(std::function<void(const AccountOutput&)> vCallback);
    bool UpdateAccount(const RowID& vRowID, const BankName &vBankName, const AccountInput& vAccountInput);
    bool DeleteAccount(const RowID& vRowID);
    bool DeleteAccounts();

    // ENTITY

    bool AddEntity(const EntityInput& vEntityInput);
    bool GetEntity(const EntityName& vEntityName, RowID& vOutRowID);
    bool GetEntities(std::function<void(const EntityOutput&)> vCallback);
    bool GetEntitiesStats(const RowID& vAccountID, std::function<void(const EntityOutput&)> vCallback);
    bool UpdateEntity(const RowID& vRowID, const EntityInput& vEntityInput);
    bool DeleteEntities();

    // CATEGORY

    bool AddCategory(const CategoryInput& vCategoryInput);
    bool GetCategory(const CategoryName& vCategoryName, RowID& vOutRowID);
    bool GetCategories(std::function<void(const CategoryOutput&)> vCallback);
    bool GetCategoriesStats(const RowID& vAccountID, std::function<void(const CategoryOutput&)> vCallback);
    bool UpdateCategory(const RowID& vRowID, const CategoryInput& vCategoryInput);
    bool DeleteCategories();

    // OPERATION

    bool AddOperation(const OperationInput& vOperationInput);
    bool GetOperation(const OperationName& vOperationName, RowID& vOutRowID);
    bool GetOperations(std::function<void(const OperationOutput&)> vCallback);
    bool GetOperationsStats(const RowID& vAccountID, std::function<void(const OperationOutput&)> vCallback);
    bool UpdateOperation(const RowID& vRowID, const OperationInput& vOperationInput);
    bool DeleteOperations();

    // SOURCE

    void AddSource(const SourceName& vSourceName, const SourceType& vSourceType, const SourceSha& vSourceSha);
    bool GetSource(const SourceName& vSourceName, RowID& vOutRowID);
    void GetSources(std::function<void(const SourceName&)> vCallback);
    void UpdateSource(const RowID& vRowID, const SourceName& vSourceName);
    void DeleteSources();

    // INCOME

    bool AddIncome(const RowID& vAccountID, const IncomeInput& vIncomeInput);
    bool GetIncomes(const RowID& vAccountID, std::function<void(const IncomeOutput&)> vCallback);
    bool UpdateIncome(const RowID& vRowID, const IncomeInput& vIncomeInput);
    bool DeleteIncome(const RowID& vRowID);
    bool DeleteIncomes(const std::set<RowID>& vRowIDs);
    bool DeleteIncomes();

    // TRANSACTION

    void AddTransaction(  //
        const RowID& vAccountID,
        const EntityName& vEntityName,
        const CategoryName& vCategoryName,
        const OperationName& vOperationName,
        const SourceName& vSourceName,
        const SourceType& vSourceType,
        const SourceSha& vSourceSha,
        const TransactionDate& vDate,
        const TransactionDescription& vDescription,
        const TransactionComment& vComment,
        const TransactionAmount& vAmount,
        const TransactionConfirmed& vConfirmed,
        const TransactionSha& vSha);
    void GetTransactions(  //
        const RowID& vAccountID,
        std::function<void(  //
            const RowID&,
            const EntityName&,
            const CategoryName&,
            const OperationName&,
            const SourceName&,
            const TransactionDate&,
            const TransactionDateEpoch&,
            const TransactionDescription&,
            const TransactionComment&,
            const TransactionAmount&,
            const TransactionConfirmed&,
            const TransactionSha&)> vCallback);
    void GetGroupedTransactions(  //
        const RowID& vAccountID,
        const GroupBy& vGroupBy,
        const DateFormat& vGroupByDate,
        std::function<void(  //
            const RowID&,
            const TransactionDate&,
            const TransactionDescription&,
            const EntityName&,
            const CategoryName&,
            const OperationName&,
            const TransactionDebit&,
            const TransactionCredit&)> vCallback);
    std::string GetFormatDate(const DateFormat& vDateFormat);
    void GetDuplicateTransactionsOnDatesAndAmount(  //
        const RowID& vAccountID,                    //
        std::function<void(const RowID&)> vCallback);
    void GetUnConfirmedTransactions(  //
        const RowID& vAccountID,      //
        std::function<void(const RowID&)> vCallback);
    void UpdateTransaction(  //
        const RowID& vRowID,
        const EntityName& vEntityName,
        const CategoryName& vCategoryName,
        const OperationName& vOperationName,
        const SourceName& vSourceName,
        const TransactionDate& vDate,
        const TransactionDescription& vDescription,
        const TransactionComment& vComment,
        const TransactionAmount& vAmount,
        const TransactionConfirmed& vConfirmed,
        const TransactionSha& vSha);
    void ConfirmTransaction(  //
        const RowID& vRowID,
        const TransactionConfirmed& vConfirmed);
    void DeleteTransaction(const RowID& vRowID);
    void DeleteTransactions();
    void DeleteTransactions(const std::set<RowID>& vRowIDs);

    // BUDGET

    bool ComputeBudget(  //
        const RowID& vAccountID,
        const BudgetProjectedDays& vProjectedDays,
        std::function<void(const BudgetOutput&)> vCallback);

private:
    bool m_OpenDB();
    void m_CloseDB();
    bool m_CreateDB();
    void m_CreateDBTables(const bool& vPrintLogs = true);
    bool m_EnableForeignKey();
    static int32_t m_debug_sqlite3_exec(  //
        const char* vDebugLabel,
        sqlite3* db,                                 /* An open database */
        const char* sql_query,                       /* SQL to be evaluated */
        int (*callback)(void*, int, char**, char**), /* Callback function */
        void* arg1,                                  /* 1st argument to callback */
        char** errmsg);

    static int32_t m_debug_sqlite3_prepare_v2(  //
        const char* vDebugLabel,
        sqlite3* db,           /* Database handle. */
        const char* sql_query, /* UTF-8 encoded SQL statement. */
        int nBytes,            /* Length of zSql in bytes. */
        sqlite3_stmt** ppStmt, /* OUT: A pointer to the prepared statement */
        const char** pzTail);

public:
    DataBase() = default;                // Prevent construction
    DataBase(const DataBase&) = delete;  // Prevent construction by copying
    DataBase& operator=(const DataBase&) { return *this; };  // Prevent assignment
    virtual ~DataBase() = default;  // Prevent unwanted destruction};
};
