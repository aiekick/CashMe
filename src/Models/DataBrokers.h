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

class DataBrokers : public conf::ConfigAbstract {
private:
    typedef std::string DataBrokerName;
    std::map<DataBrokerName, Cash::BankStatementModulePtr> m_DataBrokerModules;
    std::vector<DataBrokerName> m_DataBrokerNames;
    int32_t m_BrokerComboIdx = 0;
    Cash::BankStatementModuleWeak m_SelectedBroker;

public:
    bool init();
    void unit();

    void load();

    bool draw();
    void drawDialogs();
    bool drawImportMenu();
    bool drawCreationMenu();

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    bool m_DrawPluginBrockers();
    bool m_DrawDBSymbols();
    void m_Clear();
    void m_GetAvailableDataBrokers();
    void m_SelectBrocker(const DataBrokerName& vName);
    void m_RefreshSymbols();

    // ImGui funcs
    void m_DrawAccountCreation();

public:  // singleton
    static DataBrokers* Instance() {
        static DataBrokers _instance;
        return &_instance;
    }
};
