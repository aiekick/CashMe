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
    return m_Accounts.at(vIdx).amount;
}

void AccountsTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Accounts.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.bank);
    m_drawColumnText(e.agency);
    m_drawColumnText(e.number);
    m_drawColumnText(e.type);
    m_drawColumnText(e.name);
    m_drawColumnAmount(e.base_solde);
    m_drawColumnDebit(e.debit);
    m_drawColumnCredit(e.credit);
    m_drawColumnAmount(e.amount);
    m_drawColumnBars(e.amount, vMaxAmount, 100.0f);
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
            Account account_to_update;
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
    DataBase::Instance()->GetAccountsStats(  //
        [this](
            const RowID& vRowID,
            const BankName& vBankName,
            const BankAgency& vBankAgency,
            const AccountNumber& vAccountNumber,
            const AccountType& vAccountType,
            const AccountName& vAccountName,
            const AccountBaseSolde& vAccountBaseSolde,
            const TransactionDebit& vTransactionDebit,
            const TransactionCredit& vTransactionCredit,
            const TransactionsCount& vTransactionCount) {  //
            Account e;
            e.id = vRowID;
            e.bank = vBankName;
            e.agency = vBankAgency;
            e.number = vAccountNumber;
            e.type = vAccountType;
            e.name = vAccountName;
            e.base_solde = vAccountBaseSolde;
            e.debit = vTransactionDebit;
            e.credit = vTransactionCredit;
            e.amount = vTransactionDebit + vTransactionCredit;
            e.count = vTransactionCount;
            m_Accounts.push_back(e);
        });
}
