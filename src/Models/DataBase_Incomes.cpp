#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezSha.hpp>

bool DataBase::AddIncome(const RowID& vAccountID, const IncomeInput& vIncomeInput) {
    bool ret = true;
    EntityInput ei;
    ei.name = vIncomeInput.entity.name;
    AddEntity(ei);
    OperationInput oi;
    oi.name = vIncomeInput.operation.name;
    AddOperation(oi);
    CategoryInput ci;
    ci.name = vIncomeInput.category.name;
    AddCategory(ci);
    auto insert_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("incomes")
            .addOrSetField("account_id", vAccountID)
            .addOrSetField("name", vIncomeInput.name)
            .addOrSetFieldQuery("entity_id", R"(SELECT id FROM entities WHERE entities.name = "%s")", vIncomeInput.entity.name.c_str())
            .addOrSetFieldQuery("category_id", R"(SELECT id FROM categories WHERE categories.name = "%s")", vIncomeInput.category.name.c_str())
            .addOrSetFieldQuery("operation_id", R"(SELECT id FROM operations WHERE operations.name = "%s")", vIncomeInput.operation.name.c_str())
            .addOrSetField("start_date", vIncomeInput.startDate)
            .addOrSetField("end_date", vIncomeInput.endDate)
            .addOrSetField("min_amount", vIncomeInput.minAmount)
            .addOrSetField("max_amount", vIncomeInput.maxAmount)
            .addOrSetField("min_day", vIncomeInput.minDay)
            .addOrSetField("max_day", vIncomeInput.maxDay)
            .addOrSetField("description", vIncomeInput.description)
            .addOrSetField("optional", vIncomeInput.optional)
            .addOrSetField(
                "sha",
                ez::sha1()
                    .addValue(vAccountID)
                    .add(vIncomeInput.name)
                    .add(vIncomeInput.entity.name)
                    .add(vIncomeInput.category.name)
                    .add(vIncomeInput.operation.name)
                    .add(vIncomeInput.startDate)
                    .add(vIncomeInput.endDate)
                    .addValue(vIncomeInput.minAmount)
                    .addValue(vIncomeInput.maxAmount)
                    .addValue(vIncomeInput.minDay)
                    .addValue(vIncomeInput.maxDay)
                    .add(vIncomeInput.description)
                    .finalize()
                    .getHex())
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert an income in database : %s (%s)", m_LastErrorMsg, insert_query);
        ret = false;
    }
    return ret;
}

bool DataBase::GetIncomes(const RowID& vAccountID, std::function<void(const IncomeOutput&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ez::str::toStr(
        u8R"(
SELECT
    incomes.id AS rowid,
    incomes.account_id AS account_id,
    incomes.name AS name,
    entities.name AS entity,
    categories.name AS category,
    operations.name AS operation,
    incomes.start_date AS start_date, 
    unixepoch(incomes.start_date) AS start_epoch,
    incomes.end_date AS end_date, 
    unixepoch(incomes.end_date) AS start_epoch,
    incomes.min_amount AS min_amount, 
    incomes.max_amount AS max_amount,
    incomes.min_day AS min_day,
    incomes.max_day AS max_day,
    incomes.description AS description,
    incomes.optional AS optional
FROM
    incomes
    LEFT JOIN accounts ON incomes.account_id = accounts.id
    LEFT JOIN entities ON incomes.entity_id = entities.id
    LEFT JOIN categories ON incomes.category_id = categories.id
    LEFT JOIN operations ON incomes.operation_id = operations.id
WHERE
    incomes.account_id = %u
ORDER BY
    incomes.start_date,
    incomes.end_date
;
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
                    IncomeOutput io;
                    io.id = sqlite3_column_int(stmt, 0);
                    io.datas.account_id = sqlite3_column_int(stmt, 1);
                    io.datas.name = ez::sqlite::readStringColumn(stmt, 2);
                    io.datas.entity.name = ez::sqlite::readStringColumn(stmt, 3);
                    io.datas.category.name = ez::sqlite::readStringColumn(stmt, 4);
                    io.datas.operation.name = ez::sqlite::readStringColumn(stmt, 5);
                    io.datas.startDate = ez::sqlite::readStringColumn(stmt, 6);
                    io.startEpoch = sqlite3_column_int64(stmt, 7);
                    io.datas.endDate = ez::sqlite::readStringColumn(stmt, 8);
                    io.endEpoch = sqlite3_column_int64(stmt, 9);
                    io.datas.minAmount = sqlite3_column_double(stmt, 10);
                    io.datas.maxAmount = sqlite3_column_double(stmt, 11);
                    io.datas.minDay = sqlite3_column_int(stmt, 12);
                    io.datas.maxDay = sqlite3_column_int(stmt, 13);
                    io.datas.description = ez::sqlite::readStringColumn(stmt, 14);
                    io.datas.optional = sqlite3_column_int(stmt, 15);
                    vCallback(io);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateIncome(const RowID& vRowID, const IncomeInput& vIncomeInput) {
    bool ret = true;
    auto insert_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("incomes")
            .addOrSetField("account_id", vIncomeInput.account_id)
            .addOrSetField("name", vIncomeInput.name)
            .addOrSetFieldQuery("entity_id", R"(SELECT id FROM entities WHERE entities.name = "%s")", vIncomeInput.entity.name.c_str())
            .addOrSetFieldQuery("category_id", R"(SELECT id FROM categories WHERE categories.name = "%s")", vIncomeInput.category.name.c_str())
            .addOrSetFieldQuery("operation_id", R"(SELECT id FROM operations WHERE operations.name = "%s")", vIncomeInput.operation.name.c_str())
            .addOrSetField("start_date", vIncomeInput.startDate)
            .addOrSetField("end_date", vIncomeInput.endDate)
            .addOrSetField("min_amount", vIncomeInput.minAmount)
            .addOrSetField("max_amount", vIncomeInput.maxAmount)
            .addOrSetField("min_day", vIncomeInput.minDay)
            .addOrSetField("max_day", vIncomeInput.maxDay)
            .addOrSetField("description", vIncomeInput.description)
            .addOrSetField("comment", vIncomeInput.comment)
            .addOrSetField("optional", vIncomeInput.optional)
            .addOrSetField(
                "sha",
                ez::sha1()
                    .addValue(vIncomeInput.account_id)
                    .add(vIncomeInput.name)
                    .add(vIncomeInput.entity.name)
                    .add(vIncomeInput.category.name)
                    .add(vIncomeInput.operation.name)
                    .add(vIncomeInput.startDate)
                    .add(vIncomeInput.endDate)
                    .addValue(vIncomeInput.minAmount)
                    .addValue(vIncomeInput.maxAmount)
                    .addValue(vIncomeInput.minDay)
                    .addValue(vIncomeInput.maxDay)
                    .add(vIncomeInput.description)
                    .finalize()
                    .getHex())
            .addWhere(R"(incomes.id = %u)", vRowID)
            .build(ez::sqlite::QueryType::UPDATE);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a account in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}

bool DataBase::SearchIncomeInTransactions(const TransactionInput& vTransactionInput, RowID& vOutRowID) {
    bool ret = false;
    return ret;
}

bool DataBase::setIncomeAsOptional(const RowID& vRowID, const bool& vOptional) {
    bool ret = false;
    auto insert_query = ez::str::toStr(
        u8R"(
UPDATE 
  incomes
SET 
  optional = %u
WHERE
  id = %u;
)",
        vOptional,
        vRowID);
    if (m_OpenDB()) {
        ret = true;
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to set income as optional (%s) in database : %s", vOptional ? "true" : "false", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}

bool DataBase::DeleteIncome(const RowID& vRowID) {
    bool ret = false;
    auto insert_query = ez::str::toStr(
        u8R"(
DELETE FROM 
  incomes
WHERE
  id = %u;
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

bool DataBase::DeleteIncomes(const std::set<RowID>& vRowIDs) {
    bool ret = false;
    if (BeginDBTransaction()) {
        ret = true;
        for (const auto& row_id : vRowIDs) {
            ret &= DeleteIncome(row_id);
        }
        CommitDBTransaction();
    }
    return ret;
}

bool DataBase::DeleteIncomes() {
    bool ret = false;
    if (m_OpenDB()) {
        ret = true;
        auto delete_query = ez::str::toStr(u8R"(DELETE FROM incomes;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, delete_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of transactions table in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}
