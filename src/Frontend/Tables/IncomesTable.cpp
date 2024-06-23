#include <Frontend/Tables/IncomesTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>

IncomesTable::IncomesTable() : ADataTable("IncomesTable", 10) {
}

bool IncomesTable::init() {
    return m_IncomeDialog.init();
}

void IncomesTable::unit() {
    m_IncomeDialog.unit();
    clear();
}

bool IncomesTable::load() {
    if (ADataTable::load()) {
        refreshDatas();
        return true;
    }
    return false;
}

void IncomesTable::unload() {
    ADataTable::unload();
    clear();
}

bool IncomesTable::drawMenu() {
    return false;
}

IncomeDialog& IncomesTable::getIncomeDialogRef() {
    return m_IncomeDialog;
}

size_t IncomesTable::m_getItemsCount() const {
    return m_Incomes.size();
}

RowID IncomesTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Incomes.size()) {
        return m_Incomes.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

void IncomesTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_Incomes.at(vIdx);
    m_drawColumnSelectable(vIdx, t.id, t.name);
    m_drawColumnText(t.startDate);
    m_drawColumnText(t.endDate);
    m_drawColumnText(t.entity);
    m_drawColumnText(t.category);
    m_drawColumnText(t.operation);
    m_drawColumnAmount(t.minAmount);
    m_drawColumnAmount(t.maxAmount);
    m_drawColumnInt(t.minDays);
    m_drawColumnInt(t.maxDays);
}

void IncomesTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 2);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("StartDate", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("EndDate", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("MinDays", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("MaxDays", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void IncomesTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update")) {
            std::vector<Income> incomes_to_update;
            for (const auto& trans : m_Incomes) {
                if (m_IsRowSelected(trans.id)) {
                    incomes_to_update.push_back(trans);
                }
            }
            if (incomes_to_update.size() > 1U) {
                m_IncomeDialog.setIncomesToUpdate(incomes_to_update);
                m_IncomeDialog.show(DataDialogMode::MODE_UPDATE_ALL);
            } else if (incomes_to_update.size() == 1U) {
                m_IncomeDialog.setIncome(incomes_to_update.front());
                m_IncomeDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
            }
        }
        if (m_getSelectedRows().size() > 1U) {
            if (ImGui::MenuItem("Merge")) {
                std::vector<Income> incomes_to_update;
                for (const auto& trans : m_Incomes) {
                    if (m_IsRowSelected(trans.id)) {
                        incomes_to_update.push_back(trans);
                    }
                }
                m_IncomeDialog.setIncomesToUpdate(incomes_to_update);
                m_IncomeDialog.show(DataDialogMode::MODE_MERGE_ALL);
            }
        }
        if (ImGui::MenuItem("Delete")) {
            std::vector<Income> incomes_to_delete;
            for (const auto& trans : m_Incomes) {
                if (m_IsRowSelected(trans.id)) {
                    incomes_to_delete.push_back(trans);
                }
            }
            m_IncomeDialog.setIncomesToDelete(incomes_to_delete);
            m_IncomeDialog.show(DataDialogMode::MODE_DELETE_ALL);
        }
    }
}

void IncomesTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    m_IncomeDialog.setIncome(m_Incomes.at(vIdx));
    m_IncomeDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
}

void IncomesTable::refreshDatas() {

}

void IncomesTable::drawDebugMenu(FrameActionSystem& vFrameActionSystem) {
#ifdef _DEBUG
    if (ImGui::BeginMenu("Debug")) {
        if (ImGui::MenuItem("Refresh")) {
            refreshDatas();
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Delete Tables")) {
            if (ImGui::MenuItem("Banks")) {
                DataBase::Instance()->DeleteBanks();
                refreshDatas();
            }
            if (ImGui::MenuItem("Accounts")) {
                DataBase::Instance()->DeleteAccounts();
                refreshDatas();
            }
            if (ImGui::MenuItem("Entities")) {
                DataBase::Instance()->DeleteEntities();
                refreshDatas();
            }
            if (ImGui::MenuItem("Categories")) {
                DataBase::Instance()->DeleteCategories();
                refreshDatas();
            }
            if (ImGui::MenuItem("Operations")) {
                DataBase::Instance()->DeleteOperations();
                refreshDatas();
            }
            if (ImGui::MenuItem("Incomes")) {
                CTOOL_DEBUG_BREAK;
                //DataBase::Instance()->DeleteIncomes();
                refreshDatas();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
#endif
}

void IncomesTable::clear() {
    m_Incomes.clear();
}
