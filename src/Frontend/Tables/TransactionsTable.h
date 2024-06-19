#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

class TransactionsTable : public ADataTable {
private:
    std::vector<Transaction> m_Transactions;

public:
    TransactionsTable();
    ~TransactionsTable();

    bool load();
    void unload();
    bool drawMenu();

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    double m_getItemAmount(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick() final;

private:
    void m_updateTransactions();
};
