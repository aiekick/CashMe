#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

bool DataBase::AddOperation(const OperationInput& vOperationInput) {
    bool ret = true;
    auto insert_query =             //
        ez::sqlite::QueryBuilder()  //
            .setTable("operations")
            .addOrSetField("name", vOperationInput.name)
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a entity in database : %s (%s)", m_LastErrorMsg, insert_query);
        ret = false;
    }
    return ret;
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

bool DataBase::GetOperations(std::function<void(const OperationOutput&)> vCallback) {
    bool ret = false;
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
                    OperationOutput ao;
                    ao.datas.name = ez::sqlite::readStringColumn(stmt, 0);
                    vCallback(ao);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetOperationsStats(const RowID& vAccountID, std::function<void(const OperationOutput&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ez::str::toStr(
        u8R"(
SELECT
  operations.id,
  operations.name,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS credit,
  SUM(transactions.amount),
  COUNT(transactions.id)
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
                    OperationOutput ao;
                    ao.id = sqlite3_column_int(stmt, 0);
                    ao.datas.name = ez::sqlite::readStringColumn(stmt, 1);
                    ao.amounts.debit = sqlite3_column_double(stmt, 2);
                    ao.amounts.credit = sqlite3_column_double(stmt, 3);
                    ao.amounts.amount = sqlite3_column_double(stmt, 4);
                    ao.count = sqlite3_column_int(stmt, 5);
                    vCallback(ao);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateOperation(const RowID& vRowID, const OperationInput& vOperationInput) {
    bool ret = false;
    auto insert_query = ez::str::toStr(u8R"(UPDATE operations SET name = "%s" WHERE id = %u;)", vOperationInput.name.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a operation in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}

bool DataBase::DeleteOperations() {
    bool ret = false;
    if (m_OpenDB()) {
        ret = true;
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM operations;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of operations table in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}
