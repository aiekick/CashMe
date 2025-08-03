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
    const AccountBaseSolde& vBaseSolde) {
    AddBank(vBankName);

    auto insert_query =  //
        ez::sqlite::QueryBuilder()
            .setTable("accounts")
            .addFieldQuery("bank_id", R"(SELECT id FROM banks WHERE banks.name = "%s")", vBankName.c_str())
            .addField("bank_agency", vBankAgency)
            .addField("type", vAccountType)
            .addField("name", vAccountName)
            .addField("number", vAccountNumber)
            .addField("base_solde", "%.2f", vBaseSolde)
            .build(ez::sqlite::QueryType::INSERT_IF_NOT_EXIST);
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a Bank Account in database : %s (%s)", m_LastErrorMsg, insert_query.c_str());
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
        const AccountBaseSolde&,
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
                    vCallback(
                        sqlite3_column_int(stmt, 0),            // RowID
                        ez::sqlite::readStringColumn(stmt, 1),  // BankName
                        ez::sqlite::readStringColumn(stmt, 2),  // BankAgency
                        ez::sqlite::readStringColumn(stmt, 3),  // AccountType
                        ez::sqlite::readStringColumn(stmt, 4),  // AccountName
                        ez::sqlite::readStringColumn(stmt, 5),  // AccountNumber
                        sqlite3_column_double(stmt, 6),         // AccountBaseSolde
                        sqlite3_column_int(stmt, 7)             // TransactionsCount
                    );
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::GetAccountsStats(  //
    std::function<void(  //
        const RowID&,
        const BankName&,
        const BankAgency&,
        const AccountNumber&,
        const AccountType&,
        const AccountName&,
        const AccountBaseSolde&,
        const TransactionDebit&,
        const TransactionCredit&,
        const TransactionsCount&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const std::string& select_query = R"(
SELECT
  accounts.id,
  banks.name,
  accounts.bank_agency,
  accounts.number,
  accounts.type,
  accounts.name,
  accounts.base_solde,
  ROUND(SUM(CASE WHEN transactions.amount < 0 THEN transactions.amount ELSE 0 END), 2) AS debit,
  ROUND(SUM(CASE WHEN transactions.amount > 0 THEN transactions.amount ELSE 0 END), 2) AS credit,
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
                    vCallback(
                        sqlite3_column_int(stmt, 0),                       // RowID
                        ez::sqlite::readStringColumn(stmt, 1),             // BankName
                        ez::sqlite::readStringColumn(stmt, 2),             // BankAgency
                        ez::sqlite::readStringColumn(stmt, 3),             // AccountNumber
                        ez::sqlite::readStringColumn(stmt, 4),             // AccountType
                        ez::sqlite::readStringColumn(stmt, 5),             // AccountName
                        (AccountBaseSolde)sqlite3_column_double(stmt, 6),  // AccountBaseSolde
                        (TransactionDebit)sqlite3_column_double(stmt, 7),  // TransactionDebit
                        (TransactionCredit)sqlite3_column_double(stmt, 8),  // TransactionCredit
                        (TransactionsCount)sqlite3_column_int(stmt, 9)      // TransactionCredit
                    );
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
    const AccountBaseSolde& vBaseSolde) {
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
