// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <Models/DataBase.h>

#include <vector>
#include <sstream>
#include <string.h>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <sqlite3.hpp>
#include <fstream>

// will check database header magic number
// https://www.sqlite.org/fileformat.html : section 1.3
// Offset	Size	Description
// 0	    16	    The header string : "SQLite format 3\000"
bool DataBase::IsFileASqlite3DB(const DBFile& vDBFilePathName) {
    bool res = false;
    std::ifstream file_stream(vDBFilePathName, std::ios_base::binary);
    if (file_stream.is_open()) {
        char magic_header[16 + 1];
        file_stream.read(magic_header, 16U);
        if (strcmp(magic_header, "SQLite format 3\000") == 0) {
            res = true;
        }

        file_stream.close();
    }
    return res;
}

bool DataBase::CreateDBFile(const DBFile& vDBFilePathName) {
    if (!vDBFilePathName.empty()) {
        m_DataBaseFilePathName = vDBFilePathName;
        return m_CreateDB();
    }
    return false;
}

bool DataBase::OpenDBFile() {
    return OpenDBFile(m_DataBaseFilePathName);
}

bool DataBase::OpenDBFile(const DBFile& vDBFilePathName) {
    if (!m_SqliteDB) {
        m_DataBaseFilePathName = vDBFilePathName;
        return m_OpenDB();
    } else {
        LogVarInfo("%s", "Database already opened\n");
    }
    return (m_SqliteDB != nullptr);
}

void DataBase::CloseDBFile() {
    m_CloseDB();
}

bool DataBase::BeginTransaction() {
    if (m_OpenDB()) {
        if (sqlite3_exec(m_SqliteDB, "BEGIN TRANSACTION;", nullptr, nullptr, &m_LastErrorMsg) == SQLITE_OK) {
            m_TransactionStarted = true;
            return true;
        }
    }
    LogVarError("Fail to start transaction : %s", m_LastErrorMsg);
    return false;
}

void DataBase::CommitTransaction() {
    if (sqlite3_exec(m_SqliteDB, "COMMIT;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to commit : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
}

void DataBase::RollbackTransaction() {
    if (sqlite3_exec(m_SqliteDB, "ROLLBACK;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to ROLLBACK : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
}

void DataBase::AddUser(const UserName& vUserName) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO users (name) VALUES("%s");)", vUserName.c_str());
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a user in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetUser(const UserName& vUserName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT user_id FROM users WHERE name = "%s";)", vUserName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get user id with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    vOutRowID = sqlite3_column_int(stmt, 0);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

void DataBase::GetUsers(std::function<void(const UserName&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(u8R"(SELECT name FROM users GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get users with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    const char* user_name = (const char*)sqlite3_column_text(stmt, 0);
                    vCallback(user_name != nullptr ? user_name : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateUser(const RowID& vRowID, const UserName& vUserName) {
    auto insert_query = ct::toStr(u8R"(UPDATE users SET name = "%s" WHERE user_id = %u;)", vUserName.c_str(), vRowID);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a user in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddBank(const BankName& vBankName, const std::string& vUrl) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO banks (name, url) VALUES("%s", "%s");)", vBankName.c_str(), vUrl.c_str());
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a bank in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetBank(const BankName& vBankName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT bank_id FROM banks WHERE name = "%s";)", vBankName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get bank id with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    vOutRowID = sqlite3_column_int(stmt, 0);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

void DataBase::GetBanks(std::function<void(const BankName&, const std::string&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(u8R"(SELECT name, url FROM banks GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get banks with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    const char* bank_name = (const char*)sqlite3_column_text(stmt, 0);
                    const char* bank_url = (const char*)sqlite3_column_text(stmt, 1);
                    vCallback(                                  //
                        bank_name != nullptr ? bank_name : "",  //
                        bank_url != nullptr ? bank_url : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateBank(const RowID& vRowID, const BankName& vBankName, const std::string& vUrl) {
    auto insert_query = ct::toStr(u8R"(UPDATE banks SET name = "%s", url = "%s" WHERE bank_id = %u;)", vBankName.c_str(), vUrl.c_str(), vRowID);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a bank in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddCategory(const CategoryName& vCategoryName) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO categories (name) VALUES("%s");)", vCategoryName.c_str());
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a category in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetCategory(const CategoryName& vUserName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT category_id FROM categories WHERE name = "%s";)", vUserName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get category id with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    vOutRowID = sqlite3_column_int(stmt, 0);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

void DataBase::GetCategories(std::function<void(const CategoryName&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(u8R"(SELECT name FROM categories GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get categories with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    const char* category_name = (const char*)sqlite3_column_text(stmt, 0);
                    vCallback(category_name != nullptr ? category_name : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateCategory(const RowID& vRowID, const CategoryName& vCategoryName) {
    auto insert_query = ct::toStr(u8R"(UPDATE categories SET name = "%s" WHERE category_id = %u;)", vCategoryName.c_str(), vRowID);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a category in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddOperation(const OperationName& vOperationName) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO operations (name) VALUES("%s");)", vOperationName.c_str());
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a operation in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetOperation(const OperationName& vOperationName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT operation_id FROM operations WHERE name = "%s";)", vOperationName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get operation id with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    vOutRowID = sqlite3_column_int(stmt, 0);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

void DataBase::GetOperations(std::function<void(const OperationName&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(u8R"(SELECT name FROM operations GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get operations with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    const char* operation_name = (const char*)sqlite3_column_text(stmt, 0);
                    vCallback(operation_name != nullptr ? operation_name : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateOperation(const RowID& vRowID, const OperationName& vOperationName) {
    auto insert_query = ct::toStr(u8R"(UPDATE operations SET name = "%s" WHERE operation_id = %u;)", vOperationName.c_str(), vRowID);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a operation in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddAccount(const UserName& vUserName,
                          const BankName& vBankName,
                          const AccountType& vAccountType,
                          const AccountName& vAccountName,
                          const AccountNumber& vAccountNumber,
                          const AccounBaseSolde& vBaseSolde) {
    AddUser(vUserName);
    AddBank(vBankName);
    auto insert_query = ct::toStr(
        u8R"(
INSERT OR IGNORE INTO accounts 
    (user_id, bank_id, type, name, number, base_solde) VALUES(
        (SELECT user_id FROM users WHERE users.name = "%s"), -- user id
        (SELECT bank_id FROM banks WHERE banks.name = "%s"), -- bank id
        "%s", -- account type
        "%s", -- account name
        "%s", -- account number
        %f    -- account base solde
        );)",
        vUserName.c_str(),
        vBankName.c_str(),
        vAccountType.c_str(),
        vAccountName.c_str(),
        vAccountNumber.c_str(),
        vBaseSolde);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a Bank Account in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetAccount(const AccountNumber& vAccountNumber,
                          RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(
        u8R"(
SELECT 
  account_id 
FROM 
  accounts 
WHERE 
  accounts.number = "%s"
;)",
        vAccountNumber.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get account id with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    vOutRowID = sqlite3_column_int(stmt, 0);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetAccount(const UserName& vUserName,
                          const BankName& vBankName,
                          const AccountType& vAccountType,
                          const AccountName& vAccountName,
                          const AccountNumber& vAccountNumber,
                          RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(
        u8R"(
SELECT 
  account_id 
FROM 
  accounts 
  LEFT JOIN users ON accounts.user_id = users.user_id
  LEFT JOIN banks ON accounts.bank_id = banks.bank_id
WHERE 
  users.name = "%s"
  AND banks.name = "%s"
  AND accounts.type = "%s"
  AND accounts.name = "%s"
  AND accounts.number = "%s"
;)",
        vUserName.c_str(),     //
        vBankName.c_str(),     //
        vAccountType.c_str(),  //
        vAccountName.c_str(),  //
        vAccountNumber.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get account id with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    vOutRowID = sqlite3_column_int(stmt, 0);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

void DataBase::GetAccounts(  //
    std::function<void(      //
        const RowID&,
        const UserName&,
        const BankName&,
        const AccountType&,
        const AccountName&,
        const AccountNumber&,
        const AccounBaseSolde&,
        const TransactionsCount&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    std::string select_query =
        u8R"(
SELECT
  accounts.account_id AS rowid,
  users.name AS user_name,
  banks.name AS bank_name,
  accounts.type AS account_type,
  accounts.name AS account_name,
  accounts.number AS account_number,
  accounts.base_solde AS account_base_solde,
  COUNT(t.account_id) AS nombre_transactions
FROM accounts
LEFT JOIN users ON users.user_id = accounts.user_id
LEFT JOIN banks ON banks.bank_id = accounts.bank_id
LEFT JOIN transactions t ON accounts.account_id = t.account_id
GROUP BY user_name, bank_name, account_name
ORDER BY accounts.account_id;
)";
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get accounts with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID account_id = sqlite3_column_int(stmt, 0);
                    const char* user_name = (const char*)sqlite3_column_text(stmt, 1);
                    const char* bank_name = (const char*)sqlite3_column_text(stmt, 2);
                    const char* account_type = (const char*)sqlite3_column_text(stmt, 3);
                    const char* account_name = (const char*)sqlite3_column_text(stmt, 4);
                    const char* account_number = (const char*)sqlite3_column_text(stmt, 5);
                    AccounBaseSolde account_base_solde = sqlite3_column_double(stmt, 6);
                    TransactionsCount transactions_count = sqlite3_column_int(stmt, 7);
                    vCallback(                                            //
                        account_id,                                       //
                        user_name != nullptr ? user_name : "",            //
                        bank_name != nullptr ? bank_name : "",            //
                        account_type != nullptr ? account_type : "",      //
                        account_name != nullptr ? account_name : "",      //
                        account_number != nullptr ? account_number : "",  //
                        account_base_solde,                               //
                        transactions_count);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateAccount(const RowID& vRowID,
                             const UserName& vUserName,
                             const BankName& vBankName,
                             const AccountType& vAccountType,
                             const AccountName& vAccountName,
                             const AccountNumber& vAccountNumber,
                             const AccounBaseSolde& vBaseSolde) {
    auto insert_query = ct::toStr(u8R"(
UPDATE 
  accounts
SET 
  user_id = (SELECT user_id FROM users WHERE name = "%s"),
  bank_id = (SELECT bank_id FROM banks WHERE name = "%s"),
  type = "%s",
  name = "%s",
  number = "%s",
  base_solde = %f
WHERE
  accounts.account_id = %u;
)",
        vUserName.c_str(),
        vBankName.c_str(),
        vAccountType.c_str(),
        vAccountName.c_str(),
        vAccountNumber.c_str(),
        vBaseSolde,
        vRowID);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a account in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddTransaction(const RowID& vAccountID,
                              const CategoryName& vCategoryName,
                              const OperationName& vOperationName,
                              const TransactionDate& vDate,
                              const TransactionDescription& vDescription,
                              const TransactionComment& vComment,
                              const TransactionAmount& vAmount,
                              const std::string& vHash) {
    AddCategory(vCategoryName);
    AddOperation(vOperationName);
    auto insert_query = ct::toStr(
        u8R"(
INSERT OR IGNORE INTO transactions 
    (account_id, category_id, operation_id, amount, date, description, comment, hash) VALUES(
        %u, -- account id
        (SELECT category_id FROM categories WHERE categories.name = "%s"), -- category id
        (SELECT operation_id FROM operations WHERE operations.name = "%s"), -- operation id
        %.6f, 
        "%s", 
        "%s",
        "%s",
        "%s"
        );)",
        vAccountID,
        vCategoryName.c_str(),
        vOperationName.c_str(),
        vAmount,
        vDate.c_str(),
        vDescription.c_str(),
        vComment.c_str(),
        vHash.c_str());
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a transaction in database : %s", m_LastErrorMsg);
    }
}

void DataBase::GetTransactions(  //
    const RowID& vAccountID,
    std::function<void(  //
        const RowID&,
        const TransactionDate&,
        const TransactionDescription&,
        const TransactionComment&,
        const CategoryName&,
        const OperationName&,
        const TransactionAmount&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ct::toStr(
        u8R"(
SELECT
  transactions.transaction_id AS rowid,
  transactions.date,
  transactions.description,
  transactions.comment,
  operations.name AS operation,
  categories.name AS category,
  transactions.amount
FROM
  transactions
  LEFT JOIN accounts ON transactions.account_id = accounts.account_id
  LEFT JOIN operations ON transactions.operation_id = operations.operation_id
  LEFT JOIN categories ON transactions.category_id = categories.category_id
WHERE
  transactions.account_id = %u
ORDER BY
  transactions.date;
)",
        vAccountID);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID transaction_id = sqlite3_column_int(stmt, 0);
                    const char* transaction_date = (const char*)sqlite3_column_text(stmt, 1);
                    const char* transaction_description = (const char*)sqlite3_column_text(stmt, 2);
                    const char* transaction_comment = (const char*)sqlite3_column_text(stmt, 3);
                    const char* operation_name = (const char*)sqlite3_column_text(stmt, 4);
                    const char* category_name = (const char*)sqlite3_column_text(stmt, 5);
                    double transaction_amount = sqlite3_column_double(stmt, 6);
                    vCallback(  //
                        transaction_id,
                        transaction_date != nullptr ? transaction_date : "",                //
                        transaction_description != nullptr ? transaction_description : "",  //
                        transaction_comment != nullptr ? transaction_comment : "",          //
                        category_name != nullptr ? category_name : "",                      //
                        operation_name != nullptr ? operation_name : "",                    //
                        transaction_amount);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateTransaction(  //
    const RowID& vRowID,
    const CategoryName& vCategoryName,
    const OperationName& vOperationName,
    const TransactionDate& vDate,
    const TransactionDescription& vDescription,
    const TransactionComment& vComment,
    const double& vAmount) {
    auto insert_query = ct::toStr(
        u8R"(
UPDATE 
  transactions
SET 
  category_id = (SELECT category_id FROM categories WHERE name = "%s"),
  operation_id = (SELECT operation_id FROM operations WHERE name = "%s"),
  date = "%s",
  description = "%s",
  comment = "%s",
  amount = %.6f
WHERE
  transactions.transaction_id = %u;
)",
        vCategoryName.c_str(),
        vOperationName.c_str(),
        vDate.c_str(),
        vDescription.c_str(),
        vComment.c_str(),
        vAmount,
        vRowID);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a transaction in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::m_EnableForeignKey() {
    if (!m_SqliteDB) {
        int res = sqlite3_exec(m_SqliteDB, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("Erreur lors de l'activation des clés étrangères : %s", sqlite3_errmsg(m_SqliteDB));
        }
    }
    return (m_SqliteDB != nullptr);
}

std::string DataBase::GetLastErrorMesg() {
    return std::string(m_LastErrorMsg);
}

bool DataBase::SetSettingsXMLDatas(const std::string& vXMLDatas) {
    if (!vXMLDatas.empty()) {
        std::string insert_query;
        // insert or replace at line 0
        auto xml_datas = GetSettingsXMLDatas();
        if (xml_datas.empty()) {
            insert_query = "INSERT INTO settings(xml) VALUES(\"" + vXMLDatas + "\");";
        } else {
            insert_query = "UPDATE settings SET xml = \"" + vXMLDatas + "\" WHERE rowid = 1;";
        }
        if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
#ifdef _DEBUG
            FileHelper::Instance()->SaveStringToFile(insert_query, "insert_query.txt");
            FileHelper::Instance()->SaveStringToFile(m_LastErrorMsg, "last_error_msg.txt");
#endif
            LogVarError("Fail to insert or replace xml in table settings of database : %s", m_LastErrorMsg);
            return false;
        }
        return true;
    }
    return false;
}

std::string DataBase::GetSettingsXMLDatas() {
    std::string res;
    // SELECT at line 0
    auto select_query = u8R"(SELECT * FROM settings WHERE rowid = 1;)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_SqliteDB, select_query, (int)strlen(select_query), &stmt, nullptr) != SQLITE_OK) {
        LogVarError("%s", "Fail to get xml FROM settings table of database");
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            auto len = sqlite3_column_bytes(stmt, 0);
            auto txt = (const char*)sqlite3_column_text(stmt, 0);
            if (txt && len) {
                res = std::string(txt, len);
            }
        }
    }
    sqlite3_finalize(stmt);
    return res;
}

void DataBase::ClearDataTables() {
    auto clear_query =
        u8R"(
BEGIN TRANSACTION;
DELETE FROM users;
DELETE FROM banks;
DELETE FROM accounts;
DELETE FROM categories;
DELETE FROM operations;
DELETE FROM trajectories;
DELETE FROM transactions;
COMMIT;
)";
    if (sqlite3_exec(m_SqliteDB, clear_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to clear datas tables in database : %s", m_LastErrorMsg);
    }
}

////////////////////////////////////////////////////////////
///// PRIVATE //////////////////////////////////////////////
////////////////////////////////////////////////////////////

bool DataBase::m_OpenDB() {
    if (!m_SqliteDB) {
        if (sqlite3_open_v2(m_DataBaseFilePathName.c_str(), &m_SqliteDB, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {  // db possibily not exist
            m_CreateDBTables(false);
        } else {
            m_EnableForeignKey();
        }
    }
    return (m_SqliteDB != nullptr);
}

bool DataBase::m_CreateDB() {
    m_CloseDB();
    if (!m_SqliteDB) {
        FileHelper::Instance()->DestroyFile(m_DataBaseFilePathName);
        if (sqlite3_open_v2(m_DataBaseFilePathName.c_str(), &m_SqliteDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) == SQLITE_OK) {  // db possibily not exist
            m_CreateDBTables();
            m_CloseDB();
        }
    }
    return (m_SqliteDB != nullptr);
}

void DataBase::m_CreateDBTables(const bool& vPrintLogs) {
    if (m_SqliteDB) {  // in the doubt
        const char* create_tables =
            u8R"(
CREATE TABLE users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE banks (
    bank_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    url TEXT
);

CREATE TABLE accounts (
    account_id INTEGER PRIMARY KEY AUTOINCREMENT,
    number TEXT NOT NULL UNIQUE,
    user_id INTEGER NOT NULL,
    bank_id INTEGER NOT NULL,
    type TEXT NOT NULL,
    name TEXT NOT NULL,
    base_solde REAL,
    FOREIGN KEY (user_id) REFERENCES users(user_id),
    FOREIGN KEY (bank_id) REFERENCES banks(bank_id)
);

CREATE TABLE categories (
    category_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

-- type of bank transaction, like CB, SEPA, etc... 
CREATE TABLE operations (
    operation_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE transactions (
    transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id INTEGER NOT NULL,
    operation_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    amount REAL NOT NULL,
    date DATE NOT NULL,
    description TEXT,
    comment TEXT,
    hash NOT NULL UNIQUE,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id),
    FOREIGN KEY (operation_id) REFERENCES operations(operation_id),
    FOREIGN KEY (category_id) REFERENCES categories(category_id)
);

CREATE TABLE settings (
	xml TEXT
);
)";
        if (sqlite3_exec(m_SqliteDB, create_tables, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            if (m_LastErrorMsg) {
                LogVarError("Fail to create database : %s", m_LastErrorMsg);
            } else {
                LogVarError("%s", "Fail to create database");
            }
            m_SqliteDB = nullptr;
        }
    }
}

void DataBase::m_CloseDB() {
    if (m_SqliteDB) {
        if (sqlite3_close(m_SqliteDB) == SQLITE_BUSY) {
            // try to force closing
            sqlite3_close_v2(m_SqliteDB);
        }
    }
    m_SqliteDB = nullptr;
    // there is also sqlite3LeaveMutexAndCloseZombie when sqlite is stucked
}
