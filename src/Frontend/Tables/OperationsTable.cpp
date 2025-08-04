#include <Frontend/Tables/OperationsTable.h>
#include <Models/DataBase.h>

OperationsTable::OperationsTable() : ADataBarsTable("OperationsTable", 6) {
}

bool OperationsTable::load() {
    ADataBarsTable::load();
    m_updateOperations();
    return true;
}

void OperationsTable::unload() {
    ADataBarsTable::unload();
}

bool OperationsTable::drawMenu() {
    if (m_drawAccountMenu()) {
        m_updateOperations();
        return true;
    }
    return false;
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

void OperationsTable::m_drawContextMenuContent() {
    EZ_TOOLS_DEBUG_BREAK;
}

void OperationsTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    EZ_TOOLS_DEBUG_BREAK;
}

void OperationsTable::m_updateOperations() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        m_Operations.clear();
        DataBase::ref().GetOperationsStats(                    //
            account_id,                                        //
            [this](const OperationOutput& vOperationOutput) {  //
                m_Operations.push_back(vOperationOutput);
            });
    }
}
