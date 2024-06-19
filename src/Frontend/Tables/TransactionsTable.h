#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

#include <Systems/FrameActionSystem.h>

#include <Threads/ImportWorkerThread.h>

#include <Frontend/Dialogs/TransactionDialog.h>

class TransactionsTable : public ADataTable {
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
        std::set<RowID> filtered_selected_transactions;
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

    TransactionDialog m_TransactionDialog;

    double m_CurrentBaseSolde = 0.0;
    double m_TotalDebit = 0.0;
    double m_TotalCredit = 0.0;

    std::array<ImWidgets::InputText, SearchColumns::SEARCH_COLUMN_Count> m_SearchInputTexts;
    std::array<std::string, SearchColumns::SEARCH_COLUMN_Count> m_SearchTokens;
    FilteringMode m_FilteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
    GroupingMode m_GroupingMode = GroupingMode::GROUPING_MODE_TRANSACTIONS;

    // accounts display
    std::map<BankName,                         //
             std::map<BankAgency,              //
                      std::map<AccountNumber,  //
                               Account>>>
        m_Accounts;

public:
    TransactionsTable();
    ~TransactionsTable();

    bool Init();
    void Unit();

    bool load() final;
    void unload() final;
    bool drawMenu() final;

    TransactionDialog& getTransactionDialogRef();

    void clear();    
    void refreshDatas();
    void refreshFiltering();
    void resetFiltering();

    void drawAccountsMenu(FrameActionSystem& vFrameActionSystem);
    void drawSelectMenu(FrameActionSystem& vFrameActionSystem);
    void drawDebugMenu(FrameActionSystem& vFrameActionSystem);
    void drawGroupingMenu(FrameActionSystem& vFrameActionSystem);

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    double m_getItemBarAmount(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;

private:
    bool m_isGroupingModeTransactions();
    void m_drawSearchRow();
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
    void m_drawAmount(const double& vAmount);
};
