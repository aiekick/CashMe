#include <Frontend/Graph/BuySellBarsGraph.h>
#include <Frontend/Graph/RainbowColormap.h>
#include <cmath>
#include <ezlibs/ezTools.hpp>

void BuySellBarsGraph::clear() {
    m_groupLabels.clear();
    m_groupLabelPtrs.clear();
    m_groupIndex.clear();
    m_groupColors.clear();
    m_monthLabels.clear();
    m_monthLabelPtrs.clear();
    m_monthPositions.clear();
    m_values.clear();
    m_monthTotals.clear();
    m_groupCount = 0;
    m_monthCount = 0;
    m_colormapId = -1;
}

void BuySellBarsGraph::prepare(const std::vector<BuySellStatItem>& vItems) {
    clear();

    // 1st pass : collect distinct months / groups (values are placeholders).
    // we do NOT assign the final index here because the items can arrive in any order from
    // GetBuySellStats, which would make idx 0 the "first encountered" group instead of the
    // first alphabetical one. that breaks the legend ordering AND the rainbow color sequence.
    std::map<std::string, int32_t> monthIndex;
    for (const auto& item : vItems) {
        monthIndex[item.month] = 0;
        m_groupIndex[item.group] = 0;
    }
    // 2nd pass : assign indices in std::map iteration order (= alphabetical for groups,
    // = chronological for months since the key is "YYYY/MM"). idx 0 is now the first alpha
    // group, idx N-1 the last → legend rows, m_groupColors[i] = rainbow(i, N) and bars'
    // colormap entry i all line up the same way.
    {
        int32_t counter = 0;
        for (auto& pair : monthIndex) {
            pair.second = counter++;
        }
    }
    {
        int32_t counter = 0;
        for (auto& pair : m_groupIndex) {
            pair.second = counter++;
        }
    }

    m_monthCount = static_cast<int32_t>(monthIndex.size());
    m_groupCount = static_cast<int32_t>(m_groupIndex.size());
    if (m_monthCount == 0 || m_groupCount == 0) {
        return;
    }

    // ordered label arrays
    m_monthLabels.resize(m_monthCount);
    m_monthPositions.resize(m_monthCount);
    for (const auto& pair : monthIndex) {
        m_monthLabels[pair.second] = pair.first;
        m_monthPositions[pair.second] = static_cast<double>(pair.second);
    }
    m_groupLabels.resize(m_groupCount);
    for (const auto& pair : m_groupIndex) {
        m_groupLabels[pair.second] = pair.first;
    }

    // c_str arrays, built only once the string arrays are final (no further reallocation)
    m_monthLabelPtrs.resize(m_monthCount);
    for (int32_t idx = 0; idx < m_monthCount; ++idx) {
        m_monthLabelPtrs[idx] = m_monthLabels[idx].c_str();
    }
    m_groupLabelPtrs.resize(m_groupCount);
    for (int32_t idx = 0; idx < m_groupCount; ++idx) {
        m_groupLabelPtrs[idx] = m_groupLabels[idx].c_str();
    }

    // colors driven by ez::getRainBowColor along the serie order (alphabetical via std::map)
    m_groupColors.resize(m_groupCount);
    for (int32_t idx = 0; idx < m_groupCount; ++idx) {
        const ez::fvec4 c = ez::getRainBowColor(idx, m_groupCount);
        m_groupColors[idx] = ImVec4(c.x, c.y, c.z, c.w);
    }
    // resolve the colormap once here (called on refresh) ; draw() will only push it.
    // the helper caches by size, so we never re-register a colormap for an already-seen group count.
    m_colormapId = getOrCreateEzRainbowColormap(m_groupCount);

    // values matrix [serie * monthCount + month] and the per month totals
    m_values.assign(static_cast<size_t>(m_groupCount) * m_monthCount, 0.0);
    m_monthTotals.assign(m_monthCount, 0.0);
    for (const auto& item : vItems) {
        const int32_t serie = m_groupIndex.at(item.group);
        const int32_t month = monthIndex.at(item.month);
        m_values[static_cast<size_t>(serie) * m_monthCount + month] += item.amount;
        m_monthTotals[month] += item.amount;
    }
}

void BuySellBarsGraph::draw(const ImVec2& vSize) {
    if (m_groupCount == 0 || m_monthCount == 0) {
        ImGui::TextUnformatted("No data");
        return;
    }
    // serie i takes ez::getRainBowColor(i, m_groupCount), shared with the list rows via getGroupColor.
    // the colormap was registered once in prepare() so draw() (called every frame) only pushes it.
    ImPlot::PushColormap(m_colormapId);
    if (ImPlot::BeginPlot("##BuySellBars", vSize, ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxes("Month", "Amount", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisTicks(ImAxis_X1, m_monthPositions.data(), m_monthCount, m_monthLabelPtrs.data());
        const double barGroupSize = 0.67;
        ImPlot::PlotBarGroups(m_groupLabelPtrs.data(), m_values.data(), m_groupCount, m_monthCount, barGroupSize, 0.0, ImPlotBarGroupsFlags_Stacked);
        // net total per month, drawn after the bars with an explicit color
        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImPlot::PlotLine("Total", m_monthPositions.data(), m_monthTotals.data(), m_monthCount);
        ImPlot::PopStyleColor();
        // one tooltip per hovered color block
        m_drawHoveredTooltip(barGroupSize);
        ImPlot::EndPlot();
    }
    ImPlot::PopColormap();
}

ImVec4 BuySellBarsGraph::getGroupColor(const std::string& vGroup) const {
    const auto it = m_groupIndex.find(vGroup);
    if (it != m_groupIndex.end() && it->second < static_cast<int32_t>(m_groupColors.size())) {
        return m_groupColors[static_cast<size_t>(it->second)];
    }
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

void BuySellBarsGraph::m_drawHoveredTooltip(const double& vBarGroupSize) const {
    if (!ImPlot::IsPlotHovered()) {
        return;
    }
    const ImPlotPoint mouse = ImPlot::GetPlotMousePos();
    const int32_t month = static_cast<int32_t>(std::lround(mouse.x));
    if (month < 0 || month >= m_monthCount || std::abs(mouse.x - static_cast<double>(month)) > vBarGroupSize * 0.5) {
        return;
    }
    // replicate the ImPlot stacked layout : positives stack upward, negatives downward, series in order
    double pos = 0.0;
    double neg = 0.0;
    int32_t hoveredSerie = -1;
    for (int32_t serie = 0; serie < m_groupCount; ++serie) {
        const double value = m_values[static_cast<size_t>(serie) * m_monthCount + month];
        double lo = 0.0;
        double hi = 0.0;
        if (value > 0.0) {
            lo = pos;
            hi = pos + value;
            pos += value;
        } else {
            hi = neg;
            lo = neg + value;
            neg += value;
        }
        if (value != 0.0 && mouse.y >= lo && mouse.y <= hi) {
            hoveredSerie = serie;
        }
    }
    if (hoveredSerie < 0) {
        return;
    }
    const double amount = m_values[static_cast<size_t>(hoveredSerie) * m_monthCount + month];
    const ImVec4 hoveredColor = (hoveredSerie < static_cast<int32_t>(m_groupColors.size()))  //
        ? m_groupColors[static_cast<size_t>(hoveredSerie)]                                   //
        : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImGui::BeginTooltip();
    ImGui::Text("Month: %s", m_monthLabels[month].c_str());
    ImGui::TextColored(hoveredColor, "%s", m_groupLabels[hoveredSerie].c_str());
    ImGui::Text("Amount: %.2f", amount);
    ImGui::EndTooltip();
}
