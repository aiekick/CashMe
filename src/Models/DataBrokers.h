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
        AccounBaseSolde base_solde = 0.0;
        TransactionsCount count = 0U;
    };
    struct Transaction {
        TransactionDate date;
        TransactionDescription desc;
        TransactionComment comm;
        CategoryName category;
        OperationName operation;
        TransactionAmount amount = 0.0;
        TransactionSolde solde = 0.0;
        // desc, cat, op
        std::array<std::string, 3> optimized; // 
    };
    struct Datas {
        std::vector<UserName> userNames;
        std::vector<BankName> bankNames;
        std::vector<CategoryName> categoryNames;
        std::vector<OperationName> operationNames;
        std::vector<Account> accounts;
        std::vector<AccountNumber> accountNumbers;
        std::vector<Transaction> transactions;
        std::vector<Transaction> transactions_filtered;
    } m_Datas;
    enum class DialogMode {  //
        CREATION = 0,
        UPDATE
    };

private:
    typedef std::string DataBrokerName;
    typedef std::string DataBrokerWay;
    std::map<DataBrokerName, std::map<DataBrokerWay, Cash::BankStatementModulePtr>> m_DataBrokerModules;
    Cash::BankStatementModuleWeak m_SelectedBroker;
    ImGuiListClipper m_TransactionsListClipper;
    size_t m_SelectedAccountIdx = 0U;

    DialogMode m_dialogMode = DialogMode::CREATION;

    bool m_showUserDialog = false;
    ImWidgets::InputText m_UserNameInputText;
    ImWidgets::QuickStringCombo m_UsersCombo;

    bool m_showBankDialog = false;
    ImWidgets::InputText m_BankNameInputText;
    ImWidgets::InputText m_BankUrlInputText;
    ImWidgets::QuickStringCombo m_BanksCombo;

    bool m_showCategoryDialog = false;
    ImWidgets::InputText m_CategoryNameInputText;
    ImWidgets::QuickStringCombo m_CategoriesCombo;

    bool m_showOperationDialog = false;
    ImWidgets::InputText m_OperationNameInputText;
    ImWidgets::QuickStringCombo m_OperationsCombo;

    bool m_showAccountDialog = false;
    ImWidgets::InputText m_AccountNameInputText;
    ImWidgets::InputText m_AccountTypeInputText;
    ImWidgets::InputText m_AccountNumberInputText;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    double m_AccountBaseSoldeInputDouble = 0.0;

    bool m_showTransactionDialog = false;
    ImWidgets::InputText m_TransactionDateInputText;
    ImWidgets::InputText m_TransactionDescriptionInputText;
    ImWidgets::InputText m_TransactionCommentInputText;
    double m_TransactionAmountInputDouble = 0.0;

    TransactionAmount m_CurrentBaseSolde = 0.0;
    TransactionAmount m_TotalDebit = 0.0;
    TransactionAmount m_TotalCredit = 0.0;

    // search for desc, cat, ope
    std::array<ImWidgets::InputText, 3> m_SearchInputTexts;
    std::array<std::string, 3> m_SearchTokens;

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
    void m_drawUpdateMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawImportMenu(FrameActionSystem& vFrameActionSystem);
    void m_refreshDatas();
    void m_Clear();
    void m_GetAvailableDataBrokers();
    void m_ImportFromFiles(const std::vector<std::string> vFiles);
    void m_ResetFiltering();
    void m_RefreshFiltering();

private:  // ImGui
    void m_UpdateUsers();
    void m_ShowUserDialog(const DialogMode& vDialogMode);
    void m_DrawUserDialog(const ImVec2& vPos);

    void m_UpdateBanks();
    void m_ShowBankDialog(const DialogMode& vDialogMode);
    void m_DrawBankDialog(const ImVec2& vPos);

    void m_UpdateCategories();
    void m_ShowCategoryDialog(const DialogMode& vDialogMode);
    void m_DrawCategoryDialog(const ImVec2& vPos);

    void m_UpdateOperations();
    void m_ShowOperationDialog(const DialogMode& vDialogMode);
    void m_DrawOperationDialog(const ImVec2& vPos);

    void m_UpdateAccounts();
    void m_ShowAccountDialog(const DialogMode& vDialogMode);
    void m_DrawAccountDialog(const ImVec2& vPos);

    void m_UpdateTransactions(const RowID& vAccountID);
    void m_ShowTransactionDialog(const DialogMode& vDialogMode);
    void m_DrawTransactionDialog(const ImVec2& vPos);

    void m_drawSearchRow();
    void m_drawAmount(const double& vAmount);

public:  // singleton
    static DataBrokers* Instance() {
        static DataBrokers _instance;
        return &_instance;
    }
};
