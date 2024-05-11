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
typedef std::string UserName;
typedef std::string BankName;
typedef std::string AccountType;
typedef std::string AccountName;
typedef std::string AccountNumber;


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
    bool GetUser(const UserName& vUserName, uint32_t& vOutRowID);
    void GetUsers(std::function<void(const UserName&)> vCallback);

    void AddBank(const BankName& vBankName, const std::string& vUrl = {});
    bool GetBank(const BankName& vBankName, uint32_t& vOutRowID);
    void GetBanks(std::function<void(const BankName&, const std::string&)> vCallback);

    void AddAccount(const UserName& vUserName,
                    const BankName& vBankName,
                    const AccountType& vAccountType,
                    const AccountName& vAccountName,
                    const AccountNumber& vAccountNumber);
    void GetAccounts(std::function<void(const UserName&, const BankName&, const AccountType&, const AccountName&, const AccountNumber&)> vCallback);

    void AddCategory(const std::string& vCategoryName);
    void AddOperation(const std::string& vOperationName);

    // for gain some time we must extract ID's before calling
    void AddTransaction(const uint32_t& vAccountID,
                        const std::string& vCategoryName,
                        const std::string& vOperationName,
                        const double& vAmmount,
                        const std::string& vDate,
                        const std::string& vDescription);

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