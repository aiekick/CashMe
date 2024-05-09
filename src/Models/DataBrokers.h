#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>

#include <json/json.hpp>
#include <apis/CashMePluginApi.h>
#include <ctools/ConfigAbstract.h>
#include <ImGuiPack/ImGuiPack.h>
#include <Models/DataBase.h>

enum ImportTypeEnum {
    IMPORT_FROM_BROCKER_PLUGIN = 5477, // 0 is not used, because thos enum will be passed as a userdatas, and the userdatas default value when not used is nullptr so 0
    IMPORT_FROM_METATREADER_FILE,
    IMPORT_Count
};

class DataBrokers : public conf::ConfigAbstract {
private:
    typedef std::string DataBrokerName;
    std::map<DataBrokerName, Cash::BankStatementModulePtr> m_DataBrokerModules;
    std::vector<DataBrokerName> m_DataBrokerNames;
    int32_t m_BrokerComboIdx = 0;
    Cash::BankStatementModuleWeak m_SelectedBroker;
    std::array<char, 32> m_Symmbol;
    std::array<char, 32> m_StartDate;
    std::array<char, 32> m_EndDate;
    bool m_ShowStartDatePicker = false;
    bool m_ShowEndDatePicker = false;
    int m_StartDatePickerLevel = 0; // day level
    ImPlotTime m_StartDatePicker;
    int m_EndDatePickerLevel = 0;  // day level
    ImPlotTime m_EndDatePicker;
    std::map<Market, std::map<Symbol, std::map<TimeFrame, BarsCount>>> m_DBSymbols;

public:
    bool init();
    void unit();

    void load();

    bool draw();
    void drawDialogs();

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    bool m_DrawPluginBrockers();
    bool m_DrawDBSymbols();
    void m_Clear();
    void m_GetAvailableDataBrokers();
    void m_SelectBrocker(const DataBrokerName& vName);
    void m_RefreshSymbols();


private:  // imports from files
    void m_ImportFromFiles(const std::string& vFilePathName, const ImportTypeEnum& vType);
    void m_ImportCSV(const std::string& vFilePathName, const ImportTypeEnum& vType);
    std::time_t m_convertMetaTraderDateTimeToEpoch(const std::string& vDate, const std::string& vTime);

public:  // singleton
    static DataBrokers* Instance() {
        static DataBrokers _instance;
        return &_instance;
    }
};
