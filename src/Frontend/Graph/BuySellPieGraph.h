#pragma once

#include <vector>
#include <string>
#include <imguipack.h>
#include <Headers/DatasDef.h>

// pie chart : field value distribution over the last N months (by magnitude)
class BuySellPieGraph {
private:
    std::vector<std::string> m_labels;
    std::vector<const char*> m_labelPtrs;
    std::vector<double> m_values;
    int32_t m_count{};

public:
    void clear();
    void prepare(const std::vector<BuySellStatItem>& vItems, const int32_t& vMonthsWindow);
    void draw(const ImVec2& vSize);
};
