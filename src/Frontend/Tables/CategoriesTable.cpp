#include <Frontend/Tables/CategoriesTable.h>
#include <Models/DataBase.h>

CategoriesTable::CategoriesTable() : ADataBarsTable("CategoriesTable", 6) {
}

bool CategoriesTable::load() {
    ADataBarsTable::load();
    m_updateCategories();
    return true;
}

void CategoriesTable::unload() {
    ADataBarsTable::unload();
}

bool CategoriesTable::drawMenu() {
    if (m_drawAccountMenu()) {
        m_updateCategories();
        return true;
    }
    return false;
}

size_t CategoriesTable::m_getItemsCount() const {
    return m_Categories.size();
}

RowID CategoriesTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Categories.size()) {
        return m_Categories.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double CategoriesTable::m_getItemBarAmount(const size_t& vIdx) const {
    return m_Categories.at(vIdx).amount;
}

void CategoriesTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Categories.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.name);
    m_drawColumnDebit(e.debit);
    m_drawColumnCredit(e.credit);
    m_drawColumnAmount(e.amount);
    m_drawColumnBars(e.amount, vMaxAmount, 100.0f);
    m_drawColumnInt(e.count);
}

void CategoriesTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void CategoriesTable::m_drawContextMenuContent() {
    EZ_TOOLS_DEBUG_BREAK;
}

void CategoriesTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    EZ_TOOLS_DEBUG_BREAK;
}

void CategoriesTable::m_updateCategories() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        m_Categories.clear();
        DataBase::Instance()->GetCategoriesStats(  //
            account_id,
            [this](
                const RowID& vRowID,
                const CategoryName& vCategoryName,
                const TransactionDebit& vTransactionDebit,
                const TransactionCredit& vTransactionCredit,
                const TransactionsCount& vTransactionCount) {  //
                Category e;
                e.id = vRowID;
                e.name = vCategoryName;
                e.debit = vTransactionDebit;
                e.credit = vTransactionCredit;
                e.amount = vTransactionDebit + vTransactionCredit;
                e.count = vTransactionCount;
                m_Categories.push_back(e);
            });
    }
}
