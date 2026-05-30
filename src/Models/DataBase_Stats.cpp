#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <string>

// returns the sql column expression for a given categorical search column
static std::string getFieldColumnExpr(const SearchColumns& vField) {
    switch (vField) {
        case SEARCH_COLUMN_DESCRIPTION: return "transactions.description";
        case SEARCH_COLUMN_COMMENT: return "transactions.comment";
        case SEARCH_COLUMN_ENTITY: return "entities.name";
        case SEARCH_COLUMN_CATEGORY: return "categories.name";
        case SEARCH_COLUMN_OPERATION: return "operations.name";
        default: return "categories.name";
    }
}

bool DataBase::GetBuySellStats(  //
    const RowID& vAccountID,
    const SearchColumns& vField,
    const std::string& vFilterText,
    const bool vUseDayRange,
    const int32_t& vDayStart,
    const int32_t& vDayEnd,
    std::function<void(const BuySellStatItem&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);

    const std::string fieldExpr = getFieldColumnExpr(vField);

    // the query is built by concatenation on purpose : it contains strftime format
    // specifiers (%Y, %m, %d) that a printf-like builder (ez::str::toStr) would eat
    std::string select_query;
    select_query += "SELECT\n";
    select_query += "  strftime('%Y/%m', transactions.date) AS month,\n";
    select_query += "  unixepoch(strftime('%Y-%m-01', transactions.date)) AS epoch,\n";
    select_query += "  " + fieldExpr + " AS grp,\n";
    select_query += "  ROUND(SUM(transactions.amount), 2) AS amount\n";
    select_query += "FROM transactions\n";
    select_query += "  LEFT JOIN entities ON transactions.entity_id = entities.id\n";
    select_query += "  LEFT JOIN categories ON transactions.category_id = categories.id\n";
    select_query += "  LEFT JOIN operations ON transactions.operation_id = operations.id\n";
    select_query += "WHERE transactions.account_id = " + std::to_string(vAccountID) + "\n";
    if (!vFilterText.empty()) {
        select_query += "  AND " + fieldExpr + " LIKE \"%" + vFilterText + "%\"\n";
    }
    if (vUseDayRange) {
        select_query += "  AND CAST(strftime('%d', transactions.date) AS INTEGER) BETWEEN " +  //
            std::to_string(vDayStart) + " AND " + std::to_string(vDayEnd) + "\n";
    }
    select_query += "GROUP BY month, grp\n";
    select_query += "ORDER BY month, grp;\n";

    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail to get buy/sell stats with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    BuySellStatItem item;
                    item.month = ez::sqlite::readStringColumn(stmt, 0);
                    item.epoch = sqlite3_column_int64(stmt, 1);
                    item.group = ez::sqlite::readStringColumn(stmt, 2);
                    item.amount = sqlite3_column_double(stmt, 3);
                    vCallback(item);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}
