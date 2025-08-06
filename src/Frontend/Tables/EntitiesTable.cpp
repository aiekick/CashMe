#include <Frontend/Tables/EntitiesTable.h>
#include <Models/DataBase.h>
#include <Frontend/MainFrontend.h>

EntitiesTable::EntitiesTable() : ADataBarsTable("EntitiesTable", 6) {
}

bool EntitiesTable::m_drawMenu() {
    bool ret = false;
    if (ImGui::MenuItem("Refresh")) {
        refreshDatas();
        ret = true;
    }
    return ret;
}

void EntitiesTable::refreshDatas() {
    m_Entities.clear();
    m_updateAccounts();
    DataBase::ref().GetEntitiesStats(             //
        m_getAccountID(),                         //
        [this](                                   //
            const EntityOutput& vEntityOutput) {  //
            m_Entities.push_back(vEntityOutput);
        });
}

size_t EntitiesTable::m_getItemsCount() const {
    return m_Entities.size();
}

RowID EntitiesTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Entities.size()) {
        return m_Entities.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double EntitiesTable::m_getItemBarAmount(const size_t& vIdx) const {
    return m_Entities.at(vIdx).amounts.amount;
}

void EntitiesTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Entities.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.datas.name);
    m_drawColumnDebit(e.amounts.debit);
    m_drawColumnCredit(e.amounts.credit);
    m_drawColumnAmount(e.amounts.amount);
    m_drawColumnBars(e.amounts.amount, vMaxAmount, 100.0f);
    m_drawColumnInt(e.count);
}

void EntitiesTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void EntitiesTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update")) {
            std::vector<EntityOutput> entities_to_update;
            for (const auto& e : m_Entities) {
                if (m_isRowSelected(e.id)) {
                    entities_to_update.push_back(e);
                }
            }
            MainFrontend::ref().getEntityDialogRef().setEntity(entities_to_update.front());
            MainFrontend::ref().getEntityDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
        }
    }
}

void EntitiesTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    auto entity = m_Entities.at(vIdx);
    entity.id = vRowID;
    MainFrontend::ref().getEntityDialogRef().setEntity(entity);
    MainFrontend::ref().getEntityDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
}
