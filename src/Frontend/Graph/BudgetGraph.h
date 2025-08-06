#pragma once

#include <vector>
#include <imguipack.h>
#include <Headers/DatasDef.h>

class BudgetGraph {
private:
    std::vector<BudgetOutput> m_budgets;
    std::vector<double> m_dates;
    std::vector<double> m_soldeMin;
    std::vector<double> m_soldeMax;
    std::pair<double, double> m_soldeRange{};

public:
    void prepare(const std::vector<BudgetOutput>& vBudgets);
    void draw(const ImVec2& vSize);
};
