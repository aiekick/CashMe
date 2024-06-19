#include <Frontend/Tables/TransactionsTable.h>
#include <Models/DataBase.h>

TransactionsTable::TransactionsTable() : ADataTable("TransactionsTable", 5) {
}

TransactionsTable::~TransactionsTable() {
}

bool TransactionsTable::load() {
    ADataTable::load();
    m_updateTransactions();
    return true;
}

void TransactionsTable::unload() {
    ADataTable::unload();
}

bool TransactionsTable::drawMenu() {
    if (ADataTable::drawMenu()) {
        m_updateTransactions();
        return true;
    }
    return false;
}

size_t TransactionsTable::m_getItemsCount() const {
    return m_Transactions.size();
}

RowID TransactionsTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Transactions.size()) {
        return m_Transactions.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double TransactionsTable::m_getItemAmount(const size_t& vIdx) const {
    return m_Transactions.at(vIdx).amount;
}

void TransactionsTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& t = m_Transactions.at(vIdx);
    m_drawColumnSelectable(vIdx, t.id, t.description);
    m_drawColumnDebit(t.debit);
    m_drawColumnCredit(t.credit);
    m_drawColumnAmount(t.amount);
    m_drawColumnBars(t.amount, vMaxAmount);
}

void TransactionsTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void TransactionsTable::m_drawContextMenuContent() {
    CTOOL_DEBUG_BREAK;
}

void TransactionsTable::m_doActionOnDblClick() {
    CTOOL_DEBUG_BREAK;
}

void TransactionsTable::m_updateTransactions() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        m_Transactions.clear();
        CTOOL_DEBUG_BREAK;
    }
}
