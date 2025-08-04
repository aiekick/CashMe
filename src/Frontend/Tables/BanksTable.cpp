#include <Frontend/Tables/BanksTable.h>
#include <Models/DataBase.h>

BanksTable::BanksTable() : ADataBarsTable("BanksTable", 7) {
}

bool BanksTable::load() {
    m_updateBanks();
    return true;
}

void BanksTable::unload() {
    ADataBarsTable::unload();
}

bool BanksTable::drawMenu() {
    return false;
}

BankDialog& BanksTable::getBankDialogRef() {
    return m_BankDialog;
}

size_t BanksTable::m_getItemsCount() const {
    return m_Banks.size();
}

RowID BanksTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Banks.size()) {
        return m_Banks.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double BanksTable::m_getItemBarAmount(const size_t& vIdx) const {
    return m_Banks.at(vIdx).amounts.amount;
}

void BanksTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Banks.at(vIdx);
    m_drawColumnSelectable(vIdx, e.id, e.datas.name);
    m_drawColumnText(e.datas.url);
    m_drawColumnDebit(e.amounts.debit);
    m_drawColumnCredit(e.amounts.credit);
    m_drawColumnAmount(e.amounts.amount);
    m_drawColumnBars(e.amounts.amount, vMaxAmount, 100.0f);
    m_drawColumnInt(e.count);
}

void BanksTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("url", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void BanksTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update selection")) {
            BankOutput bank_to_update;
            for (const auto& bank : m_Banks) {
                if (m_IsRowSelected(bank.id)) {
                    bank_to_update = bank;
                }
            }
            m_BankDialog.setBank(bank_to_update);
            m_BankDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
        }
    }
}

void BanksTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    m_BankDialog.setBank(m_Banks.at(vIdx));
    m_BankDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
}

void BanksTable::m_updateBanks() {
    m_Banks.clear();
    DataBase::ref().GetBanksStats(         //
        [this](const BankOutput& vBankOutput) {  //
            m_Banks.push_back(vBankOutput);
        });
}
