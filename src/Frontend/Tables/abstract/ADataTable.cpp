#include <Frontend/Tables/abstract/ADataTable.h>
#include <Systems/SettingsDialog.h>
#include <Models/DataBase.h>

ADataTable::ADataTable(const char* vTableName, const int32_t& vColummCount) : m_TableName(vTableName), m_ColummCount(vColummCount) {
}

bool ADataTable::init() {
    return true;
}

void ADataTable::unit() {
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
    return m_AccountsCombo.displayWithColumn(width, "Account", align);
}

void ADataTable::draw(const ImVec2& vSize) {
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 30.0f);
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable(m_TableName, m_ColummCount, flags, vSize)) {
        m_setupColumns();
        int32_t idx = 0;
        m_TextHeight = ImGui::GetTextLineHeight();
        const float& item_h = ImGui::GetTextLineHeightWithSpacing();
        m_ListClipper.Begin((int32_t)m_getItemsCount(), item_h);
        while (m_ListClipper.Step()) {
            double max_amount = m_computeMaxPrice();
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
    DataBase::Instance()->GetAccounts(                 //
        [this](const AccountOutput& vAccountOutput) {  //
            m_Accounts.push_back(vAccountOutput);
            m_AccountsCombo.getArrayRef().push_back(vAccountOutput.datas.number);
        });
    m_AccountsCombo.getIndexRef() = 0;
}

double ADataTable::m_computeMaxPrice() {
    return DBL_MIN;
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

const ImGuiListClipper& ADataTable::m_GetListClipper() const {
    return m_ListClipper;
}

float ADataTable::m_GetTextHeight() const {
    return m_TextHeight;
}

void ADataTable::m_drawColumnDebit(const double& vDebit) {
    ImGui::TableNextColumn();
    if (vDebit < 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::BadColor);
        ImGui::Text("%.2f", vDebit);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vDebit);
        ImGui::PopStyleColor();
    }
}

void ADataTable::m_drawColumnCredit(const double& vCredit) {
    ImGui::TableNextColumn();
    if (vCredit > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::GoodColor);
        ImGui::Text("%.2f", vCredit);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vCredit);
        ImGui::PopStyleColor();
    }
}

void ADataTable::m_drawColumnAmount(const double& vAmount) {
    ImGui::TableNextColumn();
    if (vAmount < 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::BadColor);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::GoodColor);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
    }
}

void ADataTable::m_drawColumnInt(const int32_t& vValue) {
    ImGui::TableNextColumn();
    {
        ImGui::Text("%i", vValue);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%i", vValue);
    }
}

const std::set<RowID>& ADataTable::m_getSelectedRows() {
    return m_SelectedItems;
}

void ADataTable::m_selectRows(const size_t& vStartIdx, const size_t& vEndIdx) {
    for (size_t idx = vStartIdx; idx < vEndIdx; ++idx) {
        m_SelectRow(m_getItemRowID(idx));
    }
}