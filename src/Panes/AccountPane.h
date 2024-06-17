#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <ImGuiPack.h>
#include <ctools/ConfigAbstract.h>

#include <Systems/FrameActionSystem.h>
#include <Threads/ImportWorkerThread.h>
#include <Frontend/Dialogs/BankDialog.h>
#include <Frontend/Dialogs/AccountDialog.h>
#include <Frontend/Dialogs/EntityDialog.h>
#include <Frontend/Dialogs/CategoryDialog.h>
#include <Frontend/Dialogs/OperationDialog.h>
#include <Frontend/Dialogs/TransactionDialog.h>

class ProjectFile;
class AccountPane : public AbstractPane, public conf::ConfigAbstract {
private:
    struct Datas {
        std::vector<BankName> bankNames;
        std::vector<EntityName> entityNames;
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

    DataBrockerContainer m_DataBrokerModules;
    Cash::BankStatementModuleWeak m_SelectedBroker;
    ImGuiListClipper m_TransactionsListClipper;
    size_t m_SelectedAccountIdx = 0U;

    ImportWorkerThread m_ImportThread;

    BankDialog m_BankDialog;
    AccountDialog m_AccountDialog;
    EntityDialog m_EntityDialog;
    CategoryDialog m_CategoryDialog;
    OperationDialog m_OperationDialog;
    TransactionDialog m_TransactionDialog;

    TransactionAmount m_CurrentBaseSolde = 0.0;
    TransactionAmount m_TotalDebit = 0.0;
    TransactionAmount m_TotalCredit = 0.0;

    std::array<ImWidgets::InputText, SearchColumns::SEARCH_COLUMN_Count> m_SearchInputTexts;
    std::array<std::string, SearchColumns::SEARCH_COLUMN_Count> m_SearchTokens;
    FilteringMode m_FilteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
    GroupingMode m_GroupingMode = GroupingMode::GROUPING_MODE_TRANSACTIONS;

    // selection
    std::set<RowID> m_SelectedTransactions;
    std::set<RowID> m_FilteredSelectedTransactions;
    int32_t m_CurrSelectedItemIdx = -1;
    int32_t m_LastSelectedItemIdx = -1;

    // accounts display
    std::map<BankName, std::map<BankAgency, std::map<AccountNumber, Account>>> m_Accounts;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void DoBackend();

    void Load();

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
    void m_drawMenu(FrameActionSystem& vFrameActionSystem);
    void m_displayTransactions();
    void m_refreshDatas();

    void m_drawAccountsMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawCreationMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawImportMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawSelectMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawDebugMenu(FrameActionSystem& vFrameActionSystem);
    void m_drawGroupingMenu(FrameActionSystem& vFrameActionSystem);

    void m_Clear();
    void m_GetAvailableDataBrokers();
    void m_ResetFiltering();
    void m_refreshFiltering();
    void m_SelectOrDeselectRow(const Transaction& vTransaction);
    bool m_IsRowSelected(const RowID& vRowID) const;
    void m_ResetSelection();
    void m_SelectCurrentRows();
    void m_FilterSelection();
    void m_SelectPossibleDuplicateEntryOnPricesAndDates();
    void m_SelectUnConfirmedTransactions();
    void m_SelectEmptyColumn(const SearchColumns& vColumn);
    void m_GroupTransactions(const GroupingMode& vGroupingMode);
    void m_UpdateBanks();
    void m_UpdateAccounts();
    void m_UpdateEntities(); 
    void m_UpdateCategories();
    void m_UpdateOperations();
    void m_UpdateTransactions(const RowID& vAccountID);

    bool m_IsGroupingModeTransactions();

    void m_drawAccountMenu(const Account& vAccount);
    void m_drawTransactionMenu(const Transaction& vTransaction);
    void m_drawSearchRow();
    void m_drawAmount(const double& vAmount);

    void m_ImportFromFiles(const std::vector<std::string>& vFiles);

public:  // singleton
    static std::shared_ptr<AccountPane> Instance() {
        static std::shared_ptr<AccountPane> _instance = std::make_shared<AccountPane>();
        return _instance;
    }

public:
    AccountPane();                              // Prevent construction
    AccountPane(const AccountPane&) = delete;  // Prevent construction by copying
    AccountPane& operator=(const AccountPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~AccountPane();  // Prevent unwanted destruction};
};
