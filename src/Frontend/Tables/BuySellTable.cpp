#include <Frontend/Tables/BuySellTable.h>
#include <Models/DataBase.h>
#include <algorithm>

BuySellTable::BuySellTable() : ADataTable("BuySellTable", 3) {}

void BuySellTable::clear() {
    m_items.clear();
    m_barsGraph.clear();
    m_pieGraph.clear();
}

void BuySellTable::refreshDatas() {
    m_updateAccounts();
    m_items.clear();
    DataBase::ref().GetBuySellStats(  //
        m_getAccountID(),
        m_field,
        m_filterInput.GetText(),
        m_useDayRange,
        m_dayStart,
        m_dayEnd,
        [this](const BuySellStatItem& vItem) {  //
            m_items.push_back(vItem);
        });
    m_barsGraph.prepare(m_items);
    m_pieGraph.prepare(m_items, m_monthsWindow);
    m_applySort();
}

ez::xml::Nodes BuySellTable::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    node.addChild("field").setContent(static_cast<int32_t>(m_field));
    node.addChild("filter").setContent(m_filterInput.GetText());
    node.addChild("use_day_range").setContent(m_useDayRange);
    node.addChild("day_start").setContent(m_dayStart);
    node.addChild("day_end").setContent(m_dayEnd);
    node.addChild("pie_months").setContent(m_monthsWindow);
    return node.getChildren();
}

bool BuySellTable::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    const auto& strName = vNode.getName();
    if (strName == "field") {
        m_field = static_cast<SearchColumns>(vNode.getContent<int32_t>());
    } else if (strName == "filter") {
        m_filterInput.SetText(vNode.getContent());
    } else if (strName == "use_day_range") {
        m_useDayRange = vNode.getContent<bool>();
    } else if (strName == "day_start") {
        m_dayStart = vNode.getContent<int32_t>();
    } else if (strName == "day_end") {
        m_dayEnd = vNode.getContent<int32_t>();
    } else if (strName == "pie_months") {
        m_monthsWindow = vNode.getContent<int32_t>();
    }
    return false;
}

bool BuySellTable::m_drawControls() {
    bool changed = false;
    static const std::vector<std::string> sFieldLabels = {"Description", "Comment", "Entity", "Category", "Operation"};
    static const SearchColumns sFieldColumns[] = {
        SEARCH_COLUMN_DESCRIPTION, SEARCH_COLUMN_COMMENT, SEARCH_COLUMN_ENTITY, SEARCH_COLUMN_CATEGORY, SEARCH_COLUMN_OPERATION};
    const int32_t fieldCount = static_cast<int32_t>(sFieldLabels.size());
    int currentField = 3;  // default : Category
    for (int32_t idx = 0; idx < fieldCount; ++idx) {
        if (sFieldColumns[idx] == m_field) {
            currentField = idx;
            break;
        }
    }
    if (ImGui::ContrastedComboVectorDefault(150.0f, "Field", &currentField, sFieldLabels, 3)) {
        if (currentField >= 0 && currentField < fieldCount) {
            m_field = sFieldColumns[currentField];
            changed = true;
        }
    }
    ImGui::SameLine();
    if (m_filterInput.DisplayInputText(200.0f, "Filter", "", false)) {
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::CheckBoxBoolDefault("Day range", &m_useDayRange, false)) {
        changed = true;
    }
    if (m_useDayRange) {
        ImGui::SameLine();
        if (ImGui::InputIntDefault(90.0f, "Start", &m_dayStart, 1)) {
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::InputIntDefault(90.0f, "End", &m_dayEnd, 31)) {
            changed = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::InputIntDefault(90.0f, "Pie months", &m_monthsWindow, 6)) {
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::ContrastedButton("Update")) {
        changed = true;
    }
    // keep the parameters in a valid range
    if (m_dayStart < 1) {
        m_dayStart = 1;
    }
    if (m_dayEnd > 31) {
        m_dayEnd = 31;
    }
    if (m_dayEnd < m_dayStart) {
        m_dayEnd = m_dayStart;
    }
    if (m_monthsWindow < 1) {
        m_monthsWindow = 1;
    }
    return changed;
}

void BuySellTable::m_draw(const ImVec2& vSize) {
    if (m_drawControls()) {
        refreshDatas();
    }
    static float tableWidth = 350.0f;
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    float graphWidth = avail.x - tableWidth;
    if (graphWidth < 50.0f) {
        graphWidth = 50.0f;
    }
    if (ImGui::BeginChild("##buysellGraphs", ImVec2(graphWidth, 0.0f))) {
        if (ImGui::BeginTabBar("##buysellTabs")) {
            if (ImGui::BeginTabItem("Bars")) {
                m_barsGraph.draw(ImGui::GetContentRegionAvail());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Pie")) {
                m_pieGraph.draw(ImGui::GetContentRegionAvail());
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ADataTable::m_draw(ImVec2(tableWidth, -1.0f));
}

size_t BuySellTable::m_getItemsCount() const {
    return m_items.size();
}

RowID BuySellTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_items.size()) {
        return static_cast<RowID>(vIdx + 1);  // synthetic id, only used to drive the row selection
    }
    return 0;
}

void BuySellTable::m_drawTableContent(const size_t& vIdx, const double& /*vMaxAmount*/) {
    const auto& item = m_items.at(vIdx);
    const ImVec4 groupColor = m_barsGraph.getGroupColor(item.group);
    ImGui::PushStyleColor(ImGuiCol_Text, groupColor);
    m_drawColumnSelectable(vIdx, static_cast<RowID>(vIdx + 1), item.month);
    m_drawColumnText(item.group);
    ImGui::PopStyleColor();
    m_drawColumnAmount(item.amount);
}

void BuySellTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Month", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

ImGuiTableFlags BuySellTable::m_getTableFlags() const {
    return ADataTable::m_getTableFlags() | ImGuiTableFlags_Sortable;
}

void BuySellTable::m_sortDatas(ImGuiTableSortSpecs* vSortSpecs) {
    if (vSortSpecs == nullptr || vSortSpecs->SpecsCount == 0) {
        m_sortColumn = -1;
        return;
    }
    const ImGuiTableColumnSortSpecs& spec = vSortSpecs->Specs[0];
    m_sortColumn = spec.ColumnIndex;
    m_sortAscending = (spec.SortDirection == ImGuiSortDirection_Ascending);
    m_applySort();
    m_ResetSelection();
}

void BuySellTable::m_applySort() {
    if (m_sortColumn < 0) {
        return;
    }
    const int32_t column = m_sortColumn;
    const bool ascending = m_sortAscending;
    std::sort(m_items.begin(), m_items.end(), [column, ascending](const BuySellStatItem& vA, const BuySellStatItem& vB) {
        int32_t cmp = 0;
        switch (column) {
            case 0: cmp = vA.month.compare(vB.month); break;   // Month
            case 1: cmp = vA.group.compare(vB.group); break;   // Value (field)
            case 2: cmp = (vA.amount < vB.amount) ? -1 : ((vA.amount > vB.amount) ? 1 : 0); break;  // Amount
            default: break;
        }
        return ascending ? (cmp < 0) : (cmp > 0);
    });
}
