#include <Frontend/Tables/BudgetTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>

BudgetTable::BudgetTable() : ADataTable("BudgetTable", 9) {}

bool BudgetTable::init() {
    return true;
}

void BudgetTable::unit() {
    clear();
}

bool BudgetTable::load() {
    if (ADataTable::load()) {
        refreshDatas();
        return true;
    }
    return false;
}

void BudgetTable::unload() {
    ADataTable::unload();
    clear();
}

bool BudgetTable::drawMenu() {
    if (ImGui::MenuItem("Refresh")) {
        refreshDatas();
    }
    if (m_drawAccountMenu()) {
        m_UpdateBudget();
        return true;
    }
    drawDebugMenu();
    return false;
}

void BudgetTable::clear() {
    m_budgets.clear();
}

void BudgetTable::refreshDatas() {
    m_updateAccounts();
}

void BudgetTable::drawDebugMenu(FrameActionSystem& vFrameActionSystem) {
#ifdef _DEBUG
    if (ImGui::BeginMenu("Debug")) {
        ImGui::EndMenu();
    }
#endif
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
    ImGui::TableSetupScrollFreeze(0, 2);
    ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void BudgetTable::m_drawContextMenuContent() {

}

void BudgetTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {

}

void BudgetTable::m_UpdateBudget() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        clear();
        DataBase::ref().ComputeBudget(                  //
            account_id,                                      //
            190,
            [this](const BudgetOutput& vBudgetOutput) {  //
                m_budgets.push_back(vBudgetOutput);
            });
    }
}

