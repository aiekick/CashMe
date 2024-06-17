#include <Frontend/Tables/abstract/ADataTable.h>
#include <Systems/SettingsDialog.h>
#include <Models/DataBase.h>

ADataTable::ADataTable(const char* vTableName, const int32_t& vColummCount) 
    : m_TableName(vTableName), m_ColummCount(vColummCount) {
}

bool ADataTable::load() {
    m_updateAccounts();
    return true;
}

void ADataTable::unload() {

}

bool ADataTable::drawMenu() {
    const auto align = 100.0f;
    const auto width = 10;
    return m_AccountsCombo.displayCombo(width, "Account", align);
}

void ADataTable::draw(const ImVec2& vSize) {
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 30.0f);
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable(m_TableName, m_ColummCount, flags, vSize)) { 
        m_setupColumns();
        int32_t idx = 0;
        double max_amount = DBL_MIN;
        m_TextHeight = ImGui::GetTextLineHeight();
        const float& item_h = ImGui::GetTextLineHeightWithSpacing();
        m_ListClipper.Begin((int32_t)m_getItemsCount(), item_h);
        while (m_ListClipper.Step()) {
            max_amount = m_computeMaxPrice();
            for (idx = m_ListClipper.DisplayStart; idx < m_ListClipper.DisplayEnd; ++idx) {
                if (idx < 0) {
                    continue;
                }
                m_drawContent((size_t)idx, max_amount);
            }
        }
        m_ListClipper.End();
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}

void ADataTable::m_updateAccounts() {
    m_Accounts.clear();
    m_AccountsCombo.clear();
    DataBase::Instance()->GetAccounts(  //
        [this](const RowID& vRowID,
               const BankName& vBankName,
               const BankAgency& vBankAgency,
               const AccountType& vAccountType,
               const AccountName& vAccountName,
               const AccountNumber& vAccountNumber,
               const AccounBaseSolde& vBaseSolde,
               const TransactionsCount& vCount) {  //
            Account a;
            a.id = vRowID;
            a.bank = vBankName;
            a.agency = vBankAgency;
            a.type = vAccountType;
            a.name = vAccountName;
            a.number = vAccountNumber;
            a.base_solde = vBaseSolde;
            a.count = vCount;
            m_Accounts.push_back(a);
            m_AccountsCombo.getArrayRef().push_back(vAccountNumber);
        });
    m_AccountsCombo.getIndexRef() = 0;
}

double ADataTable::m_computeMaxPrice() {
    double max_price = 0.0;
    for (int32_t idx = m_ListClipper.DisplayStart; idx < m_ListClipper.DisplayEnd; ++idx) {
        if (idx < 0) {
            continue;
        }
        const auto& as = std::abs(m_getAmount(idx));
        if (as > max_price) {
            max_price = as;
        }
    }
    return max_price;
}

RowID ADataTable::m_getAccountID() {
    if (m_AccountsCombo.getIndex() < m_Accounts.size()) {
        return m_Accounts.at(m_AccountsCombo.getIndex()).id;
    }
    return 0U;
}

void ADataTable::m_drawColumnDebit(const double& vDebit) {
    ImGui::TableNextColumn();
    if (vDebit < 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_BadColor);
        ImGui::Text("%.2f", vDebit);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vDebit);
        ImGui::PopStyleColor();
    }
}

void ADataTable::m_drawColumnCredit(const double& vCredit) {
    ImGui::TableNextColumn();
    if (vCredit > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_GoodColor);
        ImGui::Text("%.2f", vCredit);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vCredit);
        ImGui::PopStyleColor();
    }
}

void ADataTable::m_drawColumnAmount(const double& vAmount) {
    ImGui::TableNextColumn();
    if (vAmount < 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_BadColor);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, m_GoodColor);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
    }
}

void ADataTable::m_drawColumnBars(const double vAmount, const double vMaxAmount) {
    ImGui::TableNextColumn();
    auto drawListPtr = ImGui::GetWindowDrawList();
    const auto& cursor = ImGui::GetCursorScreenPos();
    ImGuiContext& g = *GImGui;
    ImGuiTable* table = g.CurrentTable;
    const auto& table_column = table->Columns[table->CurrentColumn];
    const auto column_width = table_column.MaxX - table_column.MinX;
    const ImVec2 pMin(cursor.x, cursor.y + m_TextHeight * 0.1f);
    const ImVec2 pMax(cursor.x + column_width, cursor.y + m_TextHeight * 0.9f);
    const float pMidX((pMin.x + pMax.x) * 0.5f);
    ImGui::SetCursorScreenPos(pMin);
    const float bw(column_width * 0.5f * std::abs((float)vAmount) / (float)vMaxAmount);
    if (vAmount < 0.0) {
        drawListPtr->AddRectFilled(ImVec2(pMidX - bw, pMin.y), ImVec2(pMidX, pMax.y), ImGui::GetColorU32(m_BadColor));
    } else if (vAmount > 0.0) {
        drawListPtr->AddRectFilled(ImVec2(pMidX, pMin.y), ImVec2(pMidX + bw, pMax.y), ImGui::GetColorU32(m_GoodColor));
    }
    ImGui::SetCursorScreenPos(pMax);
}
