#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

bool DataBase::AddCategory(const CategoryInput& vCategoryInput) {
    bool ret = true;
    auto insert_query =             //
        ez::sqlite::QueryBuilder()  //
            .setTable("categories")
            .addOrSetField("name", vCategoryInput.name)
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a category in database : %s (%s)", m_LastErrorMsg, insert_query);
        ret = false;
    }
    return ret;
}

bool DataBase::GetCategory(const CategoryName& vCategoryName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ez::str::toStr(u8R"(SELECT id FROM categories WHERE name = "%s";)", vCategoryName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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

bool DataBase::GetCategories(std::function<void(const CategoryOutput&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ez::str::toStr(u8R"(SELECT name FROM categories GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get categories with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    CategoryOutput co;
                    co.datas.name = ez::sqlite::readStringColumn(stmt, 0);
                    vCallback(co);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetCategoriesStats(  
    const RowID& vAccountID,
    std::function<void(  
        const CategoryOutput&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ez::str::toStr(
        u8R"(
SELECT
  categories.id,
  categories.name,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS credit,
  SUM(transactions.amount),
  COUNT(transactions.id)
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get categories with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    CategoryOutput co;
                    co.id = sqlite3_column_int(stmt, 0);
                    co.datas.name = ez::sqlite::readStringColumn(stmt, 1);
                    co.amounts.debit = sqlite3_column_double(stmt, 2);
                    co.amounts.credit = sqlite3_column_double(stmt, 3);
                    co.amounts.amount = sqlite3_column_double(stmt, 4);
                    co.count = sqlite3_column_int(stmt, 4);
                    vCallback(co);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateCategory(const RowID& vRowID, const CategoryInput& vCategoryInput) {
    bool ret = true;
    auto insert_query = ez::str::toStr(u8R"(UPDATE categories SET name = "%s" WHERE id = %u;)", vCategoryInput.name.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a category in database : %s", m_LastErrorMsg);
        ret = true;
    }
    return ret;
}

bool DataBase::DeleteCategories() {
    bool ret = false;
    if (m_OpenDB()) {
        ret = true;
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM categories;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of categories table in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}
