#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezSha.hpp>

bool DataBase::AddTransaction(const RowID& vAccountID, const TransactionInput& vTransactionInput) {
    bool ret = true;
    AddEntity(vTransactionInput.entity);
    AddOperation(vTransactionInput.operation);
    AddCategory(vTransactionInput.category);
    AddSource(vTransactionInput.source.name, vTransactionInput.source.type, vTransactionInput.source.sha);
    RowID incomeID{};
    if (SearchIncomeInTransactions(vTransactionInput, incomeID)) {
    }
    auto insert_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("transactions")
            .addOrSetField("account_id", vAccountID)
            .addOrSetFieldQuery("entity_id", R"(SELECT id FROM entities WHERE entities.name = "%s")", vTransactionInput.entity.name.c_str())
            .addOrSetFieldQuery("operation_id", R"(SELECT id FROM operations WHERE operations.name = "%s")", vTransactionInput.operation.name.c_str())
            .addOrSetFieldQuery("category_id", R"(SELECT id FROM categories WHERE categories.name = "%s")", vTransactionInput.category.name.c_str())
            .addOrSetFieldQuery("source_id", R"(SELECT id FROM sources WHERE sources.sha = "%s")", vTransactionInput.source.sha.c_str())
            //.addOrSetFieldQuery("income_id", R"(SELECT id FROM incomes WHERE incomes.sha = "%s")", vTransactionInput.income.sha.c_str())
            .addOrSetField("date", vTransactionInput.date)
            .addOrSetField("description", vTransactionInput.description)
            .addOrSetField("comment", vTransactionInput.comment)
            .addOrSetField("amount", vTransactionInput.amount)
            .addOrSetField("confirmed", vTransactionInput.confirmed)
            .addOrSetField("sha", vTransactionInput.sha)
            .build(ez::sqlite::QueryType::INSERT);
    const auto rc = m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg);
    if (rc != SQLITE_OK) {
        LogVarError("Fail to insert a transaction in database : %s (%s)", m_LastErrorMsg, insert_query);
        ret = false;
    } 
    return ret;
}

bool DataBase::AddTransactionAutoSha(const RowID& vAccountID, const TransactionInput& vTransactionInput) {
    bool ret = true;
    AddEntity(vTransactionInput.entity);
    AddOperation(vTransactionInput.operation);
    AddCategory(vTransactionInput.category);
    AddSource(vTransactionInput.source.name, vTransactionInput.source.type, vTransactionInput.source.sha);
    RowID incomeID{};
    if (SearchIncomeInTransactions(vTransactionInput, incomeID)) {
    }
    auto insertBuilder =  //
        ez::sqlite::QueryBuilder()
            .setTable("transactions")
            .addOrSetField("account_id", vAccountID)
            .addOrSetFieldQuery("entity_id", R"(SELECT id FROM entities WHERE entities.name = "%s")", vTransactionInput.entity.name.c_str())
            .addOrSetFieldQuery("operation_id", R"(SELECT id FROM operations WHERE operations.name = "%s")", vTransactionInput.operation.name.c_str())
            .addOrSetFieldQuery("category_id", R"(SELECT id FROM categories WHERE categories.name = "%s")", vTransactionInput.category.name.c_str())
            .addOrSetFieldQuery("source_id", R"(SELECT id FROM sources WHERE sources.sha = "%s")", vTransactionInput.source.sha.c_str())
            //.addOrSetFieldQuery("income_id", R"(SELECT id FROM incomes WHERE incomes.sha = "%s")", vTransactionInput.income.sha.c_str())
            .addOrSetField("date", vTransactionInput.date)
            .addOrSetField("description", vTransactionInput.description)
            .addOrSetField("comment", vTransactionInput.comment)
            .addOrSetField("amount", vTransactionInput.amount)
            .addOrSetField("confirmed", vTransactionInput.confirmed);
    std::string baseSha =  //
        ez::sha1()         //
            .add(vTransactionInput.date)
            // un fichier ofc ne peut pas avoir des description de longueur > a 30
            // alors on limite le sha a utiliser un description de 30
            // comme cela un ofc ne rentrera pas un collision avec un autre type de fcihier comme les pdf par ex
            .add(vTransactionInput.description.substr(0, 30))
            // must be unique per oepration
            .addValue(vTransactionInput.amount)
            .finalize()
            .getHex();
    uint32_t attempt = 0;
    while (attempt < m_maxInsertAttempts) {
        const auto& insert_query = insertBuilder.addOrSetField("sha", baseSha + "_" + std::to_string(attempt)).build(ez::sqlite::QueryType::INSERT);
        const auto rc = m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg);
        if (rc == SQLITE_OK) {
            ret = true;
            break;
        } else {
            if ((rc == SQLITE_CONSTRAINT) &&  //
                (sqlite3_extended_errcode(m_SqliteDB) == SQLITE_CONSTRAINT_UNIQUE)) {
                ++attempt;  // retry abd we will increment suffix
            } else {
                LogVarError("Fail to insert a transaction in database : %s (%s)", m_LastErrorMsg, insert_query);
                ret = false;
                break;
            }
        }
    }
    if (attempt == m_maxInsertAttempts) {
        LogVarError("Fail to insert transaction: more than (%u) collisions on sha (%s)", m_maxInsertAttempts, baseSha.c_str());
        ret = false;
    }
    return ret;
}

bool DataBase::GetTransactions(const RowID& vAccountID, std::function<void(const TransactionOutput&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    if (vAccountID == 0) {
        return ret;
    }
    assert(vCallback);
    auto select_query = ez::str::toStr(
        u8R"(
SELECT
  transactions.id AS rowid,
  entities.name AS entity,
  categories.name AS category,
  operations.name AS operation,
  sources.name AS source,
  transactions.date,
  unixepoch(transactions.date) AS epoch,
  transactions.description,
  transactions.comment,
  transactions.amount,
  transactions.confirmed,
  transactions.sha
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    TransactionOutput to;
                    to.id = sqlite3_column_int(stmt, 0);
                    to.datas.entity.name = ez::sqlite::readStringColumn(stmt, 1);
                    to.datas.category.name = ez::sqlite::readStringColumn(stmt, 2);
                    to.datas.operation.name = ez::sqlite::readStringColumn(stmt, 3);
                    to.datas.source.name = ez::sqlite::readStringColumn(stmt, 4);
                    to.datas.date = ez::sqlite::readStringColumn(stmt, 5);
                    to.dateEpoch = sqlite3_column_int64(stmt, 6);
                    to.datas.description = ez::sqlite::readStringColumn(stmt, 7);
                    to.datas.comment = ez::sqlite::readStringColumn(stmt, 8);
                    to.datas.amount = sqlite3_column_double(stmt, 9);
                    to.datas.confirmed = static_cast<bool>(!!sqlite3_column_int(stmt, 10));
                    to.datas.sha = ez::sqlite::readStringColumn(stmt, 11);
                    vCallback(to);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
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

bool DataBase::GetGroupedTransactions(  //
    const RowID& vAccountID,
    const GroupBy& vGroupBy,
    const DateFormat& vDateFormat,
    std::function<void(  //
        const TransactionOutput&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    if (vAccountID == 0) {
        return ret;
    }
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
    auto select_query = ez::str::toStr(
        u8R"(
SELECT
  transactions.id,
  strftime("%s", transactions.date) AS new_date,
  transactions.description AS new_description,
  entities.name AS new_entity,
  categories.name AS new_category,
  operations.name AS new_operation,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN amount ELSE 0 END), 2) AS new_debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN amount ELSE 0 END), 2) AS new_credit,
  SUM(transactions.amount) AS new_amounx
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    TransactionOutput to;
                    to.id = sqlite3_column_int(stmt, 0);
                    if (vGroupBy == GroupBy::DATES) {
                        to.datas.date = ez::sqlite::readStringColumn(stmt, 1);
                    }
                    if (vGroupBy == GroupBy::DESCRIPTIONS) {
                        to.datas.description = ez::sqlite::readStringColumn(stmt, 2);
                    }
                    if (vGroupBy == GroupBy::ENTITIES) {
                        to.datas.entity.name = ez::sqlite::readStringColumn(stmt, 3);
                    }
                    if (vGroupBy == GroupBy::CATEGORIES) {
                        to.datas.category.name = ez::sqlite::readStringColumn(stmt, 4);
                    }
                    if (vGroupBy == GroupBy::OPERATIONS) {
                        to.datas.operation.name = ez::sqlite::readStringColumn(stmt, 5);
                    }
                    to.amounts.debit = sqlite3_column_double(stmt, 6);
                    to.amounts.credit = sqlite3_column_double(stmt, 7);
                    to.amounts.amount = sqlite3_column_double(stmt, 8);
                    vCallback(to);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetDuplicateTransactionsOnDatesAndAmount(const RowID& vAccountID, std::function<void(const RowID&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ez::str::toStr(
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID id = sqlite3_column_int(stmt, 0);
                    vCallback(id);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetUnConfirmedTransactions(const RowID& vAccountID, std::function<void(const RowID&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ez::str::toStr(
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get transactions with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    RowID id = sqlite3_column_int(stmt, 0);
                    vCallback(id);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateTransaction(const RowID& vRowID, const TransactionInput& vTransactionInput) {
    bool ret = true;
    auto insert_query = ez::str::toStr(
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
  sha = "%s"
WHERE
  transactions.id = %u;
)",
        vTransactionInput.entity.name.c_str(),
        vTransactionInput.category.name.c_str(),
        vTransactionInput.operation.name.c_str(),
        vTransactionInput.source.name.c_str(),
        vTransactionInput.date.c_str(),
        vTransactionInput.description.c_str(),
        vTransactionInput.comment.c_str(),
        vTransactionInput.amount,
        vTransactionInput.confirmed,
        vTransactionInput.sha.c_str(),
        vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a transaction in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}

bool DataBase::ConfirmTransaction(const RowID& vRowID, const bool& vConfirmed) {
    bool ret = false;
    auto insert_query = ez::str::toStr(
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
        ret = true;
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to confirm a transaction in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}

bool DataBase::DeleteTransaction(const RowID& vRowID) {
    bool ret = false;
    auto insert_query = ez::str::toStr(
        u8R"(
DELETE FROM 
  transactions
WHERE
  transactions.id = %u;
)",
        vRowID);
    if (m_OpenDB()) {
        ret = true;
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete a transaction in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}

bool DataBase::DeleteTransactions() {
    bool ret = false;
    if (m_OpenDB()) {
        ret = true;
        auto delete_query = ez::str::toStr(u8R"(DELETE FROM transactions;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, delete_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of transactions table in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}

bool DataBase::DeleteTransactions(const std::set<RowID>& vRowIDs) {
    bool ret = false;
    if (BeginDBTransaction()) {
        ret = true;
        for (const auto& row_id : vRowIDs) {
            auto insert_query = ez::str::toStr(
                u8R"(
DELETE FROM 
  transactions
WHERE
  transactions.id = %u;
)",
                row_id);
            if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
                LogVarError("Fail to delete a transaction in database : %s", m_LastErrorMsg);
                ret = false;
            }
        }
        CommitDBTransaction();
    }
    return ret;
}