#pragma once

#include <map>
#include <set>
#include <array>
#include <vector>
#include <string>

#include <json/json.hpp>
#include <Models/DataBase.h>
#include <Headers/DatasDef.h>
#include <ImGuiPack/ImGuiPack.h>
#include <apis/CashMePluginApi.h>
#include <ctools/ConfigAbstract.h>
#include <Systems/FrameActionSystem.h>

#include <Frontend/Dialogs/BankDialog.h>
#include <Frontend/Dialogs/AccountDialog.h>
#include <Frontend/Dialogs/CategoryDialog.h>
#include <Frontend/Dialogs/OperationDialog.h>
#include <Frontend/Dialogs/TransactionDialog.h>

class DataBrokers : public conf::ConfigAbstract {
private:
    struct Datas {
        std::vector<BankName> bankNames;
        std::vector<CategoryName> categoryNames;
        std::vector<OperationName> operationNames;
        std::vector<Account> accounts;
        std::vector<AccountNumber> accountNumbers;
        std::vector<Transaction> transactions;
        std::vector<Transaction> transactions_filtered;
        std::set<RowID> transactions_filtered_rowids;
        void clear() {
            accounts.clear();
            bankNames.clear();
            transactions.clear();
            categoryNames.clear();
            accountNumbers.clear();
            operationNames.clear();
            transactions_filtered.clear();
        }
    } m_Datas;

private:
    using DataBrokerName = std::string;
    using DataBrokerWay = std::string;
    std::map<DataBrokerName, std::map<DataBrokerWay, Cash::BankStatementModulePtr>> m_DataBrokerModules;
    Cash::BankStatementModuleWeak m_SelectedBroker;
    ImGuiListClipper m_TransactionsListClipper;
    ImGuiListClipper m_TransactionsDeletionListClipper;
    size_t m_SelectedAccountIdx = 0U;

    BankDialog m_BankDialog;
    AccountDialog m_AccountDialog;
    CategoryDialog m_CategoryDialog;
    OperationDialog m_OperationDialog;
    TransactionDialog m_TransactionDialog;

    TransactionAmount m_CurrentBaseSolde = 0.0;
    TransactionAmount m_TotalDebit = 0.0;
    TransactionAmount m_TotalCredit = 0.0;

    // search for date, desc, comm, cat, ope
    std::array<ImWidgets::InputText, 5> m_SearchInputTexts;
    std::array<std::string, 5> m_SearchTokens;

    // selection
    std::set<RowID> m_SelectedTransactions;

    // accounts display
    std::map<BankName, std::map<BankAgency, std::map<AccountNumber, Account>>> m_Accounts;

    // hidden mode
    bool m_HiddenMode = false;

public:
    bool init();
    void unit();

    void load();

    bool draw();
    void drawDialogs(const ImVec2& vPos, const ImVec2& vSize);
    void drawMenu(FrameActionSystem& vFrameActionSystem);

    void DisplayTransactions();

    void RefreshDatas();

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    void m_drawAccountsMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawCreationMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawImportMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawSelectMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawDebugMenu(FrameActionSystem& vFrameActionSystem);
    void m_Clear();
    void m_GetAvailableDataBrokers();
    void m_ImportFromFiles(const std::vector<std::string> vFiles);
    void m_ResetFiltering();
    void m_RefreshFiltering();
    void m_SelectOrDeselectRow(const Transaction& vTransaction);
    bool m_IsRowSelected(const RowID& vRowID) const;
    void m_ResetSelection();
    void m_SelectCurrentRows();
    void m_SelectPossibleDuplicateEntryOnPricesAndDates();
    void m_SelectUnConfirmedTransactions();
    bool m_IsHiddenMode();
    void m_HideByFilledRectForHiddenMode(const char* fmt, ...);
    void m_DisplayAlignedWidget(  //
        const float& vWidth,
        const std::string& vLabel,
        const float& vOffsetFromStart,
        std::function<void()> vWidget);

private:  // ImGui
    void m_UpdateBanks();
    void m_UpdateAccounts();
    void m_UpdateCategories();
    void m_UpdateOperations();
    void m_UpdateTransactions(const RowID& vAccountID);

    void m_drawAccountMenu(const Account& vAccount);
    void m_drawTransactionMenu(const Transaction& vTransaction);

    void m_drawSearchRow();
    void m_drawAmount(const double& vAmount);

public:  // singleton
    static DataBrokers* Instance() {
        static DataBrokers _instance;
        return &_instance;
    }
};
