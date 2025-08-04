#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezSha.hpp>

void DataBase::AddIncome(
    const RowID& vAccountID,
    const IncomeName& vIncomeName,
    const EntityName& vEntityName,
    const CategoryName& vCategoryName,
    const OperationName& vOperationName,
    const IncomeDate& vStartDate,
    const IncomeDate& vEndDate,
    const IncomeAmount& vMinAmount,
    const IncomeAmount& vMaxAmount,
    const IncomeDay& vMinDays,
    const IncomeDay& vMaxDays,
    const IncomeDescription& vDescription) {
    EntityInput ei;
    ei.name = vEntityName;
    AddEntity(ei);
    OperationInput oi;
    oi.name = vOperationName;
    AddOperation(oi);
    CategoryInput ci;
    ci.name = vCategoryName;
    AddCategory(ci);

    auto insert_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("incomes")
            .addField("account_id", vAccountID)
            .addField("name", vIncomeName)
            .addFieldQuery("entity_id", R"(SELECT id FROM entities WHERE entities.name = "%s")", vEntityName.c_str())
            .addFieldQuery("category_id", R"(SELECT id FROM categories WHERE categories.name = "%s")", vCategoryName.c_str())
            .addFieldQuery("operation_id", R"(SELECT id FROM operations WHERE operations.name = "%s")", vOperationName.c_str())
            .addField("start_date", vStartDate)
            .addField("end_date", vEndDate)
            .addField("min_amount", vMinAmount)
            .addField("max_amount", vMaxAmount)
            .addField("min_day", vMinDays)
            .addField("max_day", vMaxDays)
            .addField("description", vDescription)
            .addField(
                "sha",
                ez::sha1()
                    .addValue(vAccountID)
                    .add(vIncomeName)
                    .add(vEntityName)
                    .add(vCategoryName)
                    .add(vOperationName)
                    .add(vStartDate)
                    .add(vEndDate)
                    .addValue(vMinAmount)
                    .addValue(vMaxAmount)
                    .addValue(vMinDays)
                    .addValue(vMaxDays)
                    .add(vDescription)
                    .finalize()
                    .getHex())
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert an income in database : %s (%s)", m_LastErrorMsg, insert_query.c_str());
    }
}

void DataBase::GetIncomes(  //
    const RowID& vAccountID,
    std::function<void(  //
        const RowID&,
        const IncomeName&,
        const EntityName&,
        const CategoryName&,
        const OperationName&,
        const IncomeDate&,
        const IncomeDateEpoch&,
        const IncomeDate&,
        const IncomeDateEpoch&,
        const IncomeAmount&,
        const IncomeAmount&,
        const IncomeDay&,
        const IncomeDay&,
        const IncomeDescription&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = ez::str::toStr(
        u8R"(
SELECT
    incomes.id AS rowid,
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
    incomes.description AS description
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
                    vCallback(
                        sqlite3_column_int(stmt, 0),            //
                        ez::sqlite::readStringColumn(stmt, 1),  //
                        ez::sqlite::readStringColumn(stmt, 2),  //
                        ez::sqlite::readStringColumn(stmt, 3),  //
                        ez::sqlite::readStringColumn(stmt, 4),  //
                        ez::sqlite::readStringColumn(stmt, 5),  //
                        sqlite3_column_int64(stmt, 6),          //
                        ez::sqlite::readStringColumn(stmt, 7),  //
                        sqlite3_column_int64(stmt, 8),          //
                        sqlite3_column_double(stmt, 9),         //
                        sqlite3_column_double(stmt, 10),        //
                        sqlite3_column_int(stmt, 11),           //
                        sqlite3_column_int(stmt, 12),           //
                        ez::sqlite::readStringColumn(stmt, 13)  //
                    );
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateIncome(  //
    const RowID& vRowID,
    const IncomeName& vIncomeName,
    const EntityName& vEntityName,
    const CategoryName& vCategoryName,
    const OperationName& vOperationName,
    const IncomeDate& vStartDate,
    const IncomeDate& vEndDate,
    const IncomeAmount& vMinAmount,
    const IncomeAmount& vMaxAmount,
    const IncomeDay& vMinDays,
    const IncomeDay& vMaxDays,
    const IncomeDescription& vDescription) {}

void DataBase::DeleteIncome(const RowID& vRowID) {}

void DataBase::DeleteIncomes() {}
