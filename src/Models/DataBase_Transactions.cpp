#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

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
    auto insert_query =             //
        ez::sqlite::QueryBuilder()  //
            .setTable("transactions")
            .addField("account_id", ez::str::toStr("%u", vAccountID))
            .addFieldQuery("entity_id", R"(SELECT id FROM entities WHERE entities.name = "%s")", vEntityName.c_str())
            .addFieldQuery("operation_id", R"(SELECT id FROM operations WHERE operations.name = "%s")", vOperationName.c_str())
            .addFieldQuery("category_id", R"(SELECT id FROM categories WHERE categories.name = "%s")", vCategoryName.c_str())
            .addFieldQuery("source_id", R"(SELECT id FROM sources WHERE sources.sha = "%s")", vSourceSha.c_str())
            .addField("date", vDate)
            .addField("description", vDescription)
            .addField("comment", vComment)
            .addField("amount", ez::str::toStr("%.6f", vAmount))
            .addField("confirmed", ez::str::toStr("%u", vConfirmed))
            .addField("hash", vHash)
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a transaction in database : %s (%s)", m_LastErrorMsg, insert_query.c_str());
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
        const TransactionDateEpoch&,
        const TransactionDescription&,
        const TransactionComment&,
        const TransactionAmount&,
        const TransactionConfirmed&,
        const TransactionHash&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
                    TransactionDateEpoch date_epoch = sqlite3_column_int(stmt, 6);
                    auto description = (const char*)sqlite3_column_text(stmt, 7);
                    auto comment = (const char*)sqlite3_column_text(stmt, 8);
                    TransactionAmount amount = sqlite3_column_double(stmt, 9);
                    TransactionConfirmed confirmed = sqlite3_column_int(stmt, 10);
                    auto hash = (const char*)sqlite3_column_text(stmt, 11);
                    vCallback(                                      //
                        id,                                         //
                        entity != nullptr ? entity : "",            //
                        category != nullptr ? category : "",        //
                        operation != nullptr ? operation : "",      //
                        source != nullptr ? source : "",            //
                        date != nullptr ? date : "",                //
                        date_epoch,                                 //
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
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
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
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a transaction in database : %s", m_LastErrorMsg);
    }
}

void DataBase::ConfirmTransaction(const RowID& vRowID, const TransactionConfirmed& vConfirmed) {
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
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to confirm a transaction in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteTransaction(const RowID& vRowID) {
    auto insert_query = ez::str::toStr(
        u8R"(
DELETE FROM 
  transactions
WHERE
  transactions.id = %u;
)",
        vRowID);
    if (m_OpenDB()) {
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete a transaction in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteTransactions() {
    if (m_OpenDB()) {
        auto delete_query = ez::str::toStr(u8R"(DELETE FROM transactions;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, delete_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of transactions table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteTransactions(const std::set<RowID>& vRowIDs) {
    if (BeginTransaction()) {
        for (const auto& row_id : vRowIDs) {
            auto insert_query = ez::str::toStr(
                u8R"(
DELETE FROM 
  transactions
WHERE
  transactions.id = %u;
)",
                row_id);
            if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
                LogVarError("Fail to delete a transaction in database : %s", m_LastErrorMsg);
            }
        }
        CommitTransaction();
    }
}