#include <Frontend/Tables/abstract/ADataTable.h>
#include <Systems/SettingsDialog.h>
#include <Models/DataBase.h>

ADataTable::ADataTable(const char* vTableName, const int32_t& vColummCount) : m_TableName(vTableName), m_ColummCount(vColummCount) {
}

bool ADataTable::Init() {
    return true;
}

void ADataTable::Unit() {
}

bool ADataTable::load() {
    m_updateAccounts();
    return true;
}

void ADataTable::unload() {
}

bool ADataTable::m_drawAccountMenu() {
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
                ImGui::TableNextRow();
                m_drawTableContent((size_t)idx, max_amount);
            }
        }
        m_ListClipper.End();

        if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            if (ImGui::IsKeyDown(ImGuiKey_A)) {
                m_selectRows(0, m_getItemsCount());
            }
        }

        // shift selection
        if (ImGui::IsKeyDown(ImGuiMod_Shift) && m_LastSelectedItemIdx > -1 && m_CurrSelectedItemIdx > -1) {
            int32_t min_idx = ImMin(m_LastSelectedItemIdx, m_CurrSelectedItemIdx);
            int32_t max_idx = ImMax(m_LastSelectedItemIdx, m_CurrSelectedItemIdx);
            m_ResetSelection();
            m_selectRows(min_idx, max_idx);
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}

ImWidgets::QuickStringCombo& ADataTable::m_getAccountComboRef() {
    return m_AccountsCombo;
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
        const auto& as = std::abs(m_getItemBarAmount(idx));
        if (as > max_price) {
            max_price = as;
        }
    }
    return max_price;
}

void ADataTable::m_showContextMenu(const size_t& vIdx) {
    if (!m_SelectedItems.empty()) {
        ImGui::PushID(vIdx);
        if (ImGui::BeginPopupContextItem(               //
                NULL,                                   //
                ImGuiPopupFlags_NoOpenOverItems |       //
                    ImGuiPopupFlags_MouseButtonRight |  //
                    ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            m_drawContextMenuContent();
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
}

RowID ADataTable::m_getAccountID() {
    if (m_AccountsCombo.getIndex() < m_Accounts.size()) {
        return m_Accounts.at(m_AccountsCombo.getIndex()).id;
    }
    return 0U;
}

void ADataTable::m_drawColumnSelectable(const size_t& vIdx, const RowID& vRowID, const std::string& vText) {
    ImGui::TableNextColumn();
    ImGui::PushID(vRowID);
    auto is_selected = m_IsRowSelected(vRowID);
    ImGui::Selectable(vText.c_str(), &is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
    if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            m_ResetSelection();
            m_SelectRow(vRowID);
            m_doActionOnDblClick(vIdx, vRowID);
        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
                m_CurrSelectedItemIdx = vIdx;
            } else {
                m_LastSelectedItemIdx = vIdx;
                if (!ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
                    m_ResetSelection();
                }
                m_SelectOrDeselectRow(vRowID);
            }
        } 
    }
    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", vText.c_str());
    ImGui::PopID();
    m_showContextMenu(vIdx);
}

void ADataTable::m_drawColumnText(const std::string& vText) {
    ImGui::TableNextColumn();
    ImGui::Text("%s", vText.c_str());
    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", vText.c_str());
}

void ADataTable::m_SelectRow(const RowID& vRowID) {
    m_SelectedItems.emplace(vRowID);  // selection
}

void ADataTable::m_SelectOrDeselectRow(const RowID& vRowID) {
    if (m_SelectedItems.find(vRowID) != m_SelectedItems.end()) {
        m_SelectedItems.erase(vRowID);  // deselection
    } else {
        m_SelectedItems.emplace(vRowID);  // selection
    }
}

void ADataTable::m_ResetSelection() {
    m_SelectedItems.clear();
}

bool ADataTable::m_IsRowSelected(const RowID& vRowID) const {
    return (m_SelectedItems.find(vRowID) != m_SelectedItems.end());
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

void ADataTable::m_drawColumnBars(const double vAmount, const double vMaxAmount, const float vColumNWidth) {
    ImGui::TableNextColumn();
    auto drawListPtr = ImGui::GetWindowDrawList();
    const auto& cursor = ImGui::GetCursorScreenPos();
    ImGuiContext& g = *GImGui;
    ImGuiTable* table = g.CurrentTable;
    const auto& table_column = table->Columns[table->CurrentColumn];
    auto column_width = vColumNWidth;
    if (column_width < 0.0f) {
        column_width = table_column.MaxX - table_column.MinX;
    }
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

const std::set<RowID>& ADataTable::m_getSelectedRows() {
    return m_SelectedItems;
}

void ADataTable::m_selectRows(const size_t& vStartIdx, const size_t& vEndIdx) {
    for (size_t idx = vStartIdx; idx < vEndIdx; ++idx) {
        m_SelectRow(m_getItemRowID(idx));
    }
}