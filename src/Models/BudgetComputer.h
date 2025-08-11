#pragma once
#include <Headers/DatasDef.h>
#include <vector>

class BudgetComputer {
private:
    std::vector<BudgetOutput> m_budgets;
    int32_t m_countMonthTransactionsDates{};
    double m_lastMonthBalance{};
    size_t m_soldeRealCount{};

public:
    void clear();
    bool compute(const RowID vAccountID, const std::pair<int32_t,uint32_t>& vMonthRangeOffset, const bool vUseOptional);
    const std::vector<BudgetOutput>& getBudgetDatas() const;
    uint32_t getCountMonthTransactionsDates() const;
    double getLastMonthBalance() const;
    size_t getSoldeRealCount() const;
};
