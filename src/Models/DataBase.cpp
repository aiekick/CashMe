// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <Models/DataBase.h>

#include <vector>
#include <sstream>
#include <string.h>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <sqlite3.hpp>
#include <fstream>

/*
BANK ACCOUNT


*/




// will check database header magic number
// https://www.sqlite.org/fileformat.html : section 1.3
// Offset	Size	Description
// 0	    16	    The header string : "SQLite format 3\000"
bool DataBase::IsFileASqlite3DB(const DBFile& vDBFilePathName) {
    bool res = false;
    std::ifstream file_stream(vDBFilePathName, std::ios_base::binary);
    if (file_stream.is_open()) {
        char magic_header[16 + 1];
        file_stream.read(magic_header, 16U);
        if (strcmp(magic_header, "SQLite format 3\000") == 0) {
            res = true;
        }

        file_stream.close();
    }
    return res;
}

bool DataBase::CreateDBFile(const DBFile& vDBFilePathName) {
    if (!vDBFilePathName.empty()) {
        m_DataBaseFilePathName = vDBFilePathName;

        return m_CreateDB();
    }
    return false;
}

bool DataBase::OpenDBFile(const DBFile& vDBFilePathName) {
    if (!m_SqliteDB) {
        m_DataBaseFilePathName = vDBFilePathName;
        return m_OpenDB();
    } else {
        LogVarInfo("%s", "Database already opened\n");
    }
    return (m_SqliteDB != nullptr);
}

void DataBase::CloseDBFile() {
    m_CloseDB();
}

bool DataBase::BeginTransaction() {
    if (m_OpenDB()) {
        if (sqlite3_exec(m_SqliteDB, "BEGIN TRANSACTION;", nullptr, nullptr, &m_LastErrorMsg) == SQLITE_OK) {
            m_TransactionStarted = true;
            return true;
        }
    }
    LogVarError("Fail to start transaction : %s", m_LastErrorMsg);
    return false;
}

void DataBase::CommitTransaction() {
    if (sqlite3_exec(m_SqliteDB, "COMMIT;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to commit : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
}

void DataBase::RollbackTransaction() {
    if (sqlite3_exec(m_SqliteDB, "ROLLBACK;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to ROLLBACK : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
}

void DataBase::AddMarket(const Market& vMarket) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO markets (market) VALUES("%s");)", vMarket.c_str());
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a market in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddSymbol(const Symbol& vSymbol) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO symbols (symbol) VALUES("%s");)", vSymbol.c_str());
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a symbol in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddTimeFrame(const TimeFrame& vTimeFrame) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO timeframes (timeframe) VALUES("%u");)", vTimeFrame);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a timeframe in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddMarketSymbolTimeFrame(const Market& vMarket, const Symbol& vSymbol, const TimeFrame& vTimeFrame) {
    AddMarket(vMarket);
    AddSymbol(vSymbol);
    AddTimeFrame(vTimeFrame);
    auto insert_query = ct::toStr(
        u8R"(
INSERT OR IGNORE INTO market_symbol_timeframe 
    (id_market, id_symbol, id_timeframe) VALUES(
        (SELECT rowid FROM markets WHERE markets.market = "%s"),
        (SELECT rowid FROM symbols WHERE symbols.symbol = "%s"),
        (SELECT rowid FROM timeframes WHERE timeframes.timeframe = "%u"));)",
        vMarket.c_str(),
        vSymbol.c_str(),
        vTimeFrame);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a tick in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddDate(const Date& vDate) {
    auto insert_query = ct::toStr(u8R"(INSERT OR IGNORE INTO dates (date) VALUES("%f");)", vDate);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a date in database : %s", m_LastErrorMsg);
    }
}

void DataBase::AddPrice(const Market& vMarket,
                        const Symbol& vSymbol,
                        const TimeFrame& vTimeFrame,
                        const Date& vDate,
                        const OpenPrice& vOpen,
                        const HighPrice& vHigh,
                        const LowPrice& vLow,
                        const ClosePrice& vClose,
                        const VolumePrice& vVolume) {
    AddMarketSymbolTimeFrame(vMarket, vSymbol, vTimeFrame);
    AddDate(vDate);
    auto insert_query = ct::toStr(
        u8R"(
INSERT OR IGNORE INTO prices 
    (id_market_symbol_timeframe, id_date, open, high, low, close, volume) VALUES(
        (SELECT rowid FROM market_symbol_timeframe WHERE 
            market_symbol_timeframe.id_market = (SELECT rowid FROM markets WHERE markets.market = "%s") AND 
            market_symbol_timeframe.id_symbol = (SELECT rowid FROM symbols WHERE symbols.symbol = "%s") AND 
            market_symbol_timeframe.id_timeframe = (SELECT rowid FROM timeframes WHERE timeframes.timeframe = "%u")),
        (SELECT rowid FROM dates WHERE dates.date = "%f"),
        %f,%f,%f,%f,%f);)",
        vMarket.c_str(),
        vSymbol.c_str(),
        vTimeFrame,
        vDate,
        vOpen,
        vHigh,
        vLow,
        vClose,
        vVolume);
    if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to insert a tick in database : %s", m_LastErrorMsg);
    }
}

void DataBase::GetSymbols(std::function<void(const Market&, const Symbol&, const TimeFrame&, const BarsCount&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);

    std::string select_query =
        u8R"(
SELECT
  markets.market AS market,
  symbols.symbol AS symbol,
  timeframes.timeframe AS timeframe,
  COUNT(prices.id_market_symbol_timeframe) AS count
FROM 
  market_symbol_timeframe
  LEFT JOIN markets ON market_symbol_timeframe.id_market = markets.rowid
  LEFT JOIN symbols ON market_symbol_timeframe.id_symbol = symbols.rowid
  LEFT JOIN timeframes ON market_symbol_timeframe.id_timeframe = timeframes.rowid
  LEFT JOIN prices ON market_symbol_timeframe.rowid = prices.id_market_symbol_timeframe
GROUP BY market, symbol, timeframe
ORDER BY market, symbol, timeframe;
)";
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get symbols with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    const char* market = (const char*)sqlite3_column_text(stmt, 0);
                    const char* symbol = (const char*)sqlite3_column_text(stmt, 1);
                    int timeframe = sqlite3_column_int(stmt, 2);
                    int count = sqlite3_column_int(stmt, 3);

                    // call callback with datas passed in args
                    vCallback(market != nullptr ? market : "", symbol != nullptr ? symbol : "", timeframe, count);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::GetPrices(const Market& vMarket,
                         const Symbol& vSymbol,
                         const TimeFrame& vTimeFrame,
                         std::function<void(const Date&, const OpenPrice&, const HighPrice&, const LowPrice&, const ClosePrice&, const VolumePrice&)> vCallback) {
    // no interest to call that without a claaback for retrieve datas
    assert(vCallback);

    std::string select_query = ct::toStr(
        u8R"(
SELECT
  dates.date as date,
  prices.open as open,
  prices.high as high,
  prices.low as low,
  prices.close as close,
  prices.volume as volume
FROM 
  prices
  JOIN market_symbol_timeframe ON prices.id_market_symbol_timeframe = market_symbol_timeframe.rowid
  JOIN timeframes ON market_symbol_timeframe.id_timeframe = timeframes.rowid
  LEFT JOIN dates ON prices.id_date = dates.rowid
WHERE
  market_symbol_timeframe.id_market = (SELECT rowid FROM markets WHERE market = "%s")
  AND market_symbol_timeframe.id_symbol = (SELECT rowid FROM symbols WHERE symbol = "%s")
  AND timeframes.timeframe = "%u";
)",
        vMarket.c_str(),
        vSymbol.c_str(),
        vTimeFrame);
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = sqlite3_prepare_v2(m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get price with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                // on r�cup�re une ligne dans la table
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    auto date = sqlite3_column_double(stmt, 0);
                    auto open = sqlite3_column_double(stmt, 1);
                    auto high = sqlite3_column_double(stmt, 2);
                    auto low = sqlite3_column_double(stmt, 3);
                    auto close = sqlite3_column_double(stmt, 4);
                    auto volume = sqlite3_column_double(stmt, 5);

                    // call callback with datas passed in args
                    vCallback(date, open, high, low, close, volume);
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
}

void DataBase::m_EnableForeignKey() {
    // todo : "PRAGMA foreign_keys = ON;"
}

std::string DataBase::GetLastErrorMesg() {
    return std::string(m_LastErrorMsg);
}

bool DataBase::SetSettingsXMLDatas(const std::string& vXMLDatas) {
    if (!vXMLDatas.empty()) {
        std::string insert_query;
        // insert or replace at line 0
        auto xml_datas = GetSettingsXMLDatas();
        if (xml_datas.empty()) {
            insert_query = "INSERT INTO settings(xml) VALUES(\"" + vXMLDatas + "\");";
        } else {
            insert_query = "UPDATE settings SET xml = \"" + vXMLDatas + "\" WHERE rowid = 1;";
        }
        if (sqlite3_exec(m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
#ifdef _DEBUG
            FileHelper::Instance()->SaveStringToFile(insert_query, "insert_query.txt");
            FileHelper::Instance()->SaveStringToFile(m_LastErrorMsg, "last_error_msg.txt");
#endif
            LogVarError("Fail to insert or replace xml in table settings of database : %s", m_LastErrorMsg);
            return false;
        }
        return true;
    }
    return false;
}

std::string DataBase::GetSettingsXMLDatas() {
    std::string res;
    // SELECT at line 0
    auto select_query = u8R"(SELECT * FROM settings WHERE rowid = 1;)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_SqliteDB, select_query, (int)strlen(select_query), &stmt, nullptr) != SQLITE_OK) {
        LogVarError("%s", "Fail to get xml FROM settings table of database");
    } else {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            auto len = sqlite3_column_bytes(stmt, 0);
            auto txt = (const char*)sqlite3_column_text(stmt, 0);
            if (txt && len) {
                res = std::string(txt, len);
            }
        }
    }
    sqlite3_finalize(stmt);
    return res;
}

void DataBase::ClearDataTables() {
    auto clear_query =
        u8R"(
BEGIN TRANSACTION;
DELETE FROM users;
DELETE FROM accounts;
DELETE FROM trajectories;
DELETE FROM transactions;
COMMIT;
)";
    if (sqlite3_exec(m_SqliteDB, clear_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to clear datas tables in database : %s", m_LastErrorMsg);
    }
}

////////////////////////////////////////////////////////////
///// PRIVATE //////////////////////////////////////////////
////////////////////////////////////////////////////////////

bool DataBase::m_OpenDB() {
    if (!m_SqliteDB) {
        if (sqlite3_open_v2(m_DataBaseFilePathName.c_str(), &m_SqliteDB, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {  // db possibily not exist
            m_CreateDBTables(false);
        }
    }
    return (m_SqliteDB != nullptr);
}

bool DataBase::m_CreateDB() {
    m_CloseDB();
    if (!m_SqliteDB) {
        FileHelper::Instance()->DestroyFile(m_DataBaseFilePathName);
        if (sqlite3_open_v2(m_DataBaseFilePathName.c_str(), &m_SqliteDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) == SQLITE_OK) {  // db possibily not exist
            m_CreateDBTables();
            m_CloseDB();
        }
    }
    return (m_SqliteDB != nullptr);
}

void DataBase::m_CreateDBTables(const bool& vPrintLogs) {
    if (m_SqliteDB) {  // in the doubt
        const char* create_tables =
            u8R"(
CREATE TABLE users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
);

CREATE TABLE accounts (
    account_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    account_type TEXT NOT NULL,
    account_name TEXT NOT NULL,
    bank_name TEXT NOT NULL,
    account_number TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

CREATE TABLE categories (
    category_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
);

CREATE TABLE transactions (
    transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id INTEGER NOT NULL,
    amount REAL NOT NULL,
    transaction_type TEXT NOT NULL,
    transaction_date DATE NOT NULL,
    description TEXT,
    category_id INTEGER,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id),
    FOREIGN KEY (category_id) REFERENCES categories(category_id)
);

CREATE TABLE settings (
	xml TEXT
);
)";
        // signal_tags.tag_color is like this format 128;250,100;255
        if (sqlite3_exec(m_SqliteDB, create_tables, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
            if (m_LastErrorMsg) {
                LogVarError("Fail to create database : %s", m_LastErrorMsg);
            } else {
                LogVarError("%s", "Fail to create database");
            }
            m_SqliteDB = nullptr;
        }
    }
}

void DataBase::m_CloseDB() {
    if (m_SqliteDB) {
        if (sqlite3_close(m_SqliteDB) == SQLITE_BUSY) {
            // try to force closing
            sqlite3_close_v2(m_SqliteDB);
        }
    }
    m_SqliteDB = nullptr;
    // there is also sqlite3LeaveMutexAndCloseZombie when sqlite is stucked
}
