#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

void DataBase::AddIncome(  //
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
    AddEntity(vEntityName);
    AddOperation(vOperationName);
    AddCategory(vCategoryName);
    auto insert_query = ez::str::toStr(
        u8R"(
INSERT OR IGNORE INTO incomes 
(
    account_id,
    name, 
    entity_id,  
    category_id, 
    operation_id,
    start_date, 
    end_date, 
    min_amount, 
    max_amount,
    min_day,
    max_day,
    description
) 
VALUES 
(
    %u,
    "%s", 
    (SELECT id FROM entities WHERE entities.name = "%s"), -- entity id
    (SELECT id FROM categories WHERE categories.name = "%s"), -- category id
    (SELECT id FROM operations WHERE operations.name = "%s"), -- operation id
    "%s", 
    "%s",
    %.2f,
    %.2f,
    %i,
    %i,
    "%s"
);
)",
        vAccountID,
        vIncomeName.c_str(),
        vEntityName.c_str(),
        vCategoryName.c_str(),
        vOperationName.c_str(),
        vStartDate.c_str(),
        vEndDate.c_str(),
        vMinAmount,
        vMaxAmount,
        vMinDays,
        vMaxDays,
        vDescription.c_str());
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a transaction in database : %s", m_LastErrorMsg);
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
                    RowID id = sqlite3_column_int(stmt, 0);
                    auto name = (const char*)sqlite3_column_text(stmt, 1);
                    auto entity = (const char*)sqlite3_column_text(stmt, 2);
                    auto category = (const char*)sqlite3_column_text(stmt, 3);
                    auto operation = (const char*)sqlite3_column_text(stmt, 4);
                    auto start_date = (const char*)sqlite3_column_text(stmt, 5);
                    auto start_epoch = sqlite3_column_int64(stmt, 6);
                    auto end_date = (const char*)sqlite3_column_text(stmt, 7);
                    auto end_epoch = sqlite3_column_int64(stmt, 8);
                    auto min_amount = sqlite3_column_double(stmt, 9);
                    auto max_amount = sqlite3_column_double(stmt, 10);
                    auto min_day = sqlite3_column_int(stmt, 11);
                    auto max_day = sqlite3_column_int(stmt, 12);
                    auto desc = (const char*)sqlite3_column_text(stmt, 13);
                    vCallback(                                    //
                        id,                                       //
                        name != nullptr ? name : "",              //
                        entity != nullptr ? entity : "",          //
                        category != nullptr ? category : "",      //
                        operation != nullptr ? operation : "",    //
                        start_date != nullptr ? start_date : "",  //
                        start_epoch,                              //
                        end_date != nullptr ? end_date : "",      //
                        end_epoch,                                //
                        min_amount,                               //
                        max_amount,                               //
                        min_day,                                  //
                        max_day,                                  //
                        desc != nullptr ? desc : "");             //
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
