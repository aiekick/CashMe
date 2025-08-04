#include <Frontend/Tables/AccountsTable.h>
#include <Models/DataBase.h>

AccountsTable::AccountsTable() : ADataBarsTable("AccountsTable", 11) {}

bool AccountsTable::load() {
    m_updateAccounts();
    return true;
}

void AccountsTable::unload() {
    ADataBarsTable::unload();
}

bool AccountsTable::drawMenu() {
    return false;
}

AccountDialog& AccountsTable::getAccountDialogRef() {
    return m_AccountDialog;
}

size_t AccountsTable::m_getItemsCount() const {
    return m_Accounts.size();
}

RowID AccountsTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Accounts.size()) {
        return m_Accounts.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double AccountsTable::m_getItemBarAmount(const size_t& vIdx) const {
    return m_Accounts.at(vIdx).amounts.amount;
}

void AccountsTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Accounts.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.bankName);
    m_drawColumnText(e.datas.bank_agency);
    m_drawColumnText(e.datas.number);
    m_drawColumnText(e.datas.type);
    m_drawColumnText(e.datas.name);
    m_drawColumnAmount(e.datas.base_solde);
    m_drawColumnDebit(e.amounts.debit);
    m_drawColumnCredit(e.amounts.credit);
    m_drawColumnAmount(e.amounts.amount);
    m_drawColumnBars(e.amounts.amount, vMaxAmount, 100.0f);
    m_drawColumnInt(e.count);
}

void AccountsTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Bank", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Agency", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Account", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("base solde", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void AccountsTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update selection")) {
            AccountOutput account_to_update;
            for (const auto& acc : m_Accounts) {
                if (m_IsRowSelected(acc.id)) {
                    account_to_update = acc;
                }
            }
            m_AccountDialog.setAccount(account_to_update);
            m_AccountDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
        }
    }
}

void AccountsTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    m_AccountDialog.setAccount(m_Accounts.at(vIdx));
    m_AccountDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
}

void AccountsTable::m_updateAccounts() {
    m_Accounts.clear();
    DataBase::ref().GetAccountsStats(  //
        [this](const AccountOutput& vAccountOutput) {  //
            m_Accounts.push_back(vAccountOutput);
        });
}
