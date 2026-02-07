#include <Frontend/Tables/IncomesTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>
#include <Panes/BudgetPane.h>

IncomesTable::IncomesTable() : ADataTable("IncomesTable", 12) {
}

bool IncomesTable::init() {
    return true;
}

void IncomesTable::unit() {
    clear();
}

void IncomesTable::clear() {
    m_Datas.clear();
}

bool IncomesTable::m_drawMenu() {
    bool ret = false;
    if (ImGui::MenuItem("Refresh")) {
        refreshDatas();
        ret = true;
    }
    return ret;
}

void IncomesTable::refreshDatas() {
    m_Datas.clear();
    m_updateAccounts();
    DataBase::ref().GetIncomes(                   //
        m_getAccountID(),                         //
        [this](                                   //
            const IncomeOutput& vIncomeOutput) {  //
            m_Datas.incomes.push_back(vIncomeOutput);
            m_Datas.minAmount += vIncomeOutput.datas.minAmount;
            m_Datas.maxAmount += vIncomeOutput.datas.maxAmount;
        });
}

size_t IncomesTable::m_getItemsCount() const {
    return m_Datas.incomes.size();
}

RowID IncomesTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Datas.incomes.size()) {
        return m_Datas.incomes.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

void IncomesTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_Datas.incomes.at(vIdx);
    m_drawColumnSelectable(vIdx, t.id, t.datas.name);
    m_drawColumnText(t.datas.startDate);
    m_drawColumnText(t.datas.endDate);
    m_drawColumnAmount(t.datas.minAmount);
    m_drawColumnAmount(t.datas.maxAmount);
    m_drawColumnInt(t.datas.minDay);
    m_drawColumnInt(t.datas.maxDay);
    m_drawColumnText(t.datas.entity.name);
    m_drawColumnText(t.datas.category.name);
    m_drawColumnText(t.datas.operation.name);
    m_drawColumnText(t.datas.description);
    if (m_drawColumnCheckbox(t.datas.optional)) {
        DataBase::ref().setIncomeAsOptional(t.id, t.datas.optional);
        BudgetPane::ref()->Init();
    }
}

void IncomesTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 2);

    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Start Date", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("End Date", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Min Day", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Max Day", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Optional", ImGuiTableColumnFlags_WidthFixed);

    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    m_drawAmount(m_Datas.minAmount);
    ImGui::TableNextColumn();
    m_drawAmount(m_Datas.maxAmount);
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();

    ImGui::TableHeadersRow();
}

void IncomesTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update")) {
            std::vector<IncomeOutput> incomes_to_update;
            for (const auto& trans : m_Datas.incomes) {
                if (m_isRowSelected(trans.id)) {
                    incomes_to_update.push_back(trans);
                }
            }
            if (incomes_to_update.size() > 1U) {
                MainFrontend::ref().getIncomeDialogRef().setIncomesToUpdate(incomes_to_update);
                MainFrontend::ref().getIncomeDialogRef().show(DataDialogMode::MODE_UPDATE_ALL);
            } else if (incomes_to_update.size() == 1U) {
                MainFrontend::ref().getIncomeDialogRef().setIncome(incomes_to_update.front());
                MainFrontend::ref().getIncomeDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
            }
        }
        if (ImGui::MenuItem("Duplicate", nullptr, false, m_getSelectedRows().size() == 1U)) {
            std::vector<IncomeOutput> incomes_to_duplicate;
            for (const auto& trans : m_Datas.incomes) {
                if (m_isRowSelected(trans.id)) {
                    incomes_to_duplicate.push_back(trans);
                }
            }
            if (incomes_to_duplicate.size() == 1U) {
                auto income_copy = incomes_to_duplicate.front();
                income_copy.id = 0;
                MainFrontend::ref().getIncomeDialogRef().setIncome(income_copy);
                MainFrontend::ref().getIncomeDialogRef().show(DataDialogMode::MODE_CREATION);
            }
        }
        if (ImGui::MenuItem("Delete")) {
            std::vector<IncomeOutput> incomes_to_delete;
            for (const auto& trans : m_Datas.incomes) {
                if (m_isRowSelected(trans.id)) {
                    incomes_to_delete.push_back(trans);
                }
            }
            MainFrontend::ref().getIncomeDialogRef().setIncomesToDelete(incomes_to_delete);
            MainFrontend::ref().getIncomeDialogRef().show(DataDialogMode::MODE_DELETE_ALL);
        }
    }
}

void IncomesTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    auto income = m_Datas.incomes.at(vIdx);
    income.id = vRowID;
    MainFrontend::ref().getIncomeDialogRef().setIncome(income);
    MainFrontend::ref().getIncomeDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
}
