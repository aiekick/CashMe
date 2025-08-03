#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

void DataBase::AddBank(const BankName& vBankName, const BankUrl& vUrl) {
    auto insert_query =             //
        ez::sqlite::QueryBuilder()  //
            .setTable("banks")
            .addField("name", vBankName)
            .addField("url", vUrl)
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a bank in database : %s (%s)", m_LastErrorMsg, insert_query.c_str());
    }
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

void DataBase::GetBanks(std::function<void(const BankName&, const BankUrl&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto& select_query = ez::str::toStr(u8R"(SELECT name, url FROM banks GROUP BY name;)");
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get banks with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    vCallback(
                        ez::sqlite::readStringColumn(stmt, 0),  // BankName
                        ez::sqlite::readStringColumn(stmt, 1)   // BankUrl
                    );
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::GetBanksStats(  //
    std::function<void(        //
        const RowID&,
        const BankName&,
        const BankUrl&,
        const TransactionDebit&,
        const TransactionCredit&,
        const TransactionsCount&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const std::string& select_query = R"(
SELECT
  banks.id,
  banks.name,
  banks.url,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN transactions.amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN transactions.amount ELSE 0 END), 2) AS credit,
  COUNT(transactions.id)
FROM
  banks
  LEFT JOIN accounts ON accounts.bank_id = banks.id
  LEFT JOIN transactions ON transactions.account_id = accounts.id
GROUP BY
  banks.name
ORDER BY
  banks.name;
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
                    vCallback(
                        sqlite3_column_int(stmt, 0),                        // RowID
                        ez::sqlite::readStringColumn(stmt, 1),              // BankName
                        ez::sqlite::readStringColumn(stmt, 2),              // BankUrl
                        (TransactionDebit)sqlite3_column_double(stmt, 3),   // TransactionDebit
                        (TransactionCredit)sqlite3_column_double(stmt, 4),  // TransactionCredit
                        (TransactionsCount)sqlite3_column_int(stmt, 5)      // TransactionCredit
                    );
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateBank(const RowID& vRowID, const BankName& vBankName, const BankUrl& vUrl) {
    auto insert_query = ez::str::toStr(u8R"(UPDATE banks SET name = "%s", url = "%s" WHERE id = %u;)", vBankName.c_str(), vUrl.c_str(), vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a bank in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteBanks() {
    if (m_OpenDB()) {
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM banks;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of banks table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}
