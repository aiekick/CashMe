#include <Frontend/Tables/BudgetTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>

BudgetTable::BudgetTable() : ADataTable("BudgetTable", 9) {}

bool BudgetTable::m_drawMenu() {
    bool ret = false;
    if (ImGui::MenuItem("Refresh")) {
        refreshDatas();
        ret = true;
    }
    ImGui::MenuItem("Table", nullptr, &m_showTable);
    ImGui::MenuItem("Graph", nullptr, &m_showGraph);
    return ret;
}

void BudgetTable::m_draw(const ImVec2& vSize) {
    if (m_showGraph) {
        auto size = vSize;
        if (m_showTable) {
            size = ImVec2(-1, 0);
        }
        m_budgetGraph.draw(size);
    }
    if (m_showTable) {
        ADataTable::m_draw(vSize);
    }
}

void BudgetTable::clear() {
    m_budgets.clear();
}

void BudgetTable::refreshDatas() {
    m_budgets.clear();
    m_updateAccounts();
    DataBase::ref().ComputeBudget(  //
        m_getAccountID(),           //
        190,
        [this](const BudgetOutput& vBudgetOutput) {  //
            m_budgets.push_back(vBudgetOutput);
        });
    m_budgetGraph.prepare(m_budgets);
}

size_t BudgetTable::m_getItemsCount() const {
    return m_budgets.size();
}

RowID BudgetTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_budgets.size()) {
        return m_budgets.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

void BudgetTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_budgets.at(vIdx);
    m_drawColumnSelectable(vIdx, t.id, t.date);
    m_drawColumnAmount(t.delta.min);
    m_drawColumnAmount(t.delta.max);
    m_drawColumnAmount(t.solde.min);
    m_drawColumnAmount(t.solde.max);
    m_drawColumnText(t.incomesMin);
    m_drawColumnText(t.incomesMinAmount);
    m_drawColumnText(t.incomesMax);
    m_drawColumnText(t.incomesMaxAmount);
}

void BudgetTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}
