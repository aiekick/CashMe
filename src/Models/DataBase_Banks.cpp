#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

void DataBase::AddBank(const BankName& vBankName, const std::string& vUrl) {
    auto insert_query = ez::str::toStr(u8R"(INSERT OR IGNORE INTO banks (name, url) VALUES("%s", "%s");)", vBankName.c_str(), vUrl.c_str());
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a bank in database : %s", m_LastErrorMsg);
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

void DataBase::GetBanks(std::function<void(const BankName&, const std::string&)> vCallback) {
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
                    const char* bank_name = (const char*)sqlite3_column_text(stmt, 0);
                    const char* bank_url = (const char*)sqlite3_column_text(stmt, 1);
                    vCallback(                                  //
                        bank_name != nullptr ? bank_name : "",  //
                        bank_url != nullptr ? bank_url : "");
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateBank(const RowID& vRowID, const BankName& vBankName, const std::string& vUrl) {
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
