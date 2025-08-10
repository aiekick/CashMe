#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>
#include <Frontend/Graph/BudgetGraph.h>

class BudgetTable : public ADataTable {
private:
    BudgetGraph m_budgetGraph;
    std::vector<BudgetOutput> m_budgets;
    bool m_showTable{true};
    bool m_showGraph{true};
    uint32_t m_projectedDays{190};
    bool m_useOptional{true};
    bool m_hideEmptyRows{false};

public:
    BudgetTable();
    ~BudgetTable() = default;

    void clear();
    void refreshDatas() final;

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    bool m_drawMenu() final;
    void m_draw(const ImVec2& vSize) final;

private:
    bool m_computeBudget();
};
