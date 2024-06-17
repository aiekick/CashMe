#include <Frontend/Tables/CategoriesTable.h>
#include <Models/DataBase.h>

CategoriesTable::CategoriesTable() : ADataTable("CategoriesTable", 5) {
}

CategoriesTable::~CategoriesTable() {
}

bool CategoriesTable::load() {
    ADataTable::load();
    m_updateCategories();
    return true;
}

void CategoriesTable::unload() {
    ADataTable::unload();
}

bool CategoriesTable::drawMenu() {
    if (ADataTable::drawMenu()) {
        m_updateCategories();
        return true;
    }
    return false;
}

double CategoriesTable::m_getAmount(const size_t& vIdx) {
    return m_Categories.at(vIdx).amount;
}

void CategoriesTable::m_drawContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Categories.at(vIdx);
    ImGui::TableNextColumn();
    ImGui::Text("%s", e.name.c_str()); 
    m_drawColumnDebit(e.debit);
    m_drawColumnCredit(e.credit);
    m_drawColumnAmount(e.amount);
    m_drawColumnBars(e.amount, vMaxAmount);
}

size_t CategoriesTable::m_getItemsCount() {
    return m_Categories.size();
}

void CategoriesTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void CategoriesTable::m_updateCategories() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        m_Categories.clear();
        DataBase::Instance()->GetCategoriesStats(  //
            account_id,
            [this](const CategoryName& vCategoryName,
                   const TransactionDebit& vTransactionDebit,
                   const TransactionCredit& vTransactionCredit) {  //
                Category e;
                e.name = vCategoryName;
                e.debit = vTransactionDebit;
                e.credit = vTransactionCredit;
                e.amount = vTransactionDebit + vTransactionCredit;
                m_Categories.push_back(e);
            });
    }
}
