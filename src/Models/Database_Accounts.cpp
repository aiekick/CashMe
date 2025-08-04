#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezSha.hpp>

bool DataBase::AddAccount(const BankName& vBankName, const AccountInput& vAccountInput) {
    bool ret = true;
    BankInput bi;
    bi.name = vBankName;
    AddBank(bi);
    auto insert_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("accounts")
            .addFieldQuery("bank_id", R"(SELECT id FROM banks WHERE banks.name = "%s")", vBankName.c_str())
            .addField("bank_agency", vAccountInput.bank_agency)
            .addField("type", vAccountInput.type)
            .addField("name", vAccountInput.name)
            .addField("number", vAccountInput.number)
            .addField("base_solde", "%.2f", vAccountInput.base_solde)
            .addField(
                "sha",
                ez::sha1()
                    .add(vBankName)
                    .add(vAccountInput.bank_agency)
                    .add(vAccountInput.type)
                    .add(vAccountInput.name)
                    .add(vAccountInput.number)
                    .addValue(vAccountInput.base_solde)
                    .finalize()
                    .getHex())
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a Bank Account in database : %s (%s)", m_LastErrorMsg, insert_query.c_str());
        ret = false;
    }
    return ret;
}

bool DataBase::GetAccount(const AccountNumber& vAccountNumber, RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ez::str::toStr(
        u8R"(
SELECT 
  id 
FROM 
  accounts 
WHERE 
  accounts.number = "%s"
;)",
        vAccountNumber.c_str());
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get account id with reason", sqlite3_errmsg(m_SqliteDB));
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

bool DataBase::GetAccounts(std::function<void(const AccountOutput&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    std::string select_query = u8R"(
SELECT
  accounts.id AS rowid,
  banks.name AS bank_name,
  accounts.number AS account_number,
  accounts.bank_agency AS bank_agency,
  accounts.type AS account_type,
  accounts.name AS account_name,
  accounts.base_solde AS account_base_solde,
  accounts.sha AS sha
FROM accounts
LEFT JOIN banks ON banks.id = accounts.bank_id
GROUP BY bank_name, bank_agency, account_name
ORDER BY accounts.id;
)";
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get accounts with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    AccountOutput ao;
                    ao.id = sqlite3_column_int(stmt, 0);
                    ao.bankName = ez::sqlite::readStringColumn(stmt, 1);
                    ao.datas.number = ez::sqlite::readStringColumn(stmt, 2);
                    ao.datas.bank_agency = ez::sqlite::readStringColumn(stmt, 3);
                    ao.datas.type = ez::sqlite::readStringColumn(stmt, 4);
                    ao.datas.name = ez::sqlite::readStringColumn(stmt, 5);
                    ao.datas.base_solde = sqlite3_column_double(stmt, 6);
                    ao.datas.sha = ez::sqlite::readStringColumn(stmt, 7);
                    vCallback(ao);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::GetAccountsStats(std::function<void(const AccountOutput&)> vCallback) {
    bool ret = false;
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const std::string& select_query = R"(
SELECT
  accounts.id,
  banks.name,
  accounts.number,
  accounts.bank_agency,
  accounts.type,
  accounts.name,
  accounts.base_solde,
  accounts.sha,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN transactions.amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN transactions.amount ELSE 0 END), 2) AS credit,
  (SUM(transactions.amount) + accounts.base_solde) AS amount,
  COUNT(transactions.id)
FROM
  accounts
  LEFT JOIN transactions ON transactions.account_id = accounts.id
  LEFT JOIN banks ON accounts.bank_id = banks.id
GROUP BY
  accounts.name
ORDER BY
  accounts.name;
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
                    AccountOutput ao;
                    ao.id = sqlite3_column_int(stmt, 0);
                    ao.bankName = ez::sqlite::readStringColumn(stmt, 1);
                    ao.datas.number = ez::sqlite::readStringColumn(stmt, 2);
                    ao.datas.bank_agency = ez::sqlite::readStringColumn(stmt, 3);
                    ao.datas.type = ez::sqlite::readStringColumn(stmt, 4);
                    ao.datas.name = ez::sqlite::readStringColumn(stmt, 5);
                    ao.datas.base_solde = sqlite3_column_double(stmt, 6);
                    ao.datas.sha = ez::sqlite::readStringColumn(stmt, 7);
                    ao.amounts.debit = sqlite3_column_double(stmt, 8);
                    ao.amounts.credit = sqlite3_column_double(stmt, 9);
                    ao.amounts.amount = sqlite3_column_double(stmt, 10);
                    ao.count = sqlite3_column_int(stmt, 11);
                    vCallback(ao);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}

bool DataBase::UpdateAccount(const RowID& vRowID, const BankName& vBankName, const AccountInput& vAccountInput) {
    bool ret = true;
    auto insert_query = ez::str::toStr(
        u8R"(
UPDATE 
  accounts
SET 
  bank_id = (SELECT id FROM banks WHERE name = "%s"),
  number = "%s",
  bank_agency = "%s",
  type = "%s",
  name = "%s",
  base_solde = %f,
  sha = "%s",
WHERE
  accounts.id = %u;
)",
        vBankName.c_str(),
        vAccountInput.number.c_str(),
        vAccountInput.bank_agency.c_str(),
        vAccountInput.type.c_str(),
        vAccountInput.name.c_str(),
        vAccountInput.base_solde,
        vAccountInput.sha.c_str(),
        vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a account in database : %s", m_LastErrorMsg);
        ret = false;
    }
    return ret;
}

bool DataBase::DeleteAccount(const RowID& vRowID) {
    bool ret = false;
    auto insert_query = ez::str::toStr(
        u8R"(
DELETE FROM 
  accounts
WHERE
  accounts.id = %u;
)",
        vRowID);
    if (m_OpenDB()) {
        ret = true;
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete a account in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}

bool DataBase::DeleteAccounts() {
    bool ret = false;
    if (m_OpenDB()) {
        ret = true;
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM accounts;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of accounts table in database : %s", m_LastErrorMsg);
            ret = false;
        }
        m_CloseDB();
    }
    return ret;
}
