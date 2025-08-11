#pragma once

#include <vector>
#include <imguipack.h>
#include <Headers/DatasDef.h>


class BudgetGraph {
public:
    struct BudgetGraphInput {
        std::vector<BudgetOutput> budgetDatas;
        int32_t monthTransactionsDatesCount{};
        double lastMonthBalance{};
    };

private:
    BudgetGraphInput m_budgetGraphInput;
    std::vector<double> m_dates;
    std::vector<double> m_soldeMin;
    std::vector<double> m_soldeMax;
    std::vector<double> m_soldeReal;
    std::vector<double> m_soldeRealDates;
    std::vector<double> m_soldeRealOpen;
    std::vector<double> m_soldeRealClose;
    std::vector<double> m_soldeRealDelta;

public:
    void prepare(const BudgetGraphInput& vBudgetGraphInput);
    void draw(const ImVec2& vSize);
};
