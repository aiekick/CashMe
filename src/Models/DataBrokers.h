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
    struct Account {
        RowID id = 0;
        UserName user;
        BankName bank;
        AccountType type;
        AccountName name;
        AccountNumber number;
    };
    struct Transaction {
        TransactionDate date;
        TransactionDescription desc;
        CategoryName category;
        OperationName operation;
        TransactionAmount amount = 0.0;
    };
    struct Datas {
        std::vector<UserName> userNames;
        std::vector<BankName> bankNames;
        std::vector<CategoryName> categoryNames;
        std::vector<OperationName> operationNames;
        std::vector<Account> accounts;
        std::vector<AccountNumber> accountNumbers;
        std::vector<Transaction> transactions;
    } m_Datas;

private:
    typedef std::string DataBrokerName;
    typedef std::string DataBrokerWay;
    std::map<DataBrokerName, std::map<DataBrokerWay, Cash::BankStatementModulePtr>> m_DataBrokerModules;
    Cash::BankStatementModuleWeak m_SelectedBroker;

    bool m_showUserCreation = false;
    ImWidgets::InputText m_UserNameInputText;
    ImWidgets::QuickStringCombo m_UsersCombo;

    bool m_showBankCreation = false;
    ImWidgets::InputText m_BankNameInputText;
    ImWidgets::InputText m_BankUrlInputText;
    ImWidgets::QuickStringCombo m_BanksCombo;

    bool m_showCategoryCreation = false;
    ImWidgets::InputText m_CategoryNameInputText;
    ImWidgets::QuickStringCombo m_CategoriesCombo;

    bool m_showOperationCreation = false;
    ImWidgets::InputText m_OperationNameInputText;
    ImWidgets::QuickStringCombo m_OperationsCombo;

    bool m_showAccountCreation = false;
    ImWidgets::InputText m_AccountNameInputText;
    ImWidgets::InputText m_AccountTypeInputText;
    ImWidgets::InputText m_AccountNumberInputText;
    ImWidgets::QuickStringCombo m_AccountsCombo;

    bool m_showTransactionCreation = false;
    ImWidgets::InputText m_TransactionDateInputText;
    ImWidgets::InputText m_TransactionDescriptionInputText;
    double m_TransactionAmountInputDouble = 0.0;


public:
    bool init();
    void unit();

    void load();

    bool draw();
    void drawDialogs(const ImVec2& vPos, const ImVec2& vSize);
    void drawMenu(FrameActionSystem& vFrameActionSystem);

    void DisplayAccounts();
    void DisplayTransactions();

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    void m_drawRefreshMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawCreationMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawImportMenu(FrameActionSystem& vFrameActionSystem);
    void m_refreshDatas();
    void m_Clear();
    void m_GetAvailableDataBrokers();

private:  // ImGui
    void m_UpdateUsers();
    void m_ShowUserCreationDialog();
    void m_DrawUserCreationDialog(const ImVec2& vPos);

    void m_UpdateBanks();
    void m_ShowBankCreationDialog();
    void m_DrawBankCreationDialog(const ImVec2& vPos);

    void m_UpdateCategories();
    void m_ShowCategoryCreationDialog();
    void m_DrawCategoryCreationDialog(const ImVec2& vPos);

    void m_UpdateOperations();
    void m_ShowOperationCreationDialog();
    void m_DrawOperationCreationDialog(const ImVec2& vPos);

    void m_UpdateAccounts();
    void m_ShowAccountCreationDialog();
    void m_DrawAccountCreationDialog(const ImVec2& vPos);

    void m_UpdateTransactions(const RowID& vAccountID);
    void m_ShowTransactionCreationDialog();
    void m_DrawTransactionCreationDialog(const ImVec2& vPos);

public:  // singleton
    static DataBrokers* Instance() {
        static DataBrokers _instance;
        return &_instance;
    }
};
