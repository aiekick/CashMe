#pragma once

#include <Frontend/Tables/abstract/ADataBarsTable.h>
#include <Systems/FrameActionSystem.h>

#include <Threads/ImportWorkerThread.h>

class TransactionsTable : public ADataBarsTable {
private:
    struct Datas {
        std::vector<BankName> bankNames;
        std::vector<EntityName> entityNames;
        std::vector<CategoryName> categoryNames;
        std::vector<OperationName> operationNames;
        std::vector<TransactionOutput> transactions;
        std::vector<TransactionOutput> transactions_filtered;
        std::set<RowID> transactions_filtered_rowids;
        std::set<RowID> filtered_selected_transactions;
        void clear() {
            bankNames.clear();
            transactions.clear();
            categoryNames.clear();
            operationNames.clear();
            transactions_filtered.clear();
        }
    } m_Datas;
    
    double m_TotalDebit = 0.0;
    double m_TotalCredit = 0.0;

    bool m_enableMultilineComment = false;

    std::array<ImWidgets::InputText, SearchColumns::SEARCH_COLUMN_Count> m_SearchInputTexts;
    std::array<std::string, SearchColumns::SEARCH_COLUMN_Count> m_SearchTokens;
    FilteringMode m_filteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
    GroupingMode m_groupingMode = GroupingMode::GROUPING_MODE_TRANSACTIONS;

    // accounts display
    std::map<BankName, std::map<BankAgency, std::map<AccountNumber, AccountOutput>>> m_Accounts;

public:
    TransactionsTable();
    ~TransactionsTable() = default;

    void clear();    
    void refreshDatas() final;
    void refreshFiltering();
    void resetFiltering();

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    double m_getItemBarAmount(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;
    bool m_drawMenu() final;


private:
    bool m_drawSelectMenu();
    bool m_drawGroupingMenu();
    bool m_isGroupingModeTransactions();
    void m_drawSearchRow();
    void m_filterSelection();
    void m_selectPossibleDuplicateEntryOnPricesAndDates();
    void m_selectUnConfirmedTransactions();
    void m_selectEmptyColumn(const SearchColumns& vColumn);
    void m_groupTransactions(const GroupingMode& vGroupingMode);
    void m_updateBanks();
    void m_updateEntities();
    void m_updateCategories();
    void m_updateOperations();
    void m_updateTransactions();
};
