#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

void DataBase::AddEntity(const EntityName& vEntityName) {
    auto insert_query = ez::str::toStr(u8R"(INSERT OR IGNORE INTO entities (name) VALUES("%s");)", vEntityName.c_str());
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a entity in database : %s", m_LastErrorMsg);
    }
}

bool DataBase::GetEntity(const EntityName& vUserName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ez::str::toStr(u8R"(SELECT id FROM entities WHERE name = "%s";)", vUserName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    const auto& select_query = ez::str::toStr(u8R"(SELECT name FROM entities GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    const auto& select_query = ez::str::toStr(
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    auto insert_query = ez::str::toStr(u8R"(UPDATE entities SET name = "%s" WHERE id = %u;)", vEntityName.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a entity in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteEntities() {
    if (m_OpenDB()) {
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM entities;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of entities table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}
