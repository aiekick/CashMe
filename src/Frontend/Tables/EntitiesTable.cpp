#include <Frontend/Tables/EntitiesTable.h>
#include <Models/DataBase.h>

EntitiesTable::EntitiesTable() : ADataTable("EntitiesTable", 5) {
}

EntitiesTable::~EntitiesTable() {
}

bool EntitiesTable::load() {
    ADataTable::load();
    m_updateEntities();
    return true;
}

void EntitiesTable::unload() {
    ADataTable::unload();
}

bool EntitiesTable::drawMenu() {
    if (ADataTable::drawMenu()) {
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

double EntitiesTable::m_getItemAmount(const size_t& vIdx) const {
    return m_Entities.at(vIdx).amount;
}

void EntitiesTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Entities.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.name);
    m_drawColumnDebit(e.debit);
    m_drawColumnCredit(e.credit);
    m_drawColumnAmount(e.amount);
    m_drawColumnBars(e.amount, vMaxAmount);
}

void EntitiesTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void EntitiesTable::m_drawContextMenuContent() {
    CTOOL_DEBUG_BREAK;
}

void EntitiesTable::m_doActionOnDblClick() {
    CTOOL_DEBUG_BREAK;
}

void EntitiesTable::m_updateEntities() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        m_Entities.clear();
        DataBase::Instance()->GetEntitiesStats(  //
            account_id,
            [this](const RowID& vRowID,
                   const EntityName& vEntityName,
                   const TransactionDebit& vTransactionDebit,
                   const TransactionCredit& vTransactionCredit) {  //
                Entity e;
                e.id = vRowID;
                e.name = vEntityName;
                e.debit = vTransactionDebit;
                e.credit = vTransactionCredit;
                e.amount = vTransactionDebit + vTransactionCredit;
                m_Entities.push_back(e);
            });
    }
}
