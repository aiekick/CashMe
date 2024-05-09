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

#pragma once

#include <memory>
#include <string>
#include <functional>

typedef std::string DBFile;
typedef std::string Market;
typedef std::string Symbol;
typedef double Date; // epoch with millisecond part epoch.millisecond
typedef uint32_t TimeFrame;
typedef uint32_t BarsCount;
typedef double OpenPrice;
typedef double HighPrice;
typedef double LowPrice;
typedef double ClosePrice;
typedef double VolumePrice;

struct sqlite3;
class DataBase {
private:
    sqlite3* m_SqliteDB = nullptr;
    std::string m_DataBaseFilePathName = "datas.db3";
    bool m_TransactionStarted = false;
    char* m_LastErrorMsg = nullptr;

public:
    bool IsFileASqlite3DB(const DBFile& vDBFilePathName);
    bool CreateDBFile(const DBFile& vDBFilePathName);
    bool OpenDBFile(const DBFile& vDBFilePathName);
    void CloseDBFile();
    bool BeginTransaction();
    void CommitTransaction();
    void RollbackTransaction();
    void AddMarket(const Market& vMarket);
    void AddSymbol(const Symbol& vSymbol);
    void AddTimeFrame(const TimeFrame& vTimeFrame);
    void AddMarketSymbolTimeFrame(const Market& vMarket, const Symbol& vSymbol, const TimeFrame& vTimeFrame);
    void AddDate(const Date& vDate);
    void AddPrice(const Market& vMarket,
                  const Symbol& vSymbol,
                  const TimeFrame& vTimeFrame,
                  const Date& vDate,
                  const OpenPrice& vOpen,
                  const HighPrice& vHigh,
                  const LowPrice& vLow,
                  const ClosePrice& vClose,
                  const VolumePrice& vVolume);
    void GetSymbols(std::function<void(const Market&, const Symbol&, const TimeFrame&, const BarsCount&)> vCallback);
    void GetPrices(
        const Market& vMarket,
        const Symbol& vSymbol,
        const TimeFrame& vTimeFrame,
        std::function<void(const Date&, const OpenPrice&, const HighPrice&, const LowPrice&, const ClosePrice&, const VolumePrice&)>
            vCallback);
    void ClearDataTables();
    std::string GetLastErrorMesg();
    bool SetSettingsXMLDatas(const std::string& vXMLDatas);
    std::string GetSettingsXMLDatas();

private:
    bool m_OpenDB();
    void m_CloseDB();
    bool m_CreateDB();
    void m_CreateDBTables(const bool& vPrintLogs = true);

    /// <summary>
    /// enable foreign key (must be done at each connections)
    /// </summary>
    void m_EnableForeignKey();

public:  // singleton
    static std::shared_ptr<DataBase> Instance() {
        static std::shared_ptr<DataBase> _instance = std::make_shared<DataBase>();
        return _instance;
    }

public:
    DataBase() = default;                // Prevent construction
    DataBase(const DataBase&) = delete;  // Prevent construction by copying
    DataBase& operator=(const DataBase&) {
        return *this;
    };                              // Prevent assignment
    virtual ~DataBase() = default;  // Prevent unwanted destruction};
};