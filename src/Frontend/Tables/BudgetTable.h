#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

#include <Systems/FrameActionSystem.h>

#include <Threads/ImportWorkerThread.h>

class BudgetTable : public ADataTable {
private:
    std::vector<BudgetOutput> m_budgets;

public:
    BudgetTable();
    ~BudgetTable() = default;

    bool init();
    void unit();

    bool load() final;
    void unload() final;
    bool drawMenu() final;

    void clear();
    void refreshDatas();
    void drawDebugMenu(FrameActionSystem& vFrameActionSystem);
    void drawAccountsMenu(FrameActionSystem& vFrameActionSystem);

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;
    void m_refreshDatas() final;
    void m_updateDatas(const RowID& vAccountID) final;

    void m_UpdateBudget();
};
