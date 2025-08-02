#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezSha.hpp>


void DataBase::AddAccount(
    const BankName& vBankName,
    const BankAgency& vBankAgency,
    const AccountType& vAccountType,
    const AccountName& vAccountName,
    const AccountNumber& vAccountNumber,
    const AccounBaseSolde& vBaseSolde) {
    AddBank(vBankName);
    auto insert_query = ez::str::toStr(
        u8R"(
INSERT OR IGNORE INTO accounts 
    (bank_id, bank_agency, type, name, number, base_solde) VALUES(
        (SELECT id FROM banks WHERE banks.name = "%s"), -- bank id
        "%s", -- bank agency
        "%s", -- account type
        "%s", -- account name
        "%s", -- account number
        %f    -- account base solde
        );)",
        vBankName.c_str(),
        vBankAgency.c_str(),
        vAccountType.c_str(),
        vAccountName.c_str(),
        vAccountNumber.c_str(),
        vBaseSolde);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a Bank Account in database : %s", m_LastErrorMsg);
    }
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

bool DataBase::GetAccount(
    const BankName& vBankName,
    const BankAgency& vBankAgency,
    const AccountType& vAccountType,
    const AccountName& vAccountName,
    const AccountNumber& vAccountNumber,
    RowID& vOutRowID) {
    bool ret = false;
    auto select_query = ez::str::toStr(
        u8R"(
SELECT 
  id 
FROM 
  accounts 
  LEFT JOIN banks ON accounts.bank_id = banks.id
WHERE 
  AND banks.name = "%s"
  AND banks.agency = "%s"
  AND accounts.type = "%s"
  AND accounts.name = "%s"
  AND accounts.number = "%s"
;)",
        vBankName.c_str(),     //
        vBankAgency.c_str(),   //
        vAccountType.c_str(),  //
        vAccountName.c_str(),  //
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

void DataBase::GetAccounts(  //
    std::function<void(      //
        const RowID&,
        const BankName&,
        const BankAgency&,
        const AccountType&,
        const AccountName&,
        const AccountNumber&,
        const AccounBaseSolde&,
        const TransactionsCount&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    std::string select_query = u8R"(
SELECT
  accounts.id AS rowid,
  banks.name AS bank_name,
  accounts.bank_agency AS bank_agency,
  accounts.type AS account_type,
  accounts.name AS account_name,
  accounts.number AS account_number,
  accounts.base_solde AS account_base_solde,
  COUNT(t.id) AS nombre_transactions
FROM accounts
LEFT JOIN banks ON banks.id = accounts.bank_id
LEFT JOIN transactions t ON accounts.id = t.account_id
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
                    RowID account_id = sqlite3_column_int(stmt, 0);
                    const char* bank_name = (const char*)sqlite3_column_text(stmt, 1);
                    const char* bank_agency = (const char*)sqlite3_column_text(stmt, 2);
                    const char* account_type = (const char*)sqlite3_column_text(stmt, 3);
                    const char* account_name = (const char*)sqlite3_column_text(stmt, 4);
                    const char* account_number = (const char*)sqlite3_column_text(stmt, 5);
                    AccounBaseSolde account_base_solde = sqlite3_column_double(stmt, 6);
                    TransactionsCount transactions_count = sqlite3_column_int(stmt, 7);
                    vCallback(                                            //
                        account_id,                                       //
                        bank_name != nullptr ? bank_name : "",            //
                        bank_agency != nullptr ? bank_agency : "",        //
                        account_type != nullptr ? account_type : "",      //
                        account_name != nullptr ? account_name : "",      //
                        account_number != nullptr ? account_number : "",  //
                        account_base_solde,                               //
                        transactions_count);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::UpdateAccount(
    const RowID& vRowID,
    const BankName& vBankName,
    const BankAgency& vBankAgency,
    const AccountType& vAccountType,
    const AccountName& vAccountName,
    const AccountNumber& vAccountNumber,
    const AccounBaseSolde& vBaseSolde) {
    auto insert_query = ez::str::toStr(
        u8R"(
UPDATE 
  accounts
SET 
  bank_id = (SELECT id FROM banks WHERE name = "%s"),
  type = "%s",
  name = "%s",
  number = "%s",
  base_solde = %f
WHERE
  accounts.id = %u;
)",
        vBankName.c_str(),
        vAccountType.c_str(),
        vAccountName.c_str(),
        vAccountNumber.c_str(),
        vBaseSolde,
        vRowID);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to update a account in database : %s", m_LastErrorMsg);
    }
}

void DataBase::DeleteAccount(const RowID& vRowID) {
    auto insert_query = ez::str::toStr(
        u8R"(
DELETE FROM 
  accounts
WHERE
  accounts.id = %u;
)",
        vRowID);
    if (m_OpenDB()) {
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete a account in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}

void DataBase::DeleteAccounts() {
    if (m_OpenDB()) {
        auto insert_query = ez::str::toStr(u8R"(DELETE FROM accounts;)");
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            LogVarError("Fail to delete content of accounts table in database : %s", m_LastErrorMsg);
        }
        m_CloseDB();
    }
}
