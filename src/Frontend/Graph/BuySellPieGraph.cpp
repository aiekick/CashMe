#include <Frontend/Graph/BuySellPieGraph.h>
#include <map>
#include <set>
#include <cmath>

void BuySellPieGraph::clear() {
    m_labels.clear();
    m_labelPtrs.clear();
    m_values.clear();
    m_count = 0;
}

void BuySellPieGraph::prepare(const std::vector<BuySellStatItem>& vItems, const int32_t& vMonthsWindow) {
    clear();

    // keep only the most recent vMonthsWindow distinct months
    std::set<std::string> months;
    for (const auto& item : vItems) {
        months.insert(item.month);
    }
    std::set<std::string> keptMonths;
    int32_t kept = 0;
    for (auto rit = months.rbegin(); rit != months.rend() && kept < vMonthsWindow; ++rit, ++kept) {
        keptMonths.insert(*rit);
    }

    // sum the net amount per group over the kept months
    std::map<std::string, double> aggregated;
    for (const auto& item : vItems) {
        if (keptMonths.find(item.month) != keptMonths.end()) {
            aggregated[item.group] += item.amount;
        }
    }

    // a pie needs non negative values : we use the magnitude of the net amount per group
    for (const auto& pair : aggregated) {
        const double magnitude = std::abs(pair.second);
        if (magnitude > 0.0) {
            m_labels.push_back(pair.first);
            m_values.push_back(magnitude);
        }
    }
    m_count = static_cast<int32_t>(m_labels.size());
    m_labelPtrs.resize(m_count);
    for (int32_t idx = 0; idx < m_count; ++idx) {
        m_labelPtrs[idx] = m_labels[idx].c_str();
    }
}

void BuySellPieGraph::draw(const ImVec2& vSize) {
    if (m_count == 0) {
        ImGui::TextUnformatted("No data");
        return;
    }
    if (ImPlot::BeginPlot("##BuySellPie", vSize, ImPlotFlags_Equal | ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxesLimits(0.0, 1.0, 0.0, 1.0);
        ImPlot::PlotPieChart(m_labelPtrs.data(), m_values.data(), m_count, 0.5, 0.5, 0.45, "%.0f", 90.0, ImPlotPieChartFlags_Normalize);
        ImPlot::EndPlot();
    }
}
