#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

void DataBase::AddCategory(const CategoryName& vCategoryName) {
    auto insert_query =             //
        ez::sqlite::QueryBuilder()  //
            .setTable("categories")
            .addField("name", vCategoryName)
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a category in database : %s (%s)", m_LastErrorMsg, insert_query.c_str());
    }
}

bool DataBase::GetCategory(const CategoryName& vUserName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ez::str::toStr(u8R"(SELECT id FROM categories WHERE name = "%s";)", vUserName.c_str());
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

void DataBase::GetCategories(std::function<void(const CategoryName&)> vCallback) {
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
        const TransactionCredit&,
        const TransactionsCount&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ez::str::toStr(
        u8R"(
SELECT
  categories.id,
  categories.name,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS credit,
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
                    vCallback(
                        sqlite3_column_int(stmt, 0),                        // RowID
                        ez::sqlite::readStringColumn(stmt, 1),              // CategoryName
                        (TransactionDebit)sqlite3_column_double(stmt, 2),   // TransactionDebit
                        (TransactionCredit)sqlite3_column_double(stmt, 3),  // TransactionCredit
                        (TransactionsCount)sqlite3_column_int(stmt, 4)      // TransactionsCount
                    );
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateCategory(const RowID& vRowID, const CategoryName& vCategoryName) {
    auto insert_query = ez::str::toStr(u8R"(UPDATE categories SET name = "%s" WHERE id = %u;)", vCategoryName.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a category in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteCategories() {
    if (m_OpenDB()) {
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM categories;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of categories table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}
