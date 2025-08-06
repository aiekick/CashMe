#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

bool DataBase::AddEntity(const EntityInput& vEntityInput) {
    bool ret = true;
    auto insert_query =             //
        ez::sqlite::QueryBuilder()  //
            .setTable("entities")
            .addOrSetField("name", vEntityInput.name)
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a entity in database : %s (%s)", m_LastErrorMsg, insert_query);
        ret = false;
    }
    return ret;
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

bool DataBase::GetEntities(std::function<void(const EntityOutput&)> vCallback) {
    bool ret = false;
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
                    EntityOutput eo;
                    eo.datas.name =  ez::sqlite::readStringColumn(stmt, 0);
                    vCallback(  eo);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetEntitiesStats(const RowID& vAccountID, std::function<void(const EntityOutput&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ez::str::toStr(
        u8R"(
SELECT
  entities.id,
  entities.name,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS credit,
  SUM(transactions.amount),
  COUNT(transactions.id)
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
                    EntityOutput eo;
                    eo.id = sqlite3_column_int(stmt, 0);
                    eo.datas.name = ez::sqlite::readStringColumn(stmt, 1);
                    eo.amounts.debit = sqlite3_column_double(stmt, 2);
                    eo.amounts.credit = sqlite3_column_double(stmt, 3);
                    eo.amounts.amount = sqlite3_column_double(stmt, 4);
                    eo.count = sqlite3_column_int(stmt, 5);
                    vCallback(eo);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateEntity(const RowID& vRowID, const EntityInput& vEntityInput) {
    bool ret = true;
    auto insert_query = ez::str::toStr(u8R"(UPDATE entities SET name = "%s" WHERE id = %u;)", vEntityInput.name.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a entity in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}

bool DataBase::DeleteEntities() {
    bool ret = false;
    if (m_OpenDB()) {
        ret = true;
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM entities;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of entities table in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}
