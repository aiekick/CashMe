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

static int debug_sqlite3_exec(  //
    const char* vDebugLabel,
    sqlite3* db,                                 /* An open database */
    const char* sql_query,                       /* SQL to be evaluated */
    int (*callback)(void*, int, char**, char**), /* Callback function */
    void* arg1,                                  /* 1st argument to callback */
    char** errmsg) {                             /* Error msg written here */
#ifdef _DEBUG
    std::string func_name = vDebugLabel;
    ct::replaceString(func_name, "DataBase::", "");
    FileHelper::Instance()->SaveStringToFile(                     //
        sql_query,                                                //
        FileHelper::Instance()->CorrectSlashTypeForFilePathName(  //
            ct::toStr("sqlite3/%s.sql", func_name.c_str())));
#endif
    return sqlite3_exec(db, sql_query, callback, arg1, errmsg);
}

static int debug_sqlite3_prepare_v2(  //
    const char* vDebugLabel,
    sqlite3* db,           /* Database handle. */
    const char* sql_query, /* UTF-8 encoded SQL statement. */
    int nBytes,            /* Length of zSql in bytes. */
    sqlite3_stmt** ppStmt, /* OUT: A pointer to the prepared statement */
    const char** pzTail) { /* OUT: End of parsed string */
#ifdef _DEBUG
    std::string func_name = vDebugLabel;
    ct::replaceString(func_name, "DataBase::", "");
    FileHelper::Instance()->SaveStringToFile(  //
        sql_query,                             //
        FileHelper::Instance()->CorrectSlashTypeForFilePathName(//
            ct::toStr("sqlite3/%s.sql", func_name.c_str())));
#endif
    return sqlite3_prepare_v2(db, sql_query, nBytes, ppStmt, pzTail);
}

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
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "BEGIN TRANSACTION;", nullptr, nullptr, &m_LastErrorMsg) == SQLITE_OK) {
            m_TransactionStarted = true;
            return true;
        }
    }
    LogVarError("Fail to start transaction : %s", m_LastErrorMsg);
    return false;
}

void DataBase::CommitTransaction() {
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "COMMIT;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to commit : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
    m_CloseDB();
}

void DataBase::RollbackTransaction() {
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "ROLLBACK;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to ROLLBACK : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
}

void DataBase::AddBank(const BankName& vBankName, const std::string& vUrl) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO banks (name, url) VALUES("%s", "%s");)", vBankName.c_str(), vUrl.c_str());
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a bank in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetBank(const BankName& vBankName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT id FROM banks WHERE name = "%s";)", vBankName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    auto insert_query = ct::toStr(u8R"(UPDATE banks SET name = "%s", url = "%s" WHERE id = %u;)", vBankName.c_str(), vUrl.c_str(), vRowID);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a bank in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteBanks() {
    if (m_OpenDB()) {
        auto insert_query = ct::toStr(u8R"(DELETE FROM banks;)");
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of banks table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::AddEntity(const EntityName& vEntityName) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO entities (name) VALUES("%s");)", vEntityName.c_str());
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a entity in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetEntity(const EntityName& vUserName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT id FROM entities WHERE name = "%s";)", vUserName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get entity id with reason", sqlite3_errmsg(m_SqliteDB));
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

void DataBase::GetEntities(std::function<void(const EntityName&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(u8R"(SELECT name FROM entities GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get entities with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    const char* entity_name = (const char*)sqlite3_column_text(stmt, 0);
                    vCallback(entity_name != nullptr ? entity_name : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::GetEntitiesStats(  //
    const RowID& vAccountID,
    std::function<void(  //
        const RowID&,
        const EntityName&,
        const TransactionDebit&,
        const TransactionCredit&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(
        u8R"(
SELECT
  entities.id,
  entities.name,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS credit
FROM
  transactions
  LEFT JOIN entities ON transactions.entity_id = entities.id
WHERE
  account_id = %u
GROUP BY
  entities.name
ORDER BY
  entities.name;
)",
        vAccountID);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get entities with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    auto row_id = sqlite3_column_int(stmt, 0);
                    auto name = (const char*)sqlite3_column_text(stmt, 1);
                    auto debit = (TransactionDebit)sqlite3_column_double(stmt, 2);
                    auto credit = (TransactionCredit)sqlite3_column_double(stmt, 3);
                    vCallback(row_id, name != nullptr ? name : "", debit, credit);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateEntity(const RowID& vRowID, const EntityName& vEntityName) {
    auto insert_query = ct::toStr(u8R"(UPDATE entities SET name = "%s" WHERE id = %u;)", vEntityName.c_str(), vRowID);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a entity in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteEntities() {
    if (m_OpenDB()) {
        auto insert_query = ct::toStr(u8R"(DELETE FROM entities;)");
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of entities table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::AddCategory(const CategoryName& vCategoryName) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO categories (name) VALUES("%s");)", vCategoryName.c_str());
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a category in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetCategory(const CategoryName& vUserName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT id FROM categories WHERE name = "%s";)", vUserName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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

void DataBase::GetCategoriesStats(  //
    const RowID& vAccountID,
    std::function<void(  //
        const RowID&,
        const CategoryName&,
        const TransactionDebit&,
        const TransactionCredit&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(
        u8R"(
SELECT
  categories.id,
  categories.name,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS credit
FROM
  transactions
  LEFT JOIN categories ON transactions.category_id = categories.id
WHERE
  account_id = %u
GROUP BY
  categories.name
ORDER BY
  categories.name;
)",
        vAccountID);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get categories with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    auto row_id = sqlite3_column_int(stmt, 0);
                    auto name = (const char*)sqlite3_column_text(stmt, 1);
                    auto debit = (TransactionDebit)sqlite3_column_double(stmt, 2);
                    auto credit = (TransactionCredit)sqlite3_column_double(stmt, 3);
                    vCallback(row_id, name != nullptr ? name : "", debit, credit);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateCategory(const RowID& vRowID, const CategoryName& vCategoryName) {
    auto insert_query = ct::toStr(u8R"(UPDATE categories SET name = "%s" WHERE id = %u;)", vCategoryName.c_str(), vRowID);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a category in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteCategories() {
    if (m_OpenDB()) {
        auto insert_query = ct::toStr(u8R"(DELETE FROM categories;)");
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of categories table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::AddOperation(const OperationName& vOperationName) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO operations (name) VALUES("%s");)", vOperationName.c_str());
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a operation in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetOperation(const OperationName& vOperationName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT id FROM operations WHERE name = "%s";)", vOperationName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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

void DataBase::GetOperationsStats(  //
    const RowID& vAccountID,
    std::function<void(  //
        const RowID&,
        const OperationName&,
        const TransactionDebit&,
        const TransactionCredit&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(
        u8R"(
SELECT
  operations.id,
  operations.name,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS credit
FROM
  transactions
  LEFT JOIN operations ON transactions.operation_id = operations.id
WHERE
  account_id = %u
GROUP BY
  operations.name
ORDER BY
  operations.name;
)",
        vAccountID);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get operations with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    auto row_id = sqlite3_column_int(stmt, 0);
                    auto name = (const char*)sqlite3_column_text(stmt, 1);
                    auto debit = (TransactionDebit)sqlite3_column_double(stmt, 2);
                    auto credit = (TransactionCredit)sqlite3_column_double(stmt, 3);
                    vCallback(row_id, name != nullptr ? name : "", debit, credit);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateOperation(const RowID& vRowID, const OperationName& vOperationName) {
    auto insert_query = ct::toStr(u8R"(UPDATE operations SET name = "%s" WHERE id = %u;)", vOperationName.c_str(), vRowID);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a operation in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteOperations() {
    if (m_OpenDB()) {
        auto insert_query = ct::toStr(u8R"(DELETE FROM operations;)");
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of operations table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::AddSource(const SourceName& vSourceName, const SourceType& vSourceType, const SourceSha& vSourceSha) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO sources (name, type, sha) VALUES("%s", "%s", "%s");)",  //
                                  vSourceName.c_str(),
                                  vSourceType.c_str(),
                                  vSourceSha.c_str());
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a source in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetSource(const SourceName& vSourceName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(u8R"(SELECT id FROM sources WHERE name = "%s";)", vSourceName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get source id with reason", sqlite3_errmsg(m_SqliteDB));
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

void DataBase::GetSources(std::function<void(const SourceName&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ct::toStr(u8R"(SELECT name FROM sources GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get sources with reason", sqlite3_errmsg(m_SqliteDB));
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

void DataBase::UpdateSource(const RowID& vRowID, const SourceName& vSourceName) {
    auto insert_query = ct::toStr(u8R"(UPDATE sources SET name = "%s" WHERE id = %u;)", vSourceName.c_str(), vRowID);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a source in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteSources() {
    if (m_OpenDB()) {
        auto insert_query = ct::toStr(u8R"(DELETE FROM sources;)");
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of sources table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::AddAccount(const BankName& vBankName,
                          const BankAgency& vBankAgency,
                          const AccountType& vAccountType,
                          const AccountName& vAccountName,
                          const AccountNumber& vAccountNumber,
                          const AccounBaseSolde& vBaseSolde) {
    AddBank(vBankName);
    auto insert_query = ct::toStr(
        u8R"(
INSERT OR IGNORE INTO accounts 
    (bank_id, bank_agency, type, name, number, base_solde) VALUES(
        (SELECT id FROM banks WHERE banks.name = "%s"), -- bank id
        "%s", -- bank agency
        "%s", -- account type
        "%s", -- account name
        "%s", -- account number
        %f    -- account base solde
        );)",
        vBankName.c_str(),
        vBankAgency.c_str(),
        vAccountType.c_str(),
        vAccountName.c_str(),
        vAccountNumber.c_str(),
        vBaseSolde);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a Bank Account in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetAccount(const AccountNumber& vAccountNumber, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(
        u8R"(
SELECT 
  id 
FROM 
  accounts 
WHERE 
  accounts.number = "%s"
;)",
        vAccountNumber.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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

bool DataBase::GetAccount(const BankName& vBankName,
                          const BankAgency& vBankAgency,
                          const AccountType& vAccountType,
                          const AccountName& vAccountName,
                          const AccountNumber& vAccountNumber,
                          RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ct::toStr(
        u8R"(
SELECT 
  id 
FROM 
  accounts 
  LEFT JOIN banks ON accounts.bank_id = banks.id
WHERE 
  AND banks.name = "%s"
  AND banks.agency = "%s"
  AND accounts.type = "%s"
  AND accounts.name = "%s"
  AND accounts.number = "%s"
;)",
        vBankName.c_str(),     //
        vBankAgency.c_str(),   //
        vAccountType.c_str(),  //
        vAccountName.c_str(),  //
        vAccountNumber.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
        const BankName&,
        const BankAgency&,
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
  accounts.id AS rowid,
  banks.name AS bank_name,
  accounts.bank_agency AS bank_agency,
  accounts.type AS account_type,
  accounts.name AS account_name,
  accounts.number AS account_number,
  accounts.base_solde AS account_base_solde,
  COUNT(t.id) AS nombre_transactions
FROM accounts
LEFT JOIN banks ON banks.id = accounts.bank_id
LEFT JOIN transactions t ON accounts.id = t.account_id
GROUP BY bank_name, bank_agency, account_name
ORDER BY accounts.id;
)";
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get accounts with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID account_id = sqlite3_column_int(stmt, 0);
                    const char* bank_name = (const char*)sqlite3_column_text(stmt, 1);
                    const char* bank_agency = (const char*)sqlite3_column_text(stmt, 2);
                    const char* account_type = (const char*)sqlite3_column_text(stmt, 3);
                    const char* account_name = (const char*)sqlite3_column_text(stmt, 4);
                    const char* account_number = (const char*)sqlite3_column_text(stmt, 5);
                    AccounBaseSolde account_base_solde = sqlite3_column_double(stmt, 6);
                    TransactionsCount transactions_count = sqlite3_column_int(stmt, 7);
                    vCallback(                                            //
                        account_id,                                       //
                        bank_name != nullptr ? bank_name : "",            //
                        bank_agency != nullptr ? bank_agency : "",        //
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
                             const BankName& vBankName,
                             const BankAgency& vBankAgency,
                             const AccountType& vAccountType,
                             const AccountName& vAccountName,
                             const AccountNumber& vAccountNumber,
                             const AccounBaseSolde& vBaseSolde) {
    auto insert_query = ct::toStr(
        u8R"(
UPDATE 
  accounts
SET 
  bank_id = (SELECT id FROM banks WHERE name = "%s"),
  type = "%s",
  name = "%s",
  number = "%s",
  base_solde = %f
WHERE
  accounts.id = %u;
)",
        vBankName.c_str(),
        vAccountType.c_str(),
        vAccountName.c_str(),
        vAccountNumber.c_str(),
        vBaseSolde,
        vRowID);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a account in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteAccount(const RowID& vRowID) {
    auto insert_query = ct::toStr(
        u8R"(
DELETE FROM 
  accounts
WHERE
  accounts.id = %u;
)",
        vRowID);
    if (m_OpenDB()) {
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete a account in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteAccounts() {
    if (m_OpenDB()) {
        auto insert_query = ct::toStr(u8R"(DELETE FROM accounts;)");
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of accounts table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::AddIncome(  //
    std::set<RowID> vAccountIDs,
    const IncomeName& vIncomeName,
    const EntityName& vEntityName,
    const CategoryName& vCategoryName,
    const OperationName& vOperationName,
    const IncomeDate& vStartDate,
    const IncomeDate& vEndDate,
    const IncomeAmount& vMinAmount,
    const IncomeAmount& vMaxAmount,
    const IncomeDelayDays& vMinDays,
    const IncomeDelayDays& vMaxDays,
    const IncomeHash& vHash) {
    if (!vAccountIDs.empty()) {
        AddEntity(vEntityName);
        AddOperation(vOperationName);
        AddCategory(vCategoryName);
        auto insert_query = ct::toStr(
            u8R"(
INSERT OR IGNORE INTO incomes 
(
    name, 
    entity_id,  
    category_id, 
    operation_id,
    start_date, 
    end_date, 
    min_amount, 
    max_amount,
    min_delay_days,
    max_delay_days,   
    hash
) 
VALUES 
(
    "%s", 
    (SELECT id FROM entities WHERE entities.name = "%s"), -- entity id
    (SELECT id FROM categories WHERE categories.name = "%s"), -- category id
    (SELECT id FROM operations WHERE operations.name = "%s"), -- operation id
    "%s", 
    "%s",
    "%.2f",
    "%.2f",
    "%i"
    "%i"
    "%s"
);
)",
            vIncomeName.c_str(),
            vEntityName.c_str(),
            vCategoryName.c_str(),
            vOperationName.c_str(),
            vStartDate.c_str(),
            vEndDate.c_str(),
            vMinAmount,
            vMaxAmount,
            vMinDays,
            vMaxDays,
            vHash.c_str());
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to insert a transaction in database : %s", m_LastErrorMsg);
        } else {
            const auto last_row_id = sqlite3_last_insert_rowid(m_SqliteDB);
            m_LinkOneIncomeWithManyAccounts(last_row_id, vAccountIDs);
        }
    }
}

void DataBase::AddTransaction(  //
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
    const TransactionHash& vHash) {
    AddEntity(vEntityName);
    AddOperation(vOperationName);
    AddCategory(vCategoryName);
    AddSource(vSourceName, vSourceType, vSourceSha);
    auto insert_query = ct::toStr(
        u8R"(
INSERT OR IGNORE INTO transactions 
    (account_id, entity_id, operation_id, category_id, source_id, date, description, comment, amount, confirmed, hash) VALUES(
        %u, -- account id
        (SELECT id FROM entities WHERE entities.name = "%s"), -- entity id
        (SELECT id FROM operations WHERE operations.name = "%s"), -- operation id
        (SELECT id FROM categories WHERE categories.name = "%s"), -- category id
        (SELECT id FROM sources WHERE sources.sha = "%s"), -- source id
        "%s", 
        "%s",
        "%s",
        %.6f,
        "%u",
        "%s"
        );)",
        vAccountID,
        vEntityName.c_str(),
        vOperationName.c_str(),
        vCategoryName.c_str(),
        vSourceSha.c_str(),
        vDate.c_str(),
        vDescription.c_str(),
        vComment.c_str(),
        vAmount,
        vConfirmed,
        vHash.c_str());
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a transaction in database : %s", m_LastErrorMsg);
    }
}

void DataBase::GetTransactions(  //
    const RowID& vAccountID,
    std::function<void(  //
        const RowID&,
        const EntityName&,
        const CategoryName&,
        const OperationName&,
        const SourceName&,
        const TransactionDate&,
        const TransactionDescription&,
        const TransactionComment&,
        const TransactionAmount&,
        const TransactionConfirmed&,
        const TransactionHash&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ct::toStr(
        u8R"(
SELECT
  transactions.id AS rowid,
  entities.name AS entity,
  categories.name AS category,
  operations.name AS operation,
  sources.name AS source,
  transactions.date,
  transactions.description,
  transactions.comment,
  transactions.amount,
  transactions.confirmed,
  transactions.hash
FROM
  transactions
  LEFT JOIN accounts ON transactions.account_id = accounts.id
  LEFT JOIN entities ON transactions.entity_id = entities.id
  LEFT JOIN categories ON transactions.category_id = categories.id
  LEFT JOIN operations ON transactions.operation_id = operations.id
  LEFT JOIN sources ON transactions.source_id = sources.id
WHERE
  transactions.account_id = %u
ORDER BY
  transactions.date;
)",
        vAccountID);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID id = sqlite3_column_int(stmt, 0);
                    auto entity = (const char*)sqlite3_column_text(stmt, 1);
                    auto category = (const char*)sqlite3_column_text(stmt, 2);
                    auto operation = (const char*)sqlite3_column_text(stmt, 3);
                    auto source = (const char*)sqlite3_column_text(stmt, 4);
                    auto date = (const char*)sqlite3_column_text(stmt, 5);
                    auto description = (const char*)sqlite3_column_text(stmt, 6);
                    auto comment = (const char*)sqlite3_column_text(stmt, 7);
                    TransactionAmount amount = sqlite3_column_double(stmt, 8);
                    TransactionConfirmed confirmed = sqlite3_column_int(stmt, 9);
                    auto hash = (const char*)sqlite3_column_text(stmt, 10);
                    vCallback(                                      //
                        id,                                         //
                        entity != nullptr ? entity : "",            //
                        category != nullptr ? category : "",        //
                        operation != nullptr ? operation : "",      //
                        source != nullptr ? source : "",            //
                        date != nullptr ? date : "",                //
                        description != nullptr ? description : "",  //
                        comment != nullptr ? comment : "",          //
                        amount,                                     //
                        confirmed,                                  //
                        hash);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

std::string DataBase::GetFormatDate(const DateFormat& vDateFormat) {
    std::string ret;
    switch (vDateFormat) {
        case DateFormat::DAYS: {
            ret = "%Y-%m-%d";
        } break;
        case DateFormat::MONTHS: {
            ret = "%Y-%m";
        } break;
        case DateFormat::YEARS: {
            ret = "%Y";
        } break;
        case DateFormat::Count:
        default: break;
    }
    return ret;
}

void DataBase::GetGroupedTransactions(  //
    const RowID& vAccountID,
    const GroupBy& vGroupBy,
    const DateFormat& vDateFormat,
    std::function<void(  //
        const RowID&,
        const TransactionDate&,
        const TransactionDescription&,
        const EntityName&,
        const CategoryName&,
        const OperationName&,
        const TransactionDebit&,
        const TransactionCredit&)> vCallback) {  // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& format_date = GetFormatDate(vDateFormat);
    std::string group_by;
    std::string order_by;
    switch (vGroupBy) {
        case GroupBy::DATES: {
            group_by = "new_date";
            order_by = "new_date";
        } break;
        case GroupBy::ENTITIES: {
            group_by = "new_entity";
            order_by = "new_entity";
        } break;
        case GroupBy::CATEGORIES: {
            group_by = "new_category";
            order_by = "new_category";
        } break;
        case GroupBy::OPERATIONS: {
            group_by = "new_operation";
            order_by = "new_operation";
        } break;
        case GroupBy::DESCRIPTIONS: {
            group_by = "new_description";
            order_by = "new_date";
        } break;
        case GroupBy::Count:
        default: break;
    }
    auto select_query = ct::toStr(
        u8R"(
SELECT
  transactions.id,
  strftime("%s", transactions.date) AS new_date,
  transactions.description AS new_description,
  entities.name AS new_entity,
  categories.name AS new_category,
  operations.name AS new_operation,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS new_debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS new_credit
FROM
  transactions
  LEFT JOIN entities ON transactions.entity_id = entities.id
  LEFT JOIN categories ON transactions.category_id = categories.id
  LEFT JOIN operations ON transactions.operation_id = operations.id
WHERE
  account_id = %u
GROUP BY
  %s
ORDER BY
  %s;
)",
        format_date.c_str(),
        vAccountID,
        group_by.c_str(),
        order_by.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID id = sqlite3_column_int(stmt, 0);
                    const char* date = nullptr;
                    if (vGroupBy == GroupBy::DATES) {
                        date = (const char*)sqlite3_column_text(stmt, 1);
                    }
                    const char* description = nullptr;
                    if (vGroupBy == GroupBy::DESCRIPTIONS) {
                        description = (const char*)sqlite3_column_text(stmt, 2);
                    }
                    const char* entity = nullptr;
                    if (vGroupBy == GroupBy::ENTITIES) {
                        entity = (const char*)sqlite3_column_text(stmt, 3);
                    }
                    const char* category = nullptr;
                    if (vGroupBy == GroupBy::CATEGORIES) {
                        category = (const char*)sqlite3_column_text(stmt, 4);
                    }
                    const char* operation = nullptr;
                    if (vGroupBy == GroupBy::OPERATIONS) {
                        operation = (const char*)sqlite3_column_text(stmt, 5);
                    }
                    TransactionDebit debit = sqlite3_column_double(stmt, 6);
                    TransactionCredit credit = sqlite3_column_double(stmt, 7);
                    vCallback(                                      //
                        id,                                         //
                        date != nullptr ? date : "",                //
                        description != nullptr ? description : "",  //
                        entity != nullptr ? entity : "",            //
                        category != nullptr ? category : "",        //
                        operation != nullptr ? operation : "",      //
                        debit,                                      //
                        credit);                                    //
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::GetDuplicateTransactionsOnDatesAndAmount(const RowID& vAccountID, std::function<void(const RowID&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ct::toStr(
        u8R"(
SELECT t.id
FROM transactions t
JOIN (
    SELECT date, amount
    FROM transactions
    WHERE account_id = %u
    GROUP BY date, amount
    HAVING COUNT(*) > 1
) AS duplicates
ON t.date = duplicates.date AND t.amount = duplicates.amount
WHERE t.account_id = %u
ORDER BY t.date, t.amount;
)",
        vAccountID,
        vAccountID);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID id = sqlite3_column_int(stmt, 0);
                    vCallback(id);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::GetUnConfirmedTransactions(const RowID& vAccountID, std::function<void(const RowID&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ct::toStr(
        u8R"(
SELECT 
  id
FROM 
  transactions
WHERE
  confirmed = 0
AND 
  account_id = %u;
)",
        vAccountID);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID id = sqlite3_column_int(stmt, 0);
                    vCallback(id);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateTransaction(  //
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
    const TransactionHash& vHash) {
    auto insert_query = ct::toStr(
        u8R"(
UPDATE 
  transactions
SET 
  entity_id = (SELECT id FROM entities WHERE name = "%s"),
  category_id = (SELECT id FROM categories WHERE name = "%s"),
  operation_id = (SELECT id FROM operations WHERE name = "%s"),
  source_id = (SELECT id FROM sources WHERE name = "%s"),
  date = "%s",
  description = "%s",
  comment = "%s",
  amount = %.6f,
  confirmed = %u,
  hash = "%s"
WHERE
  transactions.id = %u;
)",
        vEntityName.c_str(),
        vCategoryName.c_str(),
        vOperationName.c_str(),
        vSourceName.c_str(),
        vDate.c_str(),
        vDescription.c_str(),
        vComment.c_str(),
        vAmount,
        vConfirmed,
        vHash.c_str(),
        vRowID);
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a transaction in database : %s", m_LastErrorMsg);
    }
}

void DataBase::ConfirmTransaction(const RowID& vRowID, const TransactionConfirmed& vConfirmed) {
    auto insert_query = ct::toStr(
        u8R"(
UPDATE 
  transactions
SET 
  confirmed = %u
WHERE
  transactions.id = %u;
)",
        vConfirmed,
        vRowID);
    if (m_OpenDB()) {
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to confirm a transaction in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteTransaction(const RowID& vRowID) {
    auto insert_query = ct::toStr(
        u8R"(
DELETE FROM 
  transactions
WHERE
  transactions.id = %u;
)",
        vRowID);
    if (m_OpenDB()) {
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete a transaction in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteTransactions() {
    if (m_OpenDB()) {
        auto delete_query = ct::toStr(u8R"(DELETE FROM transactions;)");
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, delete_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of transactions table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteTransactions(const std::set<RowID>& vRowIDs) {
    if (BeginTransaction()) {
        for (const auto& row_id : vRowIDs) {
            auto insert_query = ct::toStr(
                u8R"(
DELETE FROM 
  transactions
WHERE
  transactions.id = %u;
)",
                row_id);
            if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
                LogVarError("Fail to delete a transaction in database : %s", m_LastErrorMsg);
            }
        }
        CommitTransaction();
    }
}

bool DataBase::m_EnableForeignKey() {
    if (!m_SqliteDB) {
        int res = debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("Erreur lors de l'activation des clés étrangères : %s", sqlite3_errmsg(m_SqliteDB));
        }
    }
    return (m_SqliteDB != nullptr);
}

bool DataBase::m_LinkOneIncomeWithManyAccounts(  //
    const RowID& vIncomeID,
    std::set<RowID> vAccountIDs) {
    if (!vAccountIDs.empty()) {
        std::string insert_query =
            u8R"(
INSERT OR IGNORE INTO income_accounts 
(
    income_id, 
    account_id
)
VALUES 
(
)";
        for (const auto& id : vAccountIDs) {
            insert_query += ct::toStr("\t(%u, %u)\n", vIncomeID, id);
        }
        insert_query += ");";
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to insert a transaction in database : %s", m_LastErrorMsg);
            return false;
        }
        return true;
    } else {
        LogVarError("The accounts ids are empty");
    }
    return false;
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
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
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
    if (debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query, (int)strlen(select_query), &stmt, nullptr) != SQLITE_OK) {
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
DELETE FROM banks;
DELETE FROM incomes;
DELETE FROM sources;
DELETE FROM accounts;
DELETE FROM entities;
DELETE FROM categories;
DELETE FROM operations;
DELETE FROM transactions;
DELETE FROM income_accounts;
COMMIT;
)";
    if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, clear_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
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
CREATE TABLE banks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    address TEXT,
    url TEXT
);

CREATE TABLE accounts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    number TEXT NOT NULL UNIQUE,
    bank_id INTEGER NOT NULL,
    bank_agency TEXT,
    type TEXT NOT NULL,
    name TEXT NOT NULL,
    base_solde REAL,
    FOREIGN KEY (bank_id) REFERENCES banks(id)
);

CREATE TABLE sources (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    type TEXT NOT NULL,
    sha TEXT NOT NULL
);

CREATE TABLE entities (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

-- type of bank transaction, like CB, SEPA, etc... 
CREATE TABLE operations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE incomes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    entity_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    operation_id INTEGER NOT NULL,
    start_date DATE NOT NULL, -- start date
    end_date DATE, -- end date (empty if no end)
    min_amount REAL NOT NULL, -- min amount
    max_amount REAL NOT NULL, -- max amount, can be the same as min
    min_delay_days INTEGER, -- delay in days, can be zero
    max_delay_days INTEGER, -- delay in days, can be the same as min
    hash TEXT NOT NULL UNIQUE,
    -- links
    FOREIGN KEY (entity_id) REFERENCES entities(id),
    FOREIGN KEY (category_id) REFERENCES categories(id),
    FOREIGN KEY (operation_id) REFERENCES operations(id),
    -- verifs
    CHECK (max_amount >= min_amount),
    CHECK (max_delay_days >= min_delay_days)
);

CREATE TABLE income_accounts (
    income_id INTEGER NOT NULL,
    account_id INTEGER NOT NULL,
    PRIMARY KEY (income_id, account_id),
    FOREIGN KEY (income_id) REFERENCES incomes(income_id),
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
);

CREATE TABLE transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id INTEGER NOT NULL,
    entity_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    operation_id INTEGER NOT NULL,
    source_id INTEGER NOT NULL,
    date DATE NOT NULL,
    description TEXT,
    comment TEXT,
    amount REAL NOT NULL,
    confirmed INTEGER,
    hash TEXT NOT NULL UNIQUE,
    FOREIGN KEY (account_id) REFERENCES accounts(id),
    FOREIGN KEY (operation_id) REFERENCES operations(id),
    FOREIGN KEY (category_id) REFERENCES categories(id),
    FOREIGN KEY (entity_id) REFERENCES entities(id),
    FOREIGN KEY (source_id) REFERENCES sources(id)
);

CREATE TABLE settings (
	xml TEXT
);
)";
        if (debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, create_tables, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
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
    if (!m_TransactionStarted) {
        if (m_SqliteDB) {
            if (sqlite3_close(m_SqliteDB) == SQLITE_BUSY) {
                // try to force closing
                sqlite3_close_v2(m_SqliteDB);
            }
        }
        m_SqliteDB = nullptr;
        // there is also sqlite3LeaveMutexAndCloseZombie when sqlite is stucked
    }
}
