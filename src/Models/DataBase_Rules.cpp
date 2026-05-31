#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezStr.hpp>
#include <cstring>

bool DataBase::AddRule(const CategorizationRule& vRule) {
    bool ret = true;
    auto insert_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("rules")
            .addOrSetField("enabled", static_cast<int32_t>(vRule.enabled))
            .addOrSetField("name", vRule.name)
            .addOrSetField("description", vRule.description)
            .addOrSetField("description_pattern", vRule.descriptionPattern)
            .addOrSetField("comment_pattern", vRule.commentPattern)
            .addOrSetField("entity_pattern", vRule.entityPattern)
            .addOrSetField("use_range", static_cast<int32_t>(vRule.useAmountRange))
            .addOrSetField("amount_min", vRule.amountMin)
            .addOrSetField("amount_max", vRule.amountMax)
            .addOrSetField("target_category", vRule.targetCategory)
            .addOrSetField("target_operation", vRule.targetOperation)
            .build(ez::sqlite::QueryType::INSERT);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a rule in database : %s (%s)", m_LastErrorMsg, insert_query);
        ret = false;
    }
    return ret;
}

bool DataBase::GetRules(std::function<void(const CategorizationRule&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    auto select_query = u8R"(
SELECT
    id, enabled, name, description,
    description_pattern, comment_pattern, entity_pattern,
    use_range, amount_min, amount_max,
    target_category, target_operation
FROM
    rules
ORDER BY
    id;
)";
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query, (int)strlen(select_query), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail to get rules with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    CategorizationRule rule;
                    rule.id = sqlite3_column_int(stmt, 0);
                    rule.enabled = (sqlite3_column_int(stmt, 1) != 0);
                    rule.name = ez::sqlite::readStringColumn(stmt, 2);
                    rule.description = ez::sqlite::readStringColumn(stmt, 3);
                    rule.descriptionPattern = ez::sqlite::readStringColumn(stmt, 4);
                    rule.commentPattern = ez::sqlite::readStringColumn(stmt, 5);
                    rule.entityPattern = ez::sqlite::readStringColumn(stmt, 6);
                    rule.useAmountRange = (sqlite3_column_int(stmt, 7) != 0);
                    rule.amountMin = sqlite3_column_double(stmt, 8);
                    rule.amountMax = sqlite3_column_double(stmt, 9);
                    rule.targetCategory = ez::sqlite::readStringColumn(stmt, 10);
                    rule.targetOperation = ez::sqlite::readStringColumn(stmt, 11);
                    vCallback(rule);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateRule(const RowID& vRowID, const CategorizationRule& vRule) {
    bool ret = true;
    auto update_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("rules")
            .addOrSetField("enabled", static_cast<int32_t>(vRule.enabled))
            .addOrSetField("name", vRule.name)
            .addOrSetField("description", vRule.description)
            .addOrSetField("description_pattern", vRule.descriptionPattern)
            .addOrSetField("comment_pattern", vRule.commentPattern)
            .addOrSetField("entity_pattern", vRule.entityPattern)
            .addOrSetField("use_range", static_cast<int32_t>(vRule.useAmountRange))
            .addOrSetField("amount_min", vRule.amountMin)
            .addOrSetField("amount_max", vRule.amountMax)
            .addOrSetField("target_category", vRule.targetCategory)
            .addOrSetField("target_operation", vRule.targetOperation)
            .addWhere(R"(rules.id = %u)", vRowID)
            .build(ez::sqlite::QueryType::UPDATE);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, update_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a rule in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}

bool DataBase::DeleteRule(const RowID& vRowID) {
    bool ret = true;
    auto delete_query = ez::str::toStr(u8R"(DELETE FROM rules WHERE id = %u;)", vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, delete_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to delete a rule in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}
