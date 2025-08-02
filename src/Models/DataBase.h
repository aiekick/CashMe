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

enum class DateFormat { DAYS = 0, MONTHS, YEARS, Count };
enum class GroupBy { DATES = 0, ENTITIES, OPERATIONS, CATEGORIES, DESCRIPTIONS, Count };

struct sqlite3;
class DataBase {
private:
    sqlite3* m_SqliteDB = nullptr;
    std::string m_DataBaseFilePathName = "datas.db3";
    bool m_TransactionStarted = false;
    char* m_LastErrorMsg = nullptr;

public:
    bool IsFileASqlite3DB(const DBFile& vDBFilePathName);
    bool CreateDBFile(const DBFile& vDBFilePathName);
    bool OpenDBFile();
    bool OpenDBFile(const DBFile& vDBFilePathName);
    void CloseDBFile();
    bool BeginTransaction();
    void CommitTransaction();
    void RollbackTransaction();

    void AddBank(const BankName& vBankName, const std::string& vUrl = {});
    bool GetBank(const BankName& vBankName, RowID& vOutRowID);
    void GetBanks(std::function<void(const BankName&, const std::string&)> vCallback);
    void UpdateBank(const RowID& vRowID, const BankName& vBankName, const std::string& vUrl);
    void DeleteBanks();

    void AddEntity(const EntityName& vEntityName);
    bool GetEntity(const EntityName& vUserName, RowID& vOutRowID);
    void GetEntities(std::function<void(const EntityName&)> vCallback);
    void GetEntitiesStats(  //
        const RowID& vAccountID,
        std::function<void(  //
            const RowID&,
            const EntityName&,
            const TransactionDebit&,
            const TransactionCredit&)> vCallback);
    void UpdateEntity(const RowID& vRowID, const EntityName& vEntityName);
    void DeleteEntities();

    void AddCategory(const CategoryName& vCategoryName);
    bool GetCategory(const CategoryName& vUserName, RowID& vOutRowID);
    void GetCategories(std::function<void(const CategoryName&)> vCallback);
    void GetCategoriesStats(  //
        const RowID& vAccountID,
        std::function<void(  //
            const RowID&,
            const CategoryName&,
            const TransactionDebit&,
            const TransactionCredit&)> vCallback);
    void UpdateCategory(const RowID& vRowID, const CategoryName& vCategoryName);
    void DeleteCategories();

    void AddOperation(const OperationName& vOperationName);
    bool GetOperation(const OperationName& vUserName, RowID& vOutRowID);
    void GetOperations(std::function<void(const OperationName&)> vCallback);
    void GetOperationsStats(  //
        const RowID& vAccountID,
        std::function<void(  //
            const RowID&,
            const OperationName&,
            const TransactionDebit&,
            const TransactionCredit&)> vCallback);
    void UpdateOperation(const RowID& vRowID, const OperationName& vOperationName);
    void DeleteOperations();

    void AddSource(const SourceName& vSourceName, const SourceType& vSourceType, const SourceSha& vSourceSha);
    bool GetSource(const SourceName& vSourceName, RowID& vOutRowID);
    void GetSources(std::function<void(const SourceName&)> vCallback);
    void UpdateSource(const RowID& vRowID, const SourceName& vSourceName);
    void DeleteSources();

    void AddAccount(  //
        const BankName& vBankName,
        const BankAgency& vBankAgency,
        const AccountType& vAccountType,
        const AccountName& vAccountName,
        const AccountNumber& vAccountNumber,
        const AccounBaseSolde& vBaseSolde);
    bool GetAccount(  //
        const AccountNumber& vAccountNumber,
        RowID& vOutRowID);
    bool GetAccount(  //
        const BankName& vBankName,
        const BankAgency& vBankAgency,
        const AccountType& vAccountType,
        const AccountName& vAccountName,
        const AccountNumber& vAccountNumber,
        RowID& vOutRowID);
    void GetAccounts(        //
        std::function<void(  //
            const RowID&,
            const BankName&,
            const BankAgency&,
            const AccountType&,
            const AccountName&,
            const AccountNumber&,
            const AccounBaseSolde&,
            const TransactionsCount&)> vCallback);
    void UpdateAccount(  //
        const RowID& vRowID,
        const BankName& vBankName,
        const BankAgency& vBankAgency,
        const AccountType& vAccountType,
        const AccountName& vAccountName,
        const AccountNumber& vAccountNumber,
        const AccounBaseSolde& vBaseSolde);
    void DeleteAccount(const RowID& vRowID);
    void DeleteAccounts();

    void AddIncome(  //
        const RowID& vAccountID,
        const IncomeName& vIncomeName,
        const EntityName& vEntityName,
        const CategoryName& vCategoryName,
        const OperationName& vOperationName,
        const IncomeDate& vStartDate,
        const IncomeDate& vEndDate,
        const IncomeAmount& vMinAmount,
        const IncomeAmount& vMaxAmount,
        const IncomeDay& vMinDays,
        const IncomeDay& vMaxDays,
        const IncomeDescription& vDescription);
    void GetIncomes(         //
        const RowID& vAccountID,
        std::function<void(  //
            const RowID&,
            const IncomeName&,
            const EntityName&,
            const CategoryName&,
            const OperationName&,
            const IncomeDate&,
            const IncomeDateEpoch&,
            const IncomeDate&,
            const IncomeDateEpoch&,
            const IncomeAmount&,
            const IncomeAmount&,
            const IncomeDay&,
            const IncomeDay&,
            const IncomeDescription&)> vCallback);
    void UpdateIncome(  //
        const RowID& vRowID,
        const IncomeName& vIncomeName,
        const EntityName& vEntityName,
        const CategoryName& vCategoryName,
        const OperationName& vOperationName,
        const IncomeDate& vStartDate,
        const IncomeDate& vEndDate,
        const IncomeAmount& vMinAmount,
        const IncomeAmount& vMaxAmount,
        const IncomeDay& vMinDays,
        const IncomeDay& vMaxDays,
        const IncomeDescription& vDescription);
    void DeleteIncome(const RowID& vRowID);
    void DeleteIncomes();

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
        const TransactionHash& vHash);
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
            const TransactionHash&)> vCallback);
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
        const TransactionHash& vHash);
    void ConfirmTransaction(  //
        const RowID& vRowID,
        const TransactionConfirmed& vConfirmed);
    void DeleteTransaction(const RowID& vRowID);
    void DeleteTransactions();
    void DeleteTransactions(const std::set<RowID>& vRowIDs);

    void ClearDataTables();
    std::string GetLastErrorMesg();
    bool SetSettingsXMLDatas(const std::string& vXMLDatas);
    std::string GetSettingsXMLDatas();

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


public:  // singleton
    static std::shared_ptr<DataBase> Instance() {
        static std::shared_ptr<DataBase> _instance = std::make_shared<DataBase>();
        return _instance;
    }

public:
    DataBase() = default;                // Prevent construction
    DataBase(const DataBase&) = delete;  // Prevent construction by copying
    DataBase& operator=(const DataBase&) {
        return *this;
    };                              // Prevent assignment
    virtual ~DataBase() = default;  // Prevent unwanted destruction};
};
