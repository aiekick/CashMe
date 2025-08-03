#include <Frontend/Tables/EntitiesTable.h>
#include <Models/DataBase.h>

EntitiesTable::EntitiesTable() : ADataBarsTable("EntitiesTable", 6) {
}

bool EntitiesTable::load() {
    ADataBarsTable::load();
    m_updateEntities();
    return true;
}

void EntitiesTable::unload() {
    ADataBarsTable::unload();
}

bool EntitiesTable::drawMenu() {
    if (m_drawAccountMenu()) {
        m_updateEntities();
        return true;
    }
    return false;
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
    return m_Entities.at(vIdx).amount;
}

void EntitiesTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Entities.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.name);
    m_drawColumnDebit(e.debit);
    m_drawColumnCredit(e.credit);
    m_drawColumnAmount(e.amount);
    m_drawColumnBars(e.amount, vMaxAmount, 100.0f);
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
            std::vector<Entity> entities_to_update;
            for (const auto& e : m_Entities) {
                if (m_IsRowSelected(e.id)) {
                    entities_to_update.push_back(e);
                }
            }
            if (entities_to_update.size() > 1U) {
                m_EntityDialog.setEntitiesToMerge(entities_to_update);
                m_EntityDialog.show(DataDialogMode::MODE_UPDATE_ALL);
            } else if (entities_to_update.size() == 1U) {
                m_EntityDialog.setEntity(entities_to_update.front());
                m_EntityDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
            }
        }
        if (m_getSelectedRows().size() > 1U) {
            if (ImGui::MenuItem("Merge")) {
                std::vector<Entity> entities_to_merge;
                for (const auto& e : m_Entities) {
                    if (m_IsRowSelected(e.id)) {
                        entities_to_merge.push_back(e);
                    }
                }
                m_EntityDialog.setEntitiesToMerge(entities_to_merge);
                m_EntityDialog.show(DataDialogMode::MODE_MERGE_ALL);
            }
        }
        if (ImGui::MenuItem("Delete")) {
            std::vector<Entity> entities_to_delete;
            for (const auto& e : m_Entities) {
                if (m_IsRowSelected(e.id)) {
                    entities_to_delete.push_back(e);
                }
            }
            m_EntityDialog.setEntitiesToDelete(entities_to_delete);
            m_EntityDialog.show(DataDialogMode::MODE_DELETE_ALL);
        }
    }
}

void EntitiesTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    EZ_TOOLS_DEBUG_BREAK;
}

void EntitiesTable::m_updateEntities() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        m_Entities.clear();
        DataBase::Instance()->GetEntitiesStats(  //
            account_id,
            [this](
                const RowID& vRowID,
                const EntityName& vEntityName,
                const TransactionDebit& vTransactionDebit,
                const TransactionCredit& vTransactionCredit,
                const TransactionsCount& vTransactionCount) {  //
                Entity e;
                e.id = vRowID;
                e.name = vEntityName;
                e.debit = vTransactionDebit;
                e.credit = vTransactionCredit;
                e.amount = vTransactionDebit + vTransactionCredit;
                e.count = vTransactionCount;
                m_Entities.push_back(e);
            });
    }
}
