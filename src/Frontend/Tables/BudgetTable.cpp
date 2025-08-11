#include <Frontend/Tables/BudgetTable.h>
#include <Systems/SettingsDialog.h>
#include <Frontend/MainFrontend.h>
#include <Models/BudgetComputer.h>
#include <Models/DataBase.h>
#include <ezlibs/ezDate.hpp>
#include <ezlibs/ezTime.hpp>
#include <ezlibs/ezStr.hpp>
#include <vector>
#include <map>

BudgetTable::BudgetTable() : ADataTable("BudgetTable", 5) {}

void BudgetTable::clear() {
}

void BudgetTable::refreshDatas() {
    m_updateAccounts();
    if (m_budgetComputer.compute(m_getAccountID(), std::make_pair(m_params.pastMonthOffset, m_params.futurMonthOffset), m_params.useOptional)) {
        BudgetGraph::BudgetGraphInput bgi;
        bgi.budgetDatas = m_budgetComputer.getBudgetDatas();
        bgi.monthTransactionsDatesCount = m_budgetComputer.getCountMonthTransactionsDates();
        bgi.lastMonthBalance = m_budgetComputer.getLastMonthBalance();
        m_budgetGraph.prepare(bgi);
    }
}

ez::xml::Nodes BudgetTable::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    node.addChild("past_month_offset").setContent(m_params.pastMonthOffset);
    node.addChild("futur_month_offset").setContent(m_params.futurMonthOffset);
    node.addChild("use_optional_incomes").setContent(m_params.useOptional);
    return node.getChildren();
}

bool BudgetTable::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    const auto& strName = vNode.getName();
    if (strName == "past_month_offset") {
        m_params.pastMonthOffset = vNode.getContent<int32_t>();
    } else if (strName == "futur_month_offset") {
        m_params.futurMonthOffset = vNode.getContent<uint32_t>();
    } else if (strName == "use_optional_incomes") {
        m_params.useOptional = vNode.getContent<bool>();
    }
    return false;
}

bool BudgetTable::m_drawMenu() {
    bool ret = false;
    ret |= ImGui::MenuItem("Refresh");
    if (ImGui::BeginMenu("Settings")) {
        ImGui::MenuItem("Show/Hide Table", nullptr, &m_showTable);
        ImGui::MenuItem("Show/Hide Graph", nullptr, &m_showGraph);
        static Params params; 
        ret |= ImGui::MenuItem("Use/Unuse Optionals", nullptr, &m_params.useOptional);
        ret |= ImGui::InputIntDefault(150.0f, "Past month offset", &m_params.pastMonthOffset, params.pastMonthOffset);
        ret |= ImGui::InputUIntDefault(150.0f, "Futur month offset", &m_params.futurMonthOffset, params.futurMonthOffset);
        ImGui::EndMenu();
    }

    if (ret) {
        refreshDatas();
    }

    return ret;
}

void BudgetTable::m_draw(const ImVec2& vSize) {
    static float tableWidth = 350.0f;
    if (m_showGraph) {
        auto size = vSize;
        if (m_showTable) {
            size = ImVec2(0, -1);
        }
        m_budgetGraph.draw(ImVec2(vSize.x - tableWidth, -1));
    }
    if (m_showTable) {
        ImGui::SameLine();
        ADataTable::m_draw(ImVec2(tableWidth, -1));
    }
}

size_t BudgetTable::m_getItemsCount() const {
    return m_budgetComputer.getBudgetDatas().size();
}

RowID BudgetTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_budgetComputer.getBudgetDatas().size()) {
        return m_budgetComputer.getBudgetDatas().at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

void BudgetTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_budgetComputer.getBudgetDatas().at(vIdx);
    m_drawColumnSelectable(vIdx, t.id, t.date);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Day:   %s", t.date.c_str());
        ImGui::Text("Balance min: %.2f", t.solde.min);
        ImGui::Text("Balane max: %.2f", t.solde.max);
        if (t.delta.min != 0.0) {
            ImGui::Text("Incomes min: %.2f \n\t%s", t.delta.min, t.incomesMin.c_str());
        }
        if (t.delta.max != 0.0) {
            ImGui::Text("Incomes max: %.2f \n\t%s", t.delta.max, t.incomesMax.c_str());
        }
        ImGui::EndTooltip();
    }
    m_drawColumnAmount(t.delta.min);
    m_drawColumnAmount(t.delta.max);
    m_drawColumnAmount(t.solde.min);
    m_drawColumnAmount(t.solde.max);
  //  m_drawColumnText(t.incomesMin);
  //  m_drawColumnText(t.incomesMinAmount);
  //  m_drawColumnText(t.incomesMax);
  //  m_drawColumnText(t.incomesMaxAmount);
}

void BudgetTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde max", ImGuiTableColumnFlags_WidthFixed);
    //ImGui::TableSetupColumn("Income Min", ImGuiTableColumnFlags_WidthFixed);
   // ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
   // ImGui::TableSetupColumn("Income Max", ImGuiTableColumnFlags_WidthFixed);
   // ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}


inline bool isMatching(const std::string& vStr, const std::string& vWildcardedPattern) {
    std::string res;
    auto patterns = ez::str::splitStringToVector(vWildcardedPattern, '*', false);
    size_t startPos = std::string::npos;
    size_t endPos = 0U;
    for (const std::string& pattern : patterns) {
        auto start = vStr.find(pattern, endPos);
        if (start != std::string::npos) {
            if (startPos == std::string::npos) {
                startPos = start;
            }
            endPos = start + pattern.size();
        } else {
            startPos = std::string::npos;
            endPos = std::string::npos;
            break;
        }
    }
    return (startPos != std::string::npos) && (endPos != std::string::npos);
}
