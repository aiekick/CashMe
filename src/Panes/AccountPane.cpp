// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Account Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/AccountPane.h>

#include <cinttypes>  // printf zu

#include <Models/DataBase.h>
#include <Project/ProjectFile.h>
#include <Frontend/MainFrontend.h>
#include <Plugins/PluginManager.h>
#include <Panes/DebitCreditPane.h>
#include <Systems/SettingsDialog.h>

AccountPane::AccountPane() = default;
AccountPane::~AccountPane() {
    Unit();
}

bool AccountPane::Init() {
    bool ret = true;
    m_GetAvailableDataBrokers();
    ret &= m_BankDialog.init();
    ret &= m_AccountDialog.init();
    ret &= m_CategoryDialog.init();
    ret &= m_OperationDialog.init();
    ret &= m_TransactionDialog.init();
    return true;
}

void AccountPane::Unit() {
    m_BankDialog.init();
    m_AccountDialog.init();
    m_CategoryDialog.init();
    m_OperationDialog.init();
    m_TransactionDialog.unit();
    m_Clear();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool AccountPane::DrawPanes(const uint32_t& /*vCurrentFrame*/, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    if (vOpened != nullptr && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
#endif
            if (ProjectFile::Instance()->IsProjectLoaded()) {
                m_drawMenu(MainFrontend::Instance()->GetActionSystemRef());
                m_displayTransactions();
            }
        }

        ImGui::End();
    }
    return change;
}

bool AccountPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool AccountPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& vRect, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    const ImVec2 center = vRect.GetCenter();

    bool ret = false;

    ret |= m_BankDialog.draw(center);
    if (m_AccountDialog.draw(center)) {
        DebitCreditPane::Instance()->Load();
        ret = true;
    }
    ret |= m_CategoryDialog.draw(center);
    ret |= m_OperationDialog.draw(center);
    ret |= m_TransactionDialog.draw(center);
    
    m_ImportThread.drawDialog(center);

    if (ret) {
        m_refreshDatas();
    }

    ImVec2 max = vRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::Instance()->Display("Import Datas", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            const auto& selection = ImGuiFileDialog::Instance()->GetSelection();
            if (!selection.empty()) {
                std::vector<std::string> files;
                for (const auto& s : selection) {
                    files.push_back(s.second);
                }
                m_ImportFromFiles(files);
                ret = true;
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    return ret;
}

bool AccountPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void AccountPane::DoBackend() {
    m_ImportThread.finishIfNeeded();
}

void AccountPane::Load() {
    m_refreshDatas();
}

void AccountPane::m_refreshDatas() {
    m_UpdateBanks();
    m_UpdateEntities();
    m_UpdateCategories();
    m_UpdateOperations();
    m_UpdateAccounts();
}

void AccountPane::m_drawMenu(FrameActionSystem& vFrameActionSystem) {
    if (ProjectFile::Instance()->IsProjectLoaded()) {
        if (ImGui::BeginMenuBar()) {
            m_drawAccountsMenu(vFrameActionSystem);
            m_drawCreationMenu(vFrameActionSystem);
            m_drawImportMenu(vFrameActionSystem);
            m_drawSelectMenu(vFrameActionSystem);
#ifdef _DEBUG
            m_drawDebugMenu(vFrameActionSystem);
#endif
            ImGui::EndMenuBar();
        }
    }
}

void AccountPane::m_displayTransactions() {
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 30.0f);
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##Transactions", 11, flags)) {
        ImGui::TableSetupScrollFreeze(0, 2);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Comments", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Solde", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
        m_drawSearchRow();
        ImGui::TableHeadersRow();
        int32_t idx = 0;
        double max_price = DBL_MIN;
        const float& bar_column_width = 100.0f;
        auto drawListPtr = ImGui::GetWindowDrawList();
        const float& text_h = ImGui::GetTextLineHeight();
        const float& item_h = ImGui::GetTextLineHeightWithSpacing();
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        m_CurrSelectedItemIdx = -1;
        m_TransactionsListClipper.Begin((int)m_Datas.transactions_filtered.size(), item_h);
        while (m_TransactionsListClipper.Step()) {
            max_price = 0.0;
            for (idx = m_TransactionsListClipper.DisplayStart; idx < m_TransactionsListClipper.DisplayEnd; ++idx) {
                if (idx < 0) {
                    continue;
                }
                const auto& t = m_Datas.transactions_filtered.at(idx);
                const auto& as = std::abs(t.solde);
                if (as > max_price) {
                    max_price = as;
                }
            }

            for (idx = m_TransactionsListClipper.DisplayStart; idx < m_TransactionsListClipper.DisplayEnd; ++idx) {
                if (idx < 0) {
                    continue;
                }

                auto& t = m_Datas.transactions_filtered.at(idx);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                {
                    ImGui::PushID(t.id);
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    if (ImGui::Checkbox("##check", &t.confirmed)) {
                        DataBase::Instance()->ConfirmTransaction(t.id, t.confirmed);
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopID();
                }

                ImGui::TableNextColumn();
                { ImGui::Text(t.date.c_str()); }

                ImGui::TableNextColumn();
                {
                    ImGui::PushID(&t);
                    auto is_selected = m_IsRowSelected(t.id);
                    ImGui::Selectable(t.description.c_str(), &is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
                    if (ImGui::IsItemHovered()) {
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            m_ResetSelection();
                            m_SelectOrDeselectRow(t);
                            m_TransactionDialog.setTransaction(t);
                            m_TransactionDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
                        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                            if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
                                m_CurrSelectedItemIdx = idx;
                            } else {
                                m_LastSelectedItemIdx = idx;

                                if (!ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
                                    m_ResetSelection();
                                }

                                m_SelectOrDeselectRow(t);
                            }
                        }
                    }
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.description.c_str());
                    ImGui::PopID();
                    m_drawTransactionMenu(t);
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.comment.c_str());
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.comment.c_str());
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.entity.c_str());
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.entity.c_str());
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.category.c_str());
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.category.c_str());
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.operation.c_str());
                    ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.operation.c_str());
                }

                ImGui::TableNextColumn();
                {
                    if (t.amount < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.amount);
                        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.amount);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.amount >= 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.amount);
                        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.amount);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.solde < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.solde);
                        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.solde);
                        ImGui::PopStyleColor();
                    } else if (t.solde > 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.solde);
                        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.solde);
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::Text("%.2f", t.solde);
                        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", t.solde);
                    }
                }

                ImGui::TableNextColumn();
                {
                    const auto& cursor = ImGui::GetCursorScreenPos();
                    const ImVec2 pMin(cursor.x, cursor.y + text_h * 0.1f);
                    const ImVec2 pMax(cursor.x + bar_column_width, cursor.y + text_h * 0.9f);
                    const float pMidX((pMin.x + pMax.x) * 0.5f);
                    ImGui::SetCursorScreenPos(pMin);
                    const float bw(bar_column_width * 0.5f * std::abs(t.solde) / (float)max_price);
                    if (t.solde < 0.0) {
                        drawListPtr->AddRectFilled(ImVec2(pMidX - bw, pMin.y), ImVec2(pMidX, pMax.y), bad_color);
                    } else if (t.solde > 0.0) {
                        drawListPtr->AddRectFilled(ImVec2(pMidX, pMin.y), ImVec2(pMidX + bw, pMax.y), good_color);
                    }
                    ImGui::SetCursorScreenPos(pMax);
                }
            }
        }
        m_TransactionsListClipper.End();

        if (ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            if (ImGui::IsKeyDown(ImGuiKey_A)) {
                m_SelectCurrentRows();
            }
        }

        // shift selection
        if (ImGui::IsKeyDown(ImGuiMod_Shift) && m_LastSelectedItemIdx > -1 && m_CurrSelectedItemIdx > -1) {
            int32_t min_idx = ImMin(m_LastSelectedItemIdx, m_CurrSelectedItemIdx);
            int32_t max_idx = ImMax(m_LastSelectedItemIdx, m_CurrSelectedItemIdx);
            m_ResetSelection();
            for (int32_t nid = min_idx; nid <= max_idx; ++nid) {
                if (nid < m_Datas.transactions_filtered.size()) {
                    const auto& t = m_Datas.transactions_filtered.at(nid);
                    m_SelectOrDeselectRow(t);
                }
            }
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}

void AccountPane::m_drawImportMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Import")) {
        for (const auto& broker : m_DataBrokerModules) {
            if (ImGui::BeginMenu(broker.first.c_str())) {
                for (const auto& way : broker.second) {
                    if (ImGui::MenuItem(way.first.c_str())) {
                        if (way.second != nullptr) {
                            m_SelectedBroker = way.second;
                            vFrameActionSystem.Clear();
                            vFrameActionSystem.Add([&way]() {
                                const auto& ext = way.second->getFileExt();
                                IGFD::FileDialogConfig config;
                                config.countSelectionMax = 0;
                                config.flags = ImGuiFileDialogFlags_Modal;
                                ImGuiFileDialog::Instance()->OpenDialog("Import Datas", "Import Datas from File", ext.c_str(), config);
                                return true;
                            });
                        }
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenu();
    }
}

void AccountPane::m_drawSelectMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Select")) {
        if (ImGui::MenuItem("Current rows")) {
            m_SelectCurrentRows();
        }
        if (ImGui::MenuItem("UnConfirmed entries")) {
            m_SelectUnConfirmedTransactions();
        }
        if (ImGui::MenuItem("Possible Duplicate entries on Prices and Dates")) {
            m_SelectPossibleDuplicateEntryOnPricesAndDates();
        }
        if (!m_SelectedTransactions.empty()) {
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Selection")) {
                m_ResetSelection();
            }
        }
        ImGui::EndMenu();
    }
}

void AccountPane::m_drawDebugMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Debug")) {
        if (ImGui::MenuItem("Refresh")) {
            m_refreshDatas();
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Delete Tables")) {
            if (ImGui::MenuItem("Banks")) {
                DataBase::Instance()->DeleteBanks();
                m_refreshDatas();
            }
            if (ImGui::MenuItem("Accounts")) {
                DataBase::Instance()->DeleteAccounts();
                m_refreshDatas();
            }
            if (ImGui::MenuItem("Entities")) {
                DataBase::Instance()->DeleteEntities();
                m_refreshDatas();
            }
            if (ImGui::MenuItem("Categories")) {
                DataBase::Instance()->DeleteCategories();
                m_refreshDatas();
            }
            if (ImGui::MenuItem("Operations")) {
                DataBase::Instance()->DeleteOperations();
                m_refreshDatas();
            }
            if (ImGui::MenuItem("Transactions")) {
                DataBase::Instance()->DeleteTransactions();
                m_refreshDatas();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
}

void AccountPane::m_drawAccountsMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Accounts")) {
        for (const auto& bank : m_Accounts) {
            if (ImGui::BeginMenu(bank.first.c_str())) {  // bank name
                for (const auto& agency : bank.second) {
                    if (ImGui::BeginMenu(agency.first.c_str())) {  // bank agency
                        static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                        if (ImGui::BeginTable("##MenuAccounts", 4, flags)) {
                            ImGui::TableSetupScrollFreeze(0, 1);
                            ImGui::TableSetupColumn("Number", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableHeadersRow();
                            size_t idx = 0U;
                            for (const auto& number : agency.second) {
                                const auto& a = number.second;
                                ImGui::TableNextRow();

                                ImGui::PushID(a.id);
                                {
                                    ImGui::TableNextColumn();
                                    ImGui::PushID(&a);
                                    {
                                        if (ImGui::Selectable(a.number.c_str(), m_SelectedAccountIdx == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                                            m_ResetSelection();
                                            m_UpdateTransactions(a.id);
                                            m_SelectedAccountIdx = idx;
                                        }
                                    }
                                    ImGui::PopID();
                                    m_drawAccountMenu(a);

                                    ImGui::TableNextColumn();
                                    ImGui::Text(a.name.c_str());

                                    ImGui::TableNextColumn();
                                    ImGui::Text(a.type.c_str());

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%u", a.count);
                                }
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
}

void AccountPane::m_drawCreationMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("Bank")) {
            m_BankDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Account")) {
            m_AccountDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Entity")) {
            m_EntityDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Category")) {
            m_CategoryDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Operation")) {
            m_OperationDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Transaction")) {
            m_TransactionDialog.show(DataDialogMode::MODE_CREATION);
        }
        ImGui::EndMenu();
    }
}

std::string AccountPane::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);
    std::string res;
    res += vOffset + "<account_pane>\n";
    // we save all modules not just the one is selected
    /*for (const auto& mod : m_DataBrokerModules) {
        auto ptr = dynamic_cast<Cash::IXmlSettings*>(mod.second.get());
        if (ptr != nullptr) {
            if (vUserDatas == "app") {
                res += ptr->GetXmlSettings(vOffset + "\t", Cash::ISettingsType::APP);
            } else if (vUserDatas == "project") {
                res += ptr->GetXmlSettings(vOffset + "\t", Cash::ISettingsType::PROJECT);
            } else {
                CTOOL_DEBUG_BREAK;  // ERROR
            }
        }
    }*/
    res += vOffset + "</account_pane>\n";
    return res;
}

bool AccountPane::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    // we load all modules not just the one is selected
    /*for (const auto& mod : m_DataBrokerModules) {
        auto ptr = dynamic_cast<Cash::IXmlSettings*>(mod.second.get());
        if (ptr != nullptr) {
            if (vUserDatas == "app") {
                ptr->SetXmlSettings(strName, strParentName, strValue, Cash::ISettingsType::APP);
                RecursParsingConfigChilds(vElem, vUserDatas);
            } else if (vUserDatas == "project") {
                ptr->SetXmlSettings(strName, strParentName, strValue, Cash::ISettingsType::PROJECT);
                RecursParsingConfigChilds(vElem, vUserDatas);
            } else {
                CTOOL_DEBUG_BREAK;  // ERROR
            }
        }
    }*/

    return true;
}

void AccountPane::m_Clear() {
    m_SelectedBroker.reset();  // must be reset before quit since point on the memroy of a plugin
    m_DataBrokerModules.clear();
    m_Datas.clear();
}

void AccountPane::m_GetAvailableDataBrokers() {
    m_Clear();
    auto modules = PluginManager::Instance()->GetPluginModulesInfos();
    for (const auto& mod : modules) {
        if (mod.type == Cash::PluginModuleType::DATA_BROKER) {
            auto ptr = std::dynamic_pointer_cast<Cash::BankStatementImportModule>(PluginManager::Instance()->CreatePluginModule(mod.label));
            if (ptr != nullptr) {
                m_DataBrokerModules[mod.path][mod.label] = ptr;
            }
        }
    }
}

void AccountPane::m_ImportFromFiles(const std::vector<std::string>& vFiles) {
    m_ImportThread.start(  //
        "Import Datas",
        m_SelectedBroker,
        vFiles,
        [this]() {
            m_refreshDatas();
            m_refreshFiltering();
        },
        nullptr);
}

void AccountPane::m_ResetFiltering() {
    m_Datas.transactions_filtered = m_Datas.transactions;
    m_Datas.transactions_filtered_rowids = {};
    m_SearchInputTexts = {};
    m_SearchTokens = {};
    m_refreshFiltering();
}

void AccountPane::m_refreshFiltering() {
    m_Datas.transactions_filtered.clear();
    m_Datas.transactions_filtered_rowids.clear();
    bool use = false;
    double solde = m_CurrentBaseSolde;
    m_TotalDebit = 0.0;
    m_TotalCredit = 0.0;
    for (auto tr : m_Datas.transactions) {
        use = true;
        for (size_t idx = 0; idx < SearchColumns::SEARCH_COLUMN_Count; ++idx) {
            const auto& tk = m_SearchTokens.at(idx);
            if (!tk.empty()) {
                use &= (tr.optimized.at(idx).find(tk) != std::string::npos);
            }
        }
        if (use) {
            solde += tr.amount;
            tr.solde = solde;
            m_Datas.transactions_filtered.push_back(tr);
            m_Datas.transactions_filtered_rowids.emplace(tr.id);
            if (tr.amount < 0.0) {
                m_TotalDebit += tr.amount;
            }
            if (tr.amount > 0.0) {
                m_TotalCredit += tr.amount;
            }
        }
    }
}

void AccountPane::m_SelectOrDeselectRow(const Transaction& vTransaction) {
    if (m_SelectedTransactions.find(vTransaction.id) != m_SelectedTransactions.end()) {
        m_SelectedTransactions.erase(vTransaction.id);  // deselection
    } else {
        m_SelectedTransactions.emplace(vTransaction.id);  // selection
    }
}

bool AccountPane::m_IsRowSelected(const RowID& vRowID) const {
    return (m_SelectedTransactions.find(vRowID) != m_SelectedTransactions.end());
}

void AccountPane::m_ResetSelection() {
    m_SelectedTransactions.clear();
}

void AccountPane::m_SelectCurrentRows() {
    m_SelectedTransactions = m_Datas.transactions_filtered_rowids;
}

void AccountPane::m_SelectPossibleDuplicateEntryOnPricesAndDates() {
    if (m_SelectedAccountIdx < m_Datas.accounts.size()) {
        RowID account_id = m_Datas.accounts.at(m_SelectedAccountIdx).id;
        m_ResetSelection();
        DataBase::Instance()->GetDuplicateTransactionsOnDatesAndAmount(  //
            account_id,                                                  //
            [this](const RowID& vRowID) {
                if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                    m_Datas.transactions_filtered_rowids.end()) {
                    m_SelectedTransactions.emplace(vRowID);  // select row id
                }
            });
    }
}

void AccountPane::m_SelectUnConfirmedTransactions() {
    if (m_SelectedAccountIdx < m_Datas.accounts.size()) {
        RowID account_id = m_Datas.accounts.at(m_SelectedAccountIdx).id;
        m_ResetSelection();
        DataBase::Instance()->GetUnConfirmedTransactions(  //
            account_id,                                    //
            [this](const RowID& vRowID) {
                if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                    m_Datas.transactions_filtered_rowids.end()) {
                    m_SelectedTransactions.emplace(vRowID);  // select row id
                }
            });
    }
}

void AccountPane::m_UpdateBanks() {
    m_Datas.bankNames.clear();
    DataBase::Instance()->GetBanks(                                       //
        [this](const BankName& vUserName, const std::string& /*vUrl*/) {  //
            m_Datas.bankNames.push_back(vUserName);
        });
}

void AccountPane::m_UpdateEntities() {
    m_Datas.entityNames.clear();
    DataBase::Instance()->GetEntities(           //
        [this](const EntityName& vEntityName) {  //
            m_Datas.entityNames.push_back(vEntityName);
        });
}

void AccountPane::m_UpdateCategories() {
    m_Datas.categoryNames.clear();
    DataBase::Instance()->GetCategories(             //
        [this](const CategoryName& vCategoryName) {  //
            m_Datas.categoryNames.push_back(vCategoryName);
        });
}

void AccountPane::m_UpdateOperations() {
    m_Datas.operationNames.clear();
    DataBase::Instance()->GetOperations(               //
        [this](const OperationName& vOperationName) {  //
            m_Datas.operationNames.push_back(vOperationName);
        });
}

void AccountPane::m_UpdateAccounts() {
    m_Accounts.clear();
    m_Datas.accounts.clear();
    m_Datas.accountNumbers.clear();
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
            m_Datas.accounts.push_back(a);
            m_Datas.accountNumbers.push_back(vAccountNumber);
            m_Accounts[vBankName + "##BankName"][vBankAgency + "##BankAgency"][vAccountNumber] = a;
        });
    if (m_SelectedAccountIdx < m_Datas.accounts.size()) {
        m_UpdateTransactions(m_Datas.accounts.at(m_SelectedAccountIdx).id);
    }
}

void AccountPane::m_drawAccountMenu(const Account& vAccount) {
    ImGui::PushID(vAccount.id);
    if (ImGui::BeginPopupContextItem(               //
            NULL,                                   //
            ImGuiPopupFlags_NoOpenOverItems |       //
                ImGuiPopupFlags_MouseButtonRight |  //
                ImGuiPopupFlags_NoOpenOverExistingPopup)) {
        if (ImGui::MenuItem("Update")) {
            m_AccountDialog.setAccount(vAccount);
            m_AccountDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
        }
        if (ImGui::MenuItem("Delete")) {
            m_AccountDialog.setAccount(vAccount);
            m_AccountDialog.show(DataDialogMode::MODE_DELETE_ONCE);
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void AccountPane::m_UpdateTransactions(const RowID& vAccountID) {
    m_Datas.transactions.clear();
    const auto& zero_based_account_id = vAccountID - 1;
    if (zero_based_account_id < m_Datas.accounts.size()) {
        double solde = m_CurrentBaseSolde = m_Datas.accounts.at(zero_based_account_id).base_solde;
        const auto& account_number = m_Datas.accounts.at(zero_based_account_id).number;
        DataBase::Instance()->GetTransactions(  //
            vAccountID,                         //
            [this, &solde, account_number](     //
                const RowID& vTransactionID,
                const EntityName& vEntityName,
                const OperationName& vOperationName,
                const CategoryName& vCategoryName,
                const SourceName& vSourceName,
                const TransactionDate& vTransactionDate,
                const TransactionDescription& vTransactionDescription,
                const TransactionComment& vTransactionComment,
                const TransactionAmount& vTransactionAmount,
                const TransactionConfirmed& vConfirmed,
                const TransactionHash& vHash) {  //
                solde += vTransactionAmount;
                Transaction t;
                t.id = vTransactionID;
                t.account = account_number;
                t.date = vTransactionDate;
                t.optimized[0] = ct::toLower(vTransactionDate);
                t.description = vTransactionDescription;
                t.optimized[1] = ct::toLower(vTransactionDescription);
                t.comment = vTransactionComment;
                t.optimized[2] = ct::toLower(vTransactionComment);
                t.entity = vEntityName;
                t.optimized[3] = ct::toLower(vEntityName);
                t.category = vCategoryName;
                t.optimized[4] = ct::toLower(vCategoryName);
                t.operation = vOperationName;
                t.optimized[5] = ct::toLower(vOperationName);
                t.hash = vHash;
                t.source = vSourceName;
                t.amount = vTransactionAmount;
                t.confirmed = vConfirmed;
                t.solde = solde;
                m_Datas.transactions.push_back(t);
            });
    }
    m_refreshFiltering();
}

void AccountPane::m_drawTransactionMenu(const Transaction& vTransaction) {
    if (!m_SelectedTransactions.empty()) {
        const auto* ptr = &vTransaction;
        ImGui::PushID(ptr);
        if (ImGui::BeginPopupContextItem(               //
                NULL,                                   //
                ImGuiPopupFlags_NoOpenOverItems |       //
                    ImGuiPopupFlags_MouseButtonRight |  //
                    ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            if (ImGui::MenuItem("Update selection")) {
                std::vector<Transaction> transactions_to_update;
                for (const auto& trans : m_Datas.transactions_filtered) {
                    if (m_SelectedTransactions.find(trans.id) != m_SelectedTransactions.end()) {
                        transactions_to_update.push_back(trans);
                    }
                }
                if (transactions_to_update.size() > 1U) {
                    m_TransactionDialog.setTransactionsToUpdate(transactions_to_update);
                    m_TransactionDialog.show(DataDialogMode::MODE_UPDATE_ALL);
                } else if (transactions_to_update.size() == 1U) {
                    m_TransactionDialog.setTransaction(transactions_to_update.front());
                    m_TransactionDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
                }
            }
            if (ImGui::MenuItem("Delete selection")) {
                std::vector<Transaction> transactions_to_delete;
                for (const auto& trans : m_Datas.transactions_filtered) {
                    if (m_SelectedTransactions.find(trans.id) != m_SelectedTransactions.end()) {
                        transactions_to_delete.push_back(trans);
                    }
                }
                m_TransactionDialog.setTransactionsToDelete(transactions_to_delete);
                m_TransactionDialog.show(DataDialogMode::MODE_DELETE_ALL);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
}

void AccountPane::m_drawSearchRow() {
    bool change = false;
    bool reset = false;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    if (ImGui::ContrastedButton("R", nullptr, nullptr, ImGui::GetColumnWidth(0))) {
        reset = true;
    }
    ImGui::PopStyleVar();
    for (size_t idx = 0; idx < 10; ++idx) {
        ImGui::TableNextColumn();
        if (idx < SearchColumns::SEARCH_COLUMN_Count) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            if (m_SearchInputTexts.at(idx).DisplayInputText(ImGui::GetColumnWidth(idx), "", "")) {
                m_SearchTokens[idx] = ct::toLower(m_SearchInputTexts.at(idx).GetText());
                change = true;
            }
            ImGui::PopStyleVar();
        } else if (idx == 6) {
            m_drawAmount(m_TotalDebit);
        } else if (idx == 7) {
            m_drawAmount(m_TotalCredit);
        } else if (idx == 8) {
            // m_drawAmount(m_CurrentBaseSolde);
        } else if (idx == 9) {
            ImGui::Text("[%u]", (uint32_t)m_Datas.transactions_filtered.size());
        }
    }
    if (reset) {
        m_ResetFiltering();
    }
    if (change) {
        m_refreshFiltering();
    }
}

void AccountPane::m_drawAmount(const double& vAmount) {
    if (vAmount < 0.0) {
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
    }
}
