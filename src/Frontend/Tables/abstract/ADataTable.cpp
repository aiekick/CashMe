#include <Frontend/Tables/abstract/ADataTable.h>
#include <Systems/SettingsDialog.h>
#include <Models/DataBase.h>
#include <Headers/DatasDef.h>

ADataTable::ADataTable(const char* vTableName, const int32_t& vColummCount)  //
    : m_TableName(vTableName), m_ColummCount(vColummCount) {}

bool ADataTable::init() {
    return true;
}

void ADataTable::unit() {}

bool ADataTable::load() {
    refreshDatas();
    return true;
}

void ADataTable::unload() {}

bool ADataTable::drawMenu() {
    bool ret = false;
    if (ImGui::BeginMenuBar()) {
        ret |= m_drawAccountMenu();
        ret |= m_drawMenu();
#ifdef _DEBUG
        ret |= m_drawDebugMenu();
#endif
        ImGui::EndMenuBar();
    }
    return ret;
}

void ADataTable::refreshDatas() {}

bool ADataTable::m_drawMenu() {
    return false;
}

bool ADataTable::m_drawDebugMenu() {
    return false;
}

void ADataTable::m_updateDatas(const RowID& /*vAccountID*/) {
    refreshDatas();
}

void ADataTable::m_draw(const ImVec2& vSize) {
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

        if (ImGui::IsWindowHovered()) {
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
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}

bool ADataTable::m_drawAccountMenu() {
    bool needRefresh = false;
    if (ImGui::BeginMenu("Accounts")) {
        for (const auto& bank : m_Accounts) {
            if (ImGui::BeginMenu(bank.first.c_str())) {  // bank name
                for (const auto& agency : bank.second) {
                    if (ImGui::BeginMenu(agency.first.c_str())) {  // bank agency
                        static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                        if (ImGui::BeginTable("##MenuAccounts", 3, flags)) {
                            ImGui::TableSetupScrollFreeze(0, 1);
                            ImGui::TableSetupColumn("Number", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableHeadersRow();
                            size_t idx = 0U;
                            for (const auto& number : agency.second) {
                                const auto& a = number.second;
                                ImGui::TableNextRow();
                                ImGui::PushID(a.id);
                                ImGui::TableNextColumn();
                                ImGui::PushID(&a);
                                if (ImGui::Selectable(a.datas.number.c_str(), m_accountID == a.id, ImGuiSelectableFlags_SpanAllColumns)) {
                                    m_ResetSelection();
                                    m_accountID = a.id;
                                    m_currentAccount = a;
                                    needRefresh = true;
                                }
                                ImGui::PopID();
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", a.datas.name.c_str());
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", a.datas.type.c_str());
                                ImGui::PopID();
                                ++idx;
                            }
                            ImGui::EndTable();
                        }
                        ImGui::EndMenu();
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenu();
    }
    if (needRefresh) {
        m_updateDatas(m_accountID);
    }
    return needRefresh;
}

void ADataTable::draw(const ImVec2& vSize) {
    drawMenu();
    m_draw(vSize);
}

void ADataTable::m_updateAccounts() {
    m_Accounts.clear();
    DataBase::ref().GetAccounts(                       //
        [this](const AccountOutput& vAccountOutput) {  //
            m_Accounts[vAccountOutput.bankName + "##BankName"][vAccountOutput.datas.bank_agency + "##BankAgency"][vAccountOutput.datas.number] = vAccountOutput;
            if (m_accountID == 0) {
                m_accountID = vAccountOutput.id;
            }
            if (vAccountOutput.id == m_accountID) {
                m_currentAccount = vAccountOutput;
            }
        });
}

void ADataTable::m_drawContextMenuContent() {}

void ADataTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {}

double ADataTable::m_computeMaxPrice() {
    return DBL_MIN;
}

void ADataTable::m_showContextMenu(const size_t& vIdx) {
    if (!m_selectedItems.empty()) {
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

int32_t ADataTable::m_getColumnCount() const {
    return m_ColummCount;
}

RowID ADataTable::m_getAccountID() const {
    return m_accountID;
}

void ADataTable::m_drawColumnSelectable(const size_t& vIdx, const RowID& vRowID, const std::string& vText) {
    ImGui::TableNextColumn();
    ImGui::PushID(vRowID);
    auto is_selected = m_isRowSelected(vRowID);
    ImGui::Selectable(vText.c_str(), &is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
    if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            m_ResetSelection();
            m_selectRow(vRowID);
            m_doActionOnDblClick(vIdx, vRowID);
        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
                m_CurrSelectedItemIdx = vIdx;
            } else {
                m_LastSelectedItemIdx = vIdx;
                if (!ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
                    m_ResetSelection();
                }
                m_selectOrDeselectRow(vRowID);
            }
        }
    }
    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%s", vText.c_str());
    ImGui::PopID();
    m_showContextMenu(vIdx);
}

bool ADataTable::m_drawColumnCheckbox(bool& vValue, const bool vVisibility) {
    bool ret = false;
    ImGui::TableNextColumn();
    if (vVisibility) {
        ImGui::PushID(&vValue);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ret = ImGui::Checkbox("##check", &vValue);
        ImGui::PopStyleVar();
        ImGui::PopID();
    } else {
        ImGui::Text("%s", "");
    }
    
    return ret;
}

void ADataTable::m_drawColumnText(const std::string& vText) {
    ImGui::TableNextColumn();
    ImGui::Text("%s", vText.c_str());
    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%s", vText.c_str());
}

void ADataTable::m_selectRow(const RowID& vRowID) {
    m_selectedItems.emplace(vRowID);  // selection
}

void ADataTable::m_selectOrDeselectRow(const RowID& vRowID) {
    if (m_selectedItems.find(vRowID) != m_selectedItems.end()) {
        m_selectedItems.erase(vRowID);  // deselection
    } else {
        m_selectedItems.emplace(vRowID);  // selection
    }
}

void ADataTable::m_ResetSelection() {
    m_selectedItems.clear();
}

bool ADataTable::m_isRowSelected(const RowID& vRowID) const {
    return (m_selectedItems.find(vRowID) != m_selectedItems.end());
}

const ImGuiListClipper& ADataTable::m_GetListClipper() const {
    return m_ListClipper;
}

float ADataTable::m_GetTextHeight() const {
    return m_TextHeight;
}

const AccountOutput& ADataTable::m_getAccount() const {
    return m_currentAccount;
}

AccountOutput ADataTable::m_getAccount(const BankName& vBankName, const BankAgency& vBankAgency, const AccountNumber& vAccountNumber) const {
    std::map<BankName, std::map<BankAgency, std::map<AccountNumber, AccountOutput>>> m_Accounts;
    if (m_Accounts.find(vBankName) != m_Accounts.end()) {
        const auto acc0 = m_Accounts.at(vBankName);
        if (acc0.find(vBankAgency) != acc0.end()) {
            const auto acc1 = acc0.at(vBankAgency);
            if (acc1.find(vAccountNumber) != acc1.end()) {
                return acc1.at(vAccountNumber);
            }
        }
    }
    return {};
}

void ADataTable::m_drawColumnDebit(const double& vDebit) {
    ImGui::TableNextColumn();
    if (vDebit < 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::BadColor);
        ImGui::Text("%.2f", vDebit);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%.2f", vDebit);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%s", "");
    }
}

void ADataTable::m_drawColumnCredit(const double& vCredit) {
    ImGui::TableNextColumn();
    if (vCredit > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::GoodColor);
        ImGui::Text("%.2f", vCredit);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%.2f", vCredit);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%s", "");
    }
}

void ADataTable::m_drawColumnAmount(const double& vAmount) {
    ImGui::TableNextColumn();
    m_drawAmount(vAmount);
}

void ADataTable::m_drawAmount(const double& vAmount) {
    if (vAmount < 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::BadColor);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::CustomStyle::GoodColor);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%.2f", vAmount);
    }
}

void ADataTable::m_drawColumnInt(const int32_t& vValue) {
    ImGui::TableNextColumn();
    ImGui::Text("%i", vValue);
    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%i", vValue);
}

const std::set<RowID>& ADataTable::m_getSelectedRows() {
    return m_selectedItems;
}

void ADataTable::m_selectRows(const size_t& vStartIdx, const size_t& vEndIdx) {
    for (size_t idx = vStartIdx; idx < vEndIdx; ++idx) {
        m_selectRow(m_getItemRowID(idx));
    }
}