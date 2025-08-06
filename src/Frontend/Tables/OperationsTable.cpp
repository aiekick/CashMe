#include <Frontend/Tables/OperationsTable.h>
#include <Models/DataBase.h>
#include <Frontend/MainFrontend.h>

OperationsTable::OperationsTable() : ADataBarsTable("OperationsTable", 6) {}

bool OperationsTable::m_drawMenu() {
    bool ret = false;
    if (ImGui::MenuItem("Refresh")) {
        refreshDatas();
        ret = true;
    }
    return ret;
}

void OperationsTable::refreshDatas() {
    m_Operations.clear();
    m_updateAccounts();
    DataBase::ref().GetOperationsStats(                    //
        m_getAccountID(),                                  //
        [this](const OperationOutput& vOperationOutput) {  //
            m_Operations.push_back(vOperationOutput);
        });
}

size_t OperationsTable::m_getItemsCount() const {
    return m_Operations.size();
}

RowID OperationsTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Operations.size()) {
        return m_Operations.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double OperationsTable::m_getItemBarAmount(const size_t& vIdx) const {
    return m_Operations.at(vIdx).amounts.amount;
}

void OperationsTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Operations.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.datas.name);
    m_drawColumnDebit(e.amounts.debit);
    m_drawColumnCredit(e.amounts.credit);
    m_drawColumnAmount(e.amounts.amount);
    m_drawColumnBars(e.amounts.amount, vMaxAmount, 100.0f);
    m_drawColumnInt(e.count);
}

void OperationsTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

/* only one at a time */
void OperationsTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update")) {
            size_t idx = 0;
            for (const auto& e : m_Operations) {
                if (m_isRowSelected(e.id)) {
                    m_doActionOnDblClick(idx, e.id);
                    break;
                }
                ++idx;
            }
        }
    }
}

void OperationsTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    auto operation = m_Operations.at(vIdx);
    operation.id = vRowID;
    MainFrontend::ref().getOperationDialogRef().setOperation(operation);
    MainFrontend::ref().getOperationDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
}
