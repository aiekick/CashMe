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
#include <cstring>
#include <fstream>

#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezFile.hpp>
#include <ezlibs/ezSqlite.hpp>

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

bool DataBase::OpenDBFile() {
    return OpenDBFile(m_DataBaseFilePathName);
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
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "BEGIN TRANSACTION;", nullptr, nullptr, &m_LastErrorMsg) == SQLITE_OK) {
            m_TransactionStarted = true;
            return true;
        }
    }
    LogVarError("Fail to start transaction : %s", m_LastErrorMsg);
    return false;
}

void DataBase::CommitTransaction() {
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "COMMIT;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to commit : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
    m_CloseDB();
}

void DataBase::RollbackTransaction() {
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "ROLLBACK;", nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
        LogVarError("Fail to ROLLBACK : %s", m_LastErrorMsg);
    }
    // we will close the db so force it to reset
    m_TransactionStarted = false;
}

bool DataBase::m_EnableForeignKey() {
    if (!m_SqliteDB) {
        int res = m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("Erreur lors de l'activation des clés étrangères : %s", sqlite3_errmsg(m_SqliteDB));
        }
    }
    return (m_SqliteDB != nullptr);
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
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, insert_query.c_str(), nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
#ifdef _DEBUG
            ez::file::saveStringToFile(insert_query, "insert_query.txt");
            ez::file::saveStringToFile(m_LastErrorMsg, "last_error_msg.txt");
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
    if (m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query, (int)strlen(select_query), &stmt, nullptr) != SQLITE_OK) {
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
DELETE FROM banks;
DELETE FROM incomes;
DELETE FROM sources;
DELETE FROM accounts;
DELETE FROM entities;
DELETE FROM categories;
DELETE FROM operations;
DELETE FROM transactions;
COMMIT;
)";
    if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, clear_query, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
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
        } else {
            m_EnableForeignKey();
        }
    }
    return (m_SqliteDB != nullptr);
}

bool DataBase::m_CreateDB() {
    m_CloseDB();
    if (!m_SqliteDB) {
        ez::file::destroyFile(m_DataBaseFilePathName);
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
CREATE TABLE banks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    address TEXT,
    url TEXT
);

CREATE TABLE accounts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    number TEXT NOT NULL UNIQUE,
    bank_id INTEGER NOT NULL,
    bank_agency TEXT,
    type TEXT NOT NULL,
    name TEXT NOT NULL,
    base_solde REAL,
    FOREIGN KEY (bank_id) REFERENCES banks(id)
);

CREATE TABLE sources (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    type TEXT NOT NULL,
    sha TEXT NOT NULL
);

CREATE TABLE entities (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

-- type of bank transaction, like CB, SEPA, etc... 
CREATE TABLE operations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE incomes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    account_id INTEGER NOT NULL,
    entity_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    operation_id INTEGER NOT NULL,
    start_date DATE NOT NULL, -- start date
    end_date DATE, -- end date (empty if no end)
    min_amount REAL NOT NULL, -- min amount
    max_amount REAL NOT NULL, -- max amount, can be the same as min
    min_day INTEGER, -- min day, can be zero
    max_day INTEGER, -- max day, can be the same as min
    description TEXT,
    -- links
    FOREIGN KEY (account_id) REFERENCES accounts(id),
    FOREIGN KEY (entity_id) REFERENCES entities(id),
    FOREIGN KEY (category_id) REFERENCES categories(id),
    FOREIGN KEY (operation_id) REFERENCES operations(id),
    -- verifs
    CHECK (max_amount >= min_amount),
    CHECK (max_day >= min_day)
);

CREATE TABLE transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id INTEGER NOT NULL,
    entity_id INTEGER NOT NULL,
    category_id INTEGER NOT NULL,
    operation_id INTEGER NOT NULL,
    source_id INTEGER NOT NULL,
    date DATE NOT NULL,
    description TEXT,
    comment TEXT,
    amount REAL NOT NULL,
    confirmed INTEGER,
    hash TEXT NOT NULL UNIQUE,
    FOREIGN KEY (account_id) REFERENCES accounts(id),
    FOREIGN KEY (operation_id) REFERENCES operations(id),
    FOREIGN KEY (category_id) REFERENCES categories(id),
    FOREIGN KEY (entity_id) REFERENCES entities(id),
    FOREIGN KEY (source_id) REFERENCES sources(id)
);

CREATE TABLE settings (
	xml TEXT
);
)";
        if (m_debug_sqlite3_exec(__FUNCTION__, m_SqliteDB, create_tables, nullptr, nullptr, &m_LastErrorMsg) != SQLITE_OK) {
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
    if (!m_TransactionStarted) {
        if (m_SqliteDB) {
            if (sqlite3_close(m_SqliteDB) == SQLITE_BUSY) {
                // try to force closing
                sqlite3_close_v2(m_SqliteDB);
            }
        }
        m_SqliteDB = nullptr;
        // there is also sqlite3LeaveMutexAndCloseZombie when sqlite is stucked
    }
}

int32_t DataBase::m_debug_sqlite3_exec(  //
    const char* vDebugLabel,
    sqlite3* db,                                 /* An open database */
    const char* sql_query,                       /* SQL to be evaluated */
    int (*callback)(void*, int, char**, char**), /* Callback function */
    void* arg1,                                  /* 1st argument to callback */
    char** errmsg) {                             /* Error msg written here */
#ifdef _DEBUG
    std::string func_name = vDebugLabel;
    ez::str::replaceString(func_name, "DataBase::", "");
    ez::file::saveStringToFile(                     //
        sql_query,                                  //
        ez::file::correctSlashTypeForFilePathName(  //
            ez::str::toStr("sqlite3/%s.sql", func_name.c_str())));
#endif
    return sqlite3_exec(db, sql_query, callback, arg1, errmsg);
}

int32_t DataBase::m_debug_sqlite3_prepare_v2(  //
    const char* vDebugLabel,
    sqlite3* db,           /* Database handle. */
    const char* sql_query, /* UTF-8 encoded SQL statement. */
    int nBytes,            /* Length of zSql in bytes. */
    sqlite3_stmt** ppStmt, /* OUT: A pointer to the prepared statement */
    const char** pzTail) { /* OUT: End of parsed string */
#ifdef _DEBUG
    std::string func_name = vDebugLabel;
    ez::str::replaceString(func_name, "DataBase::", "");
    ez::file::saveStringToFile(                     //
        sql_query,                                  //
        ez::file::correctSlashTypeForFilePathName(  //
            ez::str::toStr("sqlite3/%s.sql", func_name.c_str())));
#endif
    return sqlite3_prepare_v2(db, sql_query, nBytes, ppStmt, pzTail);
}
