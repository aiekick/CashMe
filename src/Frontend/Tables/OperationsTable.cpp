#include <Frontend/Tables/OperationsTable.h>
#include <Models/DataBase.h>

OperationsTable::OperationsTable() : ADataTable("OperationsTable", 5) {
}

OperationsTable::~OperationsTable() {
}

bool OperationsTable::load() {
    ADataTable::load();
    m_updateOperations();
    return true;
}

void OperationsTable::unload() {
    ADataTable::unload();
}

bool OperationsTable::drawMenu() {
    if (ADataTable::drawMenu()) {
        m_updateOperations();
        return true;
    }
    return false;
}

double OperationsTable::m_getAmount(const size_t& vIdx) {
    return m_Operations.at(vIdx).amount;
}

void OperationsTable::m_drawContent(const size_t& vIdx, const double& vMaxAmount) {
    const auto& e = m_Operations.at(vIdx);
    ImGui::TableNextColumn();
    ImGui::Text("%s", e.name.c_str()); 
    m_drawColumnDebit(e.debit);
    m_drawColumnCredit(e.credit);
    m_drawColumnAmount(e.amount);
    m_drawColumnBars(e.amount, vMaxAmount);
}

size_t OperationsTable::m_getItemsCount() {
    return m_Operations.size();
}

void OperationsTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

void OperationsTable::m_updateOperations() {
    const auto account_id = m_getAccountID();
    if (account_id > 0) {
        m_Operations.clear();
        DataBase::Instance()->GetOperationsStats(  //
            account_id,
            [this](const OperationName& vOperationName,
                   const TransactionDebit& vTransactionDebit,
                   const TransactionCredit& vTransactionCredit) {  //
                Operation e;
                e.name = vOperationName;
                e.debit = vTransactionDebit;
                e.credit = vTransactionCredit;
                e.amount = vTransactionDebit + vTransactionCredit;
                m_Operations.push_back(e);
            });
    }
}
