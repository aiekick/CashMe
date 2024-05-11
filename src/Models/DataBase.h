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

#include <memory>
#include <string>
#include <functional>

typedef std::string DBFile;
typedef uint32_t RowID;
typedef std::string UserName;
typedef std::string BankName;
typedef std::string AccountType;
typedef std::string AccountName;
typedef std::string CategoryName;
typedef std::string OperationName;
typedef std::string AccountNumber;
typedef std::string TransactionDate;
typedef std::string TransactionDescription;
typedef double TransactionAmount;

class DataBaseTable {
public:
};

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

    void AddUser(const UserName& vUserName);
    bool GetUser(const UserName& vUserName, RowID& vOutRowID);
    void GetUsers(std::function<void(const UserName&)> vCallback);

    void AddBank(const BankName& vBankName, const std::string& vUrl = {});
    bool GetBank(const BankName& vBankName, RowID& vOutRowID);
    void GetBanks(std::function<void(const BankName&, const std::string&)> vCallback);

    void AddCategory(const CategoryName& vCategoryName);
    bool GetCategory(const CategoryName& vUserName, RowID& vOutRowID);
    void GetCategories(std::function<void(const CategoryName&)> vCallback);

    void AddOperation(const OperationName& vOperationName);
    bool GetOperation(const OperationName& vUserName, RowID& vOutRowID);
    void GetOperations(std::function<void(const OperationName&)> vCallback);

    void AddAccount(  //
        const UserName& vUserName,
        const BankName& vBankName,
        const AccountType& vAccountType,
        const AccountName& vAccountName,
        const AccountNumber& vAccountNumber);
    bool GetAccount(  //
        const AccountNumber& vAccountNumber,
        RowID& vOutRowID);
    bool GetAccount(  //
        const UserName& vUserName,
        const BankName& vBankName,
        const AccountType& vAccountType,
        const AccountName& vAccountName,
        const AccountNumber& vAccountNumber,
        RowID& vOutRowID);
    void GetAccounts(        //
        std::function<void(  //
            const RowID&, 
            const UserName&,
            const BankName&,
            const AccountType&,
            const AccountName&,
            const AccountNumber&)> vCallback);

    void AddTransaction(  //
        const RowID& vAccountID,
        const CategoryName& vCategoryName,
        const OperationName& vOperationName,
        const std::string& vDate,
        const std::string& vDescription,
        const double& vAmmount);
    void GetTransactions(     //
        const RowID& vAccountID,
        std::function<void(   //
            const TransactionDate&,  
            const TransactionDescription&,
            const CategoryName&,
            const OperationName&,
            const TransactionAmount&)> vCallback);

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