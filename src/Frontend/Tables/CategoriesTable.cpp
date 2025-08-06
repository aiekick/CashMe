#include <Frontend/Tables/CategoriesTable.h>
#include <Models/DataBase.h>
#include <Frontend/MainFrontend.h>

CategoriesTable::CategoriesTable() : ADataBarsTable("CategoriesTable", 6) {
}

bool CategoriesTable::m_drawMenu() {
    bool ret = false;
    if (ImGui::MenuItem("Refresh")) {
        refreshDatas();
        ret = true;
    }
    return ret;
}

void CategoriesTable::refreshDatas() {
    m_Categories.clear();
    m_updateAccounts();
    DataBase::ref().GetCategoriesStats(                  //
        m_getAccountID(),                                //
        [this](const CategoryOutput& vCategoryOutput) {  //
            m_Categories.push_back(vCategoryOutput);
        });
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
    return m_Categories.at(vIdx).amounts.amount;
}

void CategoriesTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Categories.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.datas.name);
    m_drawColumnDebit(e.amounts.debit);
    m_drawColumnCredit(e.amounts.credit);
    m_drawColumnAmount(e.amounts.amount);
    m_drawColumnBars(e.amounts.amount, vMaxAmount, 100.0f);
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
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update")) {
            std::vector<CategoryOutput> entities_to_update;
            for (const auto& e : m_Categories) {
                if (m_isRowSelected(e.id)) {
                    entities_to_update.push_back(e);
                }
            }
            MainFrontend::ref().getCategoryDialogRef().setCategory(entities_to_update.front());
            MainFrontend::ref().getCategoryDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
        }
    }
}

void CategoriesTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    auto category = m_Categories.at(vIdx);
    category.id = vRowID;
    MainFrontend::ref().getCategoryDialogRef().setCategory(category);
    MainFrontend::ref().getCategoryDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
}
