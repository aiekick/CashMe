#pragma once

#include <map>
#include <vector>
#include <string>
#include <imguipack.h>
#include <Headers/DatasDef.h>

// stacked bars : one colored serie per field value, x axis = months
class BuySellBarsGraph {
private:
    std::vector<std::string> m_groupLabels;       // series labels (field values), ordered by index
    std::vector<const char*> m_groupLabelPtrs;    // c_str of m_groupLabels, for ImPlot
    std::map<std::string, int32_t> m_groupIndex;  // field value -> serie index (also colormap index)
    std::vector<std::string> m_monthLabels;       // x tick labels, ordered
    std::vector<const char*> m_monthLabelPtrs;    // c_str of m_monthLabels
    std::vector<double> m_monthPositions;         // x positions 0..monthCount-1
    std::vector<double> m_values;                 // groupCount * monthCount, row major [serie * monthCount + month]
    std::vector<double> m_monthTotals;            // net total per month
    int32_t m_groupCount{};
    int32_t m_monthCount{};

public:
    void clear();
    void prepare(const std::vector<BuySellStatItem>& vItems);
    void draw(const ImVec2& vSize);
    // colormap color of a given field value (matches the bars), used to color the list rows
    ImVec4 getGroupColor(const std::string& vGroup) const;

private:
    // finds the stacked block under the mouse and shows a tooltip for it
    void m_drawHoveredTooltip(const double& vBarGroupSize) const;
};
