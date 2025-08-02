#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

void DataBase::AddOperation(const OperationName& vOperationName) {
    auto insert_query = ez::str::toStr(u8R"(INSERT OR IGNORE INTO operations (name) VALUES("%s");)", vOperationName.c_str());
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a operation in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetOperation(const OperationName& vOperationName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ez::str::toStr(u8R"(SELECT id FROM operations WHERE name = "%s";)", vOperationName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    const auto& select_query = ez::str::toStr(u8R"(SELECT name FROM operations GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    const auto& select_query = ez::str::toStr(
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    auto insert_query = ez::str::toStr(u8R"(UPDATE operations SET name = "%s" WHERE id = %u;)", vOperationName.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a operation in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteOperations() {
    if (m_OpenDB()) {
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM operations;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of operations table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}
