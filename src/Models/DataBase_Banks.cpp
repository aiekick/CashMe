#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezSha.hpp>

bool DataBase::AddBank(const BankInput& vBankInput) {
    bool ret = true;
    auto insert_query =             //
        ez::sqlite::QueryBuilder()  //
            .setTable("banks")
            .addField("name", vBankInput.name)
            .addField("url", vBankInput.url)
            .addField("sha", ez::sha1().add(vBankInput.name).add(vBankInput.url).finalize().getHex())
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a bank in database : %s (%s)", m_LastErrorMsg, insert_query.c_str());
        ret = false;
    }
    return ret;
}

bool DataBase::GetBank(const BankName& vBankName, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ez::str::toStr(u8R"(SELECT id FROM banks WHERE name = "%s";)", vBankName.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get bank id with reason", sqlite3_errmsg(m_SqliteDB));
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

bool DataBase::GetBanks(std::function<void(const BankOutput&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ez::str::toStr(u8R"(SELECT id, name, url FROM banks GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get banks with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    BankOutput bo;
                    bo.id = sqlite3_column_int(stmt, 0);
                    bo.datas.name = ez::sqlite::readStringColumn(stmt, 1);  // BankName
                    bo.datas.url = ez::sqlite::readStringColumn(stmt, 2);   // BankUrl
                    vCallback(bo);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetBanksStats(std::function<void(const BankOutput&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const std::string& select_query = R"(
SELECT
    banks.id,
    banks.name,
    banks.url,
    banks.sha,
    ROUND(SUM(CASE WHEN transactions.amount < 0 THEN transactions.amount ELSE 0 END), 2) AS debit,
    ROUND(SUM(CASE WHEN transactions.amount > 0 THEN transactions.amount ELSE 0 END), 2) AS credit,
    ROUND(IFNULL(SUM(DISTINCT accounts.base_solde), 0) + IFNULL(SUM(transactions.amount), 0), 2) AS total_amount,
    COUNT(transactions.id) AS transactions_count
FROM banks
LEFT JOIN accounts ON accounts.bank_id = banks.id
LEFT JOIN transactions ON transactions.account_id = accounts.id
GROUP BY banks.id, banks.name, banks.url, banks.sha
ORDER BY banks.name;
)";
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get categories with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    BankOutput bo;
                    bo.id = sqlite3_column_int(stmt, 0);
                    bo.datas.name = ez::sqlite::readStringColumn(stmt, 1); 
                    bo.datas.url = ez::sqlite::readStringColumn(stmt, 2);
                    bo.datas.sha = ez::sqlite::readStringColumn(stmt, 3); 
                    bo.amounts.debit = sqlite3_column_double(stmt, 4);
                    bo.amounts.credit = sqlite3_column_double(stmt, 5);
                    bo.amounts.amount = sqlite3_column_double(stmt, 6);
                    bo.count = sqlite3_column_int(stmt, 7);
                    vCallback(bo);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateBank(const RowID& vRowID, const BankInput& vBankInput) {
    bool ret = true;
    auto insert_query = ez::str::toStr(u8R"(UPDATE banks SET name = "%s", url = "%s" WHERE id = %u;)", vBankInput.name.c_str(), vBankInput.url.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a bank in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}

bool DataBase::DeleteBanks() {
    bool ret = false;
    if (m_OpenDB()) {
        ret = true;
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM banks;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of banks table in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}
