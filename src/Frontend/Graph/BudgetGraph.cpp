#include <Frontend/Graph/BudgetGraph.h>
#include <ezlibs/ezDate.hpp>
#include <cstdint>

void BudgetGraph::prepare(const BudgetGraphInput& vBudgetGraphInput) {
    m_budgetGraphInput = vBudgetGraphInput;
    const auto& count = m_budgetGraphInput.budgetDatas.size();
    m_dates.clear();
    m_dates.reserve(count);
    m_soldeMin.clear();
    m_soldeMin.reserve(count);
    m_soldeMax.clear();
    m_soldeMax.reserve(count);
    m_soldeRealDates.clear();
    m_soldeRealDates.reserve(count);
    m_soldeRealOpen.clear();
    m_soldeRealOpen.reserve(count);
    m_soldeRealClose.clear();
    m_soldeRealClose.reserve(count);
    m_soldeReal.clear();
    m_soldeReal.reserve(count);
    m_soldeRealDelta.clear();
    m_soldeRealDelta.reserve(count);
    double lastSoldeReal = vBudgetGraphInput.lastMonthBalance;
    for (size_t idx = 0; idx < m_budgetGraphInput.budgetDatas.size(); ++idx) {
        const auto& budget = m_budgetGraphInput.budgetDatas.at(idx);
        const auto& epoch = static_cast<double>(budget.dateEpoch);
        m_dates.push_back(epoch);
        m_soldeMin.push_back(budget.solde.min);
        m_soldeMax.push_back(budget.solde.max);
        if (idx < m_budgetGraphInput.monthTransactionsDatesCount) {
            m_soldeReal.push_back(budget.soldeReal);
            m_soldeRealDelta.push_back(budget.soldeReal - lastSoldeReal);
            if (lastSoldeReal != budget.soldeReal) {
                m_soldeRealDates.push_back(epoch);
                m_soldeRealOpen.push_back(lastSoldeReal);
                m_soldeRealClose.push_back(budget.soldeReal);
                lastSoldeReal = budget.soldeReal;
            }
        }
    }
}

template <typename T>
int32_t BinarySearch(const T* arr, int32_t l, int32_t r, T x) {
    if (r >= l) {
        int32_t mid = l + (r - l) / 2;
        if (arr[mid] == x) {
            return mid;
        } else if (arr[mid] > x) {
            return BinarySearch(arr, l, mid - 1, x);
        }
        return BinarySearch(arr, mid + 1, r, x);
    }
    return -1;
}

void BudgetGraph::draw(const ImVec2& vSize) {
    static ImVec4 colorCredit{0.1f, 1.0f, 0.1f, 0.6f};
    static ImVec4 colorDebit{1.0f, 0.1f, 0.1f, 0.6f};
    static ImVec4 colorRange{0.1f, 0.4f, 1.0f, 0.6f};
    static double halfbarWithPerecent = 0.45;
    if (ImPlot::BeginPlot("Projected balance", vSize)) {
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        const auto& countDatas = static_cast<int32_t>(m_dates.size());
        if (countDatas > 0) {
            auto* p_drawList = ImPlot::GetPlotDrawList();
            if (ImPlot::IsPlotHovered()) {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                mouse.x = ImPlot::RoundTime(ImPlotTime::FromDouble(mouse.x), ImPlotTimeUnit_Day).ToDouble();
                int32_t idx = BinarySearch(m_dates.data(), 0, countDatas - 1, mouse.x);
                if (idx != -1) {
                    ImGui::BeginTooltip();
                    char buff[32];
                    ImPlot::FormatDate(ImPlotTime::FromDouble(m_dates.at(idx)), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);
                    const auto& item = m_budgetGraphInput.budgetDatas.at(idx);
                    ImGui::Text("Day:   %s", buff);
                    if (idx < m_budgetGraphInput.monthTransactionsDatesCount) {
                        ImGui::Text("Balance: %.2f", m_soldeReal.at(idx));
                        ImGui::Text("Balance delta: %.2f", m_soldeRealDelta.at(idx));
                    } else {
                        ImGui::Text("Balance min: %.2f", item.solde.min);
                        ImGui::Text("Balance max: %.2f", item.solde.max);
                        if (!item.incomesMin.empty()) {
                            ImGui::Text("delta min: %.2f \n\t%s", item.delta.min, item.incomesMin.c_str());
                        }
                        if (!item.incomesMax.empty()) {
                            ImGui::Text("delta max: %.2f \n\t%s", item.delta.max, item.incomesMax.c_str());
                        }
                    }
                    const float tx = ImPlot::PlotToPixels(mouse.x, mouse.y).x;
                    const float ttop = ImPlot::GetPlotPos().y;
                    const float tbot = ttop + ImPlot::GetPlotSize().y;
                    ImPlot::PushPlotClipRect();
                    p_drawList->AddLine(ImVec2(tx, ttop), ImVec2(tx, tbot), ImGui::GetColorU32(ImVec4(1, 1, 0, 1)), 2.0f);
                    ImPlot::PopPlotClipRect();
                    ImGui::EndTooltip();
                }
            }

            const auto& soldeRealCountDatas = static_cast<int32_t>(m_soldeRealDates.size());
            if (ImPlot::BeginItem("Realized balance")) {
                ImPlot::GetCurrentItem()->Color = IM_COL32(64, 64, 64, 255);
                if (ImPlot::FitThisFrame()) {
                    for (int i = 0; i < soldeRealCountDatas; ++i) {
                        ImPlot::FitPoint(ImPlotPoint(m_soldeRealDates.at(i), m_soldeRealOpen.at(i)));
                        ImPlot::FitPoint(ImPlotPoint(m_soldeRealDates.at(i), m_soldeRealClose.at(i)));
                    }
                }
                static const auto& barSpace = 86400.0; // count sec in one day
                auto halfBarWidth = (soldeRealCountDatas > 1) ? barSpace * halfbarWithPerecent : halfbarWithPerecent;
                for (int i = 0; i < soldeRealCountDatas; ++i) {
                    auto center = m_soldeRealDates.at(i);
                    if (i > 0) {
                        center = (center + m_soldeRealDates.at(i - 1) + barSpace) * 0.5;
                        halfBarWidth = (soldeRealCountDatas > 1) ? (m_soldeRealDates.at(i) - m_soldeRealDates.at(i - 1)) * halfbarWithPerecent : halfbarWithPerecent;
                    }
                    const auto open_pos = ImPlot::PlotToPixels(center - halfBarWidth, m_soldeRealOpen.at(i));
                    const auto close_pos = ImPlot::PlotToPixels(center + halfBarWidth, m_soldeRealClose.at(i));
                    const auto color = ImGui::GetColorU32((m_soldeRealOpen.at(i) > m_soldeRealClose.at(i)) ? colorDebit : colorCredit);
                    p_drawList->AddRectFilled(open_pos, close_pos, color);
                }
                ImPlot::EndItem();
            }

            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::PushStyleColor(ImPlotCol_Fill, colorRange);
            ImPlot::PlotShaded("Projected balance range", m_dates.data(), m_soldeMin.data(), m_soldeMax.data(), countDatas);
            ImPlot::PopStyleColor();
            ImPlot::PopStyleVar();
            ImPlot::PushStyleColor(ImPlotCol_Fill, colorDebit);
            ImPlot::PlotLine("Projected balance min", m_dates.data(), m_soldeMin.data(), countDatas);
            ImPlot::PopStyleColor();
            ImPlot::PushStyleColor(ImPlotCol_Fill, colorCredit);
            ImPlot::PlotLine("Projected balance max", m_dates.data(), m_soldeMax.data(), countDatas);
            ImPlot::PopStyleColor();
        }
        ImPlot::EndPlot();
    }
}
