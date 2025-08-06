#include <Frontend/Graph/BudgetGraph.h>
#include <cstdint>

void BudgetGraph::prepare(const std::vector<BudgetOutput>& vBudgets) {
    m_budgets = vBudgets;
    m_soldeRange = {};
    m_dates.clear();
    m_soldeMin.clear();
    m_soldeMax.clear();
    m_dates.reserve(vBudgets.size());
    m_soldeMin.reserve(vBudgets.size());
    m_soldeMax.reserve(vBudgets.size());
    for (const auto& budget : vBudgets) {
        m_dates.push_back(static_cast<double>(budget.dateEpoch));
        m_soldeMin.push_back(budget.solde.min);
        m_soldeMax.push_back(budget.solde.max);
        if (m_soldeRange.first > budget.solde.min) {
            m_soldeRange.first = budget.solde.min;
        }
        if (m_soldeRange.second < budget.solde.max) {
            m_soldeRange.second = budget.solde.max;
        }
    }
}

template <typename T>
int BinarySearch(const T* arr, int l, int r, T x) {
    if (r >= l) {
        int mid = l + (r - l) / 2;
        if (arr[mid] == x)
            return mid;
        if (arr[mid] > x)
            return BinarySearch(arr, l, mid - 1, x);
        return BinarySearch(arr, mid + 1, r, x);
    }
    return -1;
}

void BudgetGraph::draw(const ImVec2& vSize) {
    if (ImPlot::BeginPlot("Projected balance", vSize)) {
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        const auto& countDatas = static_cast<int32_t>(m_dates.size());
        if (countDatas > 0) {
            auto* p_drawList = ImPlot::GetPlotDrawList();
            if (ImPlot::IsPlotHovered()) {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                mouse.x = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
                int idx = BinarySearch(m_dates.data(), 0, countDatas - 1, mouse.x);
                if (idx != -1) {
                    ImGui::BeginTooltip();
                    char buff[32];
                    ImPlot::FormatDate(ImPlotTime::FromDouble(m_dates.at(idx)), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);
                    const auto& item = m_budgets.at(idx);
                    ImGui::Text("Day:   %s", buff);
                    ImGui::Text("Balance min: %.2f", item.solde.min);
                    ImGui::Text("Balane max: %.2f", item.solde.max);
                    ImGui::Text("Delta min: %.2f", item.delta.min);
                    ImGui::Text("Delta max: %.2f", item.delta.max);
                    ImGui::Text("Incomes min: %s", item.incomesMinAmount.c_str());
                    ImGui::Text("Incomes max: %s", item.incomesMaxAmount.c_str());
                    const float tx = ImPlot::PlotToPixels(mouse.x, mouse.y).x;
                    const float ttop = ImPlot::GetPlotPos().y;
                    const float tbot = ttop + ImPlot::GetPlotSize().y;
                    ImPlot::PushPlotClipRect();
                    p_drawList->AddLine(ImVec2(tx, ttop), ImVec2(tx, tbot), ImGui::GetColorU32(ImVec4(1, 1, 0, 1)), 2.0f);
                    ImPlot::PopPlotClipRect();
                    ImGui::EndTooltip();
                }
            }
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::PlotShaded("Balance range", m_dates.data(), m_soldeMin.data(), m_soldeMax.data(), countDatas);
            ImPlot::PlotLine("Balance min", m_dates.data(), m_soldeMin.data(), countDatas);
            ImPlot::PlotLine("Balance max", m_dates.data(), m_soldeMax.data(), countDatas);
            ImPlot::PopStyleVar();
        }
        ImPlot::EndPlot();
    }
}
