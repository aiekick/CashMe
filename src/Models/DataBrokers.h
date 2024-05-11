#pragma once

#include <map>
#include <set>
#include <array>
#include <vector>
#include <string>

#include <json/json.hpp>
#include <Models/DataBase.h>
#include <ImGuiPack/ImGuiPack.h>
#include <apis/CashMePluginApi.h>
#include <ctools/ConfigAbstract.h>
#include <Systems/FrameActionSystem.h>

class DataBrokers : public conf::ConfigAbstract {
private:
    struct Datas {
        std::vector<UserName> userNames;
        std::vector<BankName> bankNames;
    } m_Datas;

private:
    typedef std::string DataBrokerName;
    typedef std::string DataBrokerWay;
    std::map<DataBrokerName, std::map<DataBrokerWay, Cash::BankStatementModulePtr>> m_DataBrokerModules;
    Cash::BankStatementModuleWeak m_SelectedBroker;
    bool m_showUserCreation = false;
    bool m_showBankCreation = false;
    bool m_showAccountCreation = false;
    ImWidgets::InputText m_UserNameInputText;
    ImWidgets::InputText m_BankNameInputText;
    ImWidgets::InputText m_BankUrlInputText;
    ImWidgets::InputText m_AccountNameInputText;
    ImWidgets::InputText m_AccountTypeInputText;
    ImWidgets::InputText m_AccountNumberInputText;
    ImWidgets::QuickStringCombo m_UsersCombo;
    ImWidgets::QuickStringCombo m_BanksCombo;

public:
    bool init();
    void unit();

    void load();

    bool draw();
    void drawDialogs(const ImVec2& vPos, const ImVec2& vSize);
    void drawMenu(FrameActionSystem& vFrameActionSystem);

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    void m_drawImportMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawCreationMenu(FrameActionSystem& vFrameActionSystem);
    bool m_DrawPluginBrockers();
    bool m_DrawDBSymbols();
    void m_Clear();
    void m_GetAvailableDataBrokers();
    void m_RefreshSymbols();

private: // ImGui   
    void m_ShowUserCreationDialog();
    void m_DrawUserCreationDialog(const ImVec2& vPos);

    void m_ShowBankCreationDialog();
    void m_DrawBankCreationDialog(const ImVec2& vPos);

    void m_ShowAccountCreationDialog();
    void m_DrawAccountCreationDialog(const ImVec2& vPos);

public:  // singleton
    static DataBrokers* Instance() {
        static DataBrokers _instance;
        return &_instance;
    }
};
