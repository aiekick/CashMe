#include <Models/DataBrokers.h>

#include <Plugins/PluginManager.h>

#include <ImGuiPack.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <csv/csv.h>

#include <Project/ProjectFile.h>

#include <Models/DataBase.h>

#include <Res/fontIcons.h>

#include <chrono>
#include <vector>
#include <ctime>

#define OFFSET_IN_DAYS_FROM_NOW 360

bool DataBrokers::init() {
    m_GetAvailableDataBrokers();
    return true;
}

void DataBrokers::unit() {
    m_Clear();
}

void DataBrokers::load() {
    m_refreshDatas();
}

bool DataBrokers::draw() {
    bool change = false;
    return change;
}

void DataBrokers::drawDialogs(const ImVec2& vPos, const ImVec2& vSize) {
    const ImVec2 center = vPos + vSize * 0.5f;
    m_DrawUserDialog(center);
    m_DrawBankDialog(center);
    m_DrawCategoryDialog(center);
    m_DrawOperationDialog(center);
    m_DrawAccountDialog(center);
    m_DrawTransactionDialog(center);
    if (ImGuiFileDialog::Instance()->Display("Import Datas")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            const auto& selection = ImGuiFileDialog::Instance()->GetSelection();
            if (!selection.empty()) {
                std::vector<std::string> files;
                for (const auto& s : selection) {
                    files.push_back(s.second);
                }
                m_ImportFromFiles(files);
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void DataBrokers::drawMenu(FrameActionSystem& vFrameActionSystem) {
    if (ProjectFile::Instance()->IsProjectLoaded()) {
        if (ImGui::BeginMenuBar()) {
            m_drawAccountsMenu(vFrameActionSystem);
            m_drawCreationMenu(vFrameActionSystem);
            m_drawUpdateMenu(vFrameActionSystem);
            m_drawImportMenu(vFrameActionSystem);
            m_drawSelectMenu(vFrameActionSystem);
#ifdef _DEBUG
            m_drawDebugMenu(vFrameActionSystem);
#endif
            ImGui::EndMenuBar();
        }
    }
}

void DataBrokers::DisplayAccounts() {
    ImGui::Header("Accounts");
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("##Accounts", 6, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Users", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Banks", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Number", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        size_t idx = 0U;
        for (const auto& a : m_Datas.accounts) {
            ImGui::TableNextRow();
            
            ImGui::PushID(a.id);

            ImGui::TableNextColumn();
            ImGui::PushID(&a);
            if (ImGui::Selectable(a.user.c_str(), m_SelectedAccountIdx == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                m_ResetSelection();
                m_UpdateTransactions(a.id);
                m_SelectedAccountIdx = idx;
            }
            ImGui::PopID();
            m_drawAccountMenu(a);

            ImGui::TableNextColumn();
            ImGui::Text(a.bank.c_str());

            ImGui::TableNextColumn();
            ImGui::Text(a.type.c_str());

            ImGui::TableNextColumn();
            ImGui::Text(a.name.c_str());

            ImGui::TableNextColumn();
            ImGui::Text(a.number.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%u", a.count);

            ImGui::PopID();

            ++idx;
        }
        ImGui::EndTable();
    }
}

void DataBrokers::DisplayTransactions() {
    ImGui::Header("Transactions");
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##Transactions", 9, flags)) {
        ImGui::TableSetupScrollFreeze(0, 2);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
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
                    ImGui::Selectable(t.desc.c_str(), &is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
                    if (ImGui::IsItemHovered()) {
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            m_ResetSelection();
                            m_SelectOrDeselectRow(t);
                            m_ShowTransactionDialog(DialogMode::UPDATE_MODE, t);
                        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                            m_SelectOrDeselectRow(t);
                        }
                    }
                    ImGui::PopID();
                    m_drawTransactionMenu(t); 
                }

                ImGui::TableNextColumn();
                { ImGui::Text(t.category.c_str()); }

                ImGui::TableNextColumn();
                { ImGui::Text(t.operation.c_str()); }

                ImGui::TableNextColumn();
                {
                    if (t.amount < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.amount);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.amount >= 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.amount);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.solde < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.solde);
                        ImGui::PopStyleColor();
                    } else if (t.solde > 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.solde);
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::Text("%.2f", t.solde);
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
        ImGui::EndTable();
    }
}

void DataBrokers::m_drawImportMenu(FrameActionSystem& vFrameActionSystem) {
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

void DataBrokers::m_drawSelectMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Select")) {
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

void DataBrokers::m_drawDebugMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Debug")) {
        if (ImGui::MenuItem("Refresh")) {
            m_refreshDatas();
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Delete Tables")) {
            if (ImGui::MenuItem("Users")) {
                DataBase::Instance()->DeleteUsers();
                m_refreshDatas();
            }
            if (ImGui::MenuItem("Banks")) {
                DataBase::Instance()->DeleteBanks();
                m_refreshDatas();
            }
            if (ImGui::MenuItem("Accounts")) {
                DataBase::Instance()->DeleteAccounts();
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

void DataBrokers::m_drawAccountsMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Accounts")) {
        for (const auto& user : m_Accounts) {
            if (ImGui::BeginMenu(user.first.c_str())) {
                for (const auto& bank : user.second) {
                    if (ImGui::BeginMenu(bank.first.c_str())) {
                        static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                        if (ImGui::BeginTable("##MenuAccounts", 4, flags)) {
                            ImGui::TableSetupScrollFreeze(0, 1);
                            ImGui::TableSetupColumn("Number", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableHeadersRow();
                            size_t idx = 0U;
                            for (const auto& number : bank.second) {
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

void DataBrokers::m_drawCreationMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("User")) {
            m_ShowUserDialog(DialogMode::CREATION_MODE);
        }
        if (ImGui::MenuItem("Bank")) {
            m_ShowBankDialog(DialogMode::CREATION_MODE);
        }
        if (ImGui::MenuItem("Account")) {
            m_ShowAccountDialog(DialogMode::CREATION_MODE, Account());
        }
        if (ImGui::MenuItem("Category")) {
            m_ShowCategoryDialog(DialogMode::CREATION_MODE);
        }
        if (ImGui::MenuItem("Operation")) {
            m_ShowOperationDialog(DialogMode::CREATION_MODE);
        }
        if (ImGui::MenuItem("Transaction")) {
            m_ShowTransactionDialog(DialogMode::CREATION_MODE, Transaction());
        }
        ImGui::EndMenu();
    }
}

void DataBrokers::m_drawUpdateMenu(FrameActionSystem& vFrameActionSystem) {
    /*if (ImGui::BeginMenu("Update")) {
        if (ImGui::MenuItem("User")) {
            m_ShowUserDialog(DialogMode::UPDATE_MODE);
        }
        if (ImGui::MenuItem("Bank")) {
            m_ShowBankDialog(DialogMode::UPDATE_MODE);
        }
        if (ImGui::MenuItem("Account")) {
            m_ShowAccountDialog(DialogMode::UPDATE_MODE);
        }
        if (ImGui::MenuItem("Category")) {
            m_ShowCategoryDialog(DialogMode::UPDATE_MODE);
        }
        if (ImGui::MenuItem("Operation")) {
            m_ShowOperationDialog(DialogMode::UPDATE_MODE);
        }
        if (ImGui::MenuItem("Transaction")) {
            m_ShowTransactionDialog(DialogMode::UPDATE_MODE);
        }
        ImGui::EndMenu();
    }*/
}

std::string DataBrokers::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);
    std::string res;
    res += vOffset + "<data_brokers>\n";
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
    res += vOffset + "</data_brokers>\n";
    return res;
}

bool DataBrokers::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != 0)
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

void DataBrokers::m_refreshDatas() {
    m_UpdateUsers();
    m_UpdateBanks();
    m_UpdateCategories();
    m_UpdateOperations();
    m_UpdateAccounts();
}

void DataBrokers::m_Clear() {
    m_SelectedBroker.reset(); // must be reset before quit since point on the memroy of a plugin
    m_DataBrokerModules.clear();
    m_Datas.clear();
}

void DataBrokers::m_GetAvailableDataBrokers() {
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

void DataBrokers::m_ImportFromFiles(const std::vector<std::string> vFiles) {
    if (!m_SelectedBroker.expired()) {
        auto ptr = m_SelectedBroker.lock();
        if (ptr != nullptr) {
            for (const auto& file : vFiles) {
                const auto& stmt = ptr->importBankStatement(file);
                if (!stmt.statements.empty()) {
                    RowID account_id = 0U;
                    if (DataBase::Instance()->GetAccount(stmt.account.number, account_id)) {
                        if (DataBase::Instance()->BeginTransaction()) {
                            for (const auto& s : stmt.statements) {
                                DataBase::Instance()->AddTransaction(account_id, //
                                    s.category, s.operation, s.date, s.label, s.comment, s.amount, s.count, s.confirmed, s.hash);
                            }
                            DataBase::Instance()->CommitTransaction();
                        }
                    }
                    else {
                        LogVarError("Import interrupted, no account found for %s", stmt.account.number.c_str());
                    }
                }
            }
        }
    }
}

void DataBrokers::m_ResetFiltering() {
    m_Datas.transactions_filtered = m_Datas.transactions;
    m_Datas.transactions_filtered_rowids = {};
    m_SearchInputTexts = {};
    m_SearchTokens = {};
    m_RefreshFiltering();
}

void DataBrokers::m_RefreshFiltering() {
    m_Datas.transactions_filtered.clear();
    bool use = false;
    double solde = m_CurrentBaseSolde;
    m_TotalDebit = 0.0;
    m_TotalCredit = 0.0;
    for (auto tr : m_Datas.transactions) {
        use = true;
        for (size_t idx = 0; idx < 3; ++idx) {
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

void DataBrokers::m_SelectOrDeselectRow(const Transaction& vTransaction) {
    if (m_SelectedTransactions.find(vTransaction.id) != m_SelectedTransactions.end()) {
        m_SelectedTransactions.erase(vTransaction.id);    // deselection
    } else {
        m_SelectedTransactions.emplace(vTransaction.id);  // selection
    }
}

bool DataBrokers::m_IsRowSelected(const RowID& vRowID) const {
    return (m_SelectedTransactions.find(vRowID) != m_SelectedTransactions.end());
}

void DataBrokers::m_ResetSelection() {
    m_SelectedTransactions.clear();
}

void DataBrokers::m_SelectPossibleDuplicateEntryOnPricesAndDates() {
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

void DataBrokers::m_SelectUnConfirmedTransactions() {
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

void DataBrokers::m_DeleteHoveredOrSelectedRows() {
    if (m_SelectedTransactions.empty()) {  // delete only the hovered line
        DataBase::Instance()->DeleteTransaction(m_TransactionToUpdate.id);
    } else {  // delete the selected rows
        DataBase::Instance()->DeleteTransactions(m_SelectedTransactions);
    }
}

void DataBrokers::m_UpdateTransactionsToDelete() {
    m_Datas.transactions_to_delete.clear();
    for (const auto& t : m_Datas.transactions_filtered) {
        if (m_IsRowSelected(t.id)) {
            m_Datas.transactions_to_delete.push_back(t);
        }
    }
}

void DataBrokers::m_UpdateUsers() {
    m_Datas.userNames.clear();
    DataBase::Instance()->GetUsers(          //
        [this](const UserName& vUserName) {  //
            m_Datas.userNames.push_back(vUserName);
        });
    m_UsersCombo = ImWidgets::QuickStringCombo(0, m_Datas.userNames);
}

void DataBrokers::m_ShowUserDialog(const DialogMode& vDialogMode) {
    m_showUserDialog = true;
    m_dialogMode = vDialogMode;
}

void DataBrokers::m_DrawUserDialog(const ImVec2& vPos) {
    if (m_showUserDialog) {
        ImGui::OpenPopup("UserCreationModalPopup");
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("UserCreationModalPopup",
                                   (bool*)nullptr,
                                   ImGuiWindowFlags_NoTitleBar |            //
                                       ImGuiWindowFlags_NoResize |          //
                                       ImGuiWindowFlags_AlwaysAutoResize |  //
                                       ImGuiWindowFlags_NoDocking)) {
            ImGui::Header("User Creation");
            ImGui::Separator();
            m_UserNameInputText.DisplayInputText(200.0f, "Name", "User");
            ImGui::Separator();
            if (!m_UserNameInputText.empty()) {
                if (ImGui::ContrastedButton("Ok")) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        DataBase::Instance()->AddUser(m_UserNameInputText.GetText());
                        DataBase::Instance()->CloseDBFile();
                    }
                    m_showUserDialog = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showUserDialog = false;
            }
            ImGui::EndPopup();
        }
    }
}

void DataBrokers::m_UpdateBanks() {
    m_Datas.bankNames.clear();
    DataBase::Instance()->GetBanks(                                       //
        [this](const BankName& vUserName, const std::string& /*vUrl*/) {  //
            m_Datas.bankNames.push_back(vUserName);
        });
    m_BanksCombo = ImWidgets::QuickStringCombo(0, m_Datas.bankNames);
}

void DataBrokers::m_ShowBankDialog(const DialogMode& vDialogMode) {
    m_showBankDialog = true;
    m_dialogMode = vDialogMode;
}

void DataBrokers::m_DrawBankDialog(const ImVec2& vPos) {
    if (m_showBankDialog) {
        ImGui::OpenPopup("BankCreationModalPopup");
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("BankCreationModalPopup",
                                   (bool*)nullptr,
                                   ImGuiWindowFlags_NoTitleBar |            //
                                       ImGuiWindowFlags_NoResize |          //
                                       ImGuiWindowFlags_AlwaysAutoResize |  //
                                       ImGuiWindowFlags_NoDocking)) {
            ImGui::Header("Bank Creation");
            ImGui::Separator();
            m_BankNameInputText.DisplayInputText(200.0f, "Name", "", false, 70.0f);
            m_BankUrlInputText.DisplayInputText(200.0f, "Url", "", false, 70.0f);
            ImGui::Separator();
            if (!m_BankNameInputText.empty()) {
                if (ImGui::ContrastedButton("Ok")) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        DataBase::Instance()->AddBank(m_BankNameInputText.GetText(), m_BankUrlInputText.GetText());
                        DataBase::Instance()->CloseDBFile();
                    }
                    m_showBankDialog = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showBankDialog = false;
            }
            ImGui::EndPopup();
        }
    }
}

void DataBrokers::m_UpdateCategories() {
    m_Datas.categoryNames.clear();
    DataBase::Instance()->GetCategories(             //
        [this](const CategoryName& vCategoryName) {  //
            m_Datas.categoryNames.push_back(vCategoryName);
        });
    m_CategoriesCombo = ImWidgets::QuickStringCombo(0, m_Datas.categoryNames);
}

void DataBrokers::m_ShowCategoryDialog(const DialogMode& vDialogMode) {
    m_showCategoryDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateCategories();
}

void DataBrokers::m_DrawCategoryDialog(const ImVec2& vPos) {
    if (m_showCategoryDialog) {
        ImGui::OpenPopup("CategoryCreationModalPopup");
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("CategoryCreationModalPopup",
                                   (bool*)nullptr,
                                   ImGuiWindowFlags_NoTitleBar |            //
                                       ImGuiWindowFlags_NoResize |          //
                                       ImGuiWindowFlags_AlwaysAutoResize |  //
                                       ImGuiWindowFlags_NoDocking)) {
            ImGui::Header("Category Creation");
            ImGui::Separator();
            m_CategoryNameInputText.DisplayInputText(200.0f, "Name", "", false, 70.0f);
            ImGui::Separator();
            if (!m_CategoryNameInputText.empty()) {
                if (ImGui::ContrastedButton("Ok")) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        DataBase::Instance()->AddCategory(m_CategoryNameInputText.GetText());
                        DataBase::Instance()->CloseDBFile();
                    }
                    m_showCategoryDialog = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showCategoryDialog = false;
            }
            ImGui::EndPopup();
        }
    }
}

void DataBrokers::m_UpdateOperations() {
    m_Datas.operationNames.clear();
    DataBase::Instance()->GetOperations(               //
        [this](const OperationName& vOperationName) {  //
            m_Datas.operationNames.push_back(vOperationName);
        });
    m_OperationsCombo = ImWidgets::QuickStringCombo(0, m_Datas.operationNames);
}

void DataBrokers::m_ShowOperationDialog(const DialogMode& vDialogMode) {
    m_showOperationDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateOperations();
}

void DataBrokers::m_DrawOperationDialog(const ImVec2& vPos) {
    if (m_showOperationDialog) {
        ImGui::OpenPopup("OperationCreationModalPopup");
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("OperationCreationModalPopup",
                                   (bool*)nullptr,
                                   ImGuiWindowFlags_NoTitleBar |            //
                                       ImGuiWindowFlags_NoResize |          //
                                       ImGuiWindowFlags_AlwaysAutoResize |  //
                                       ImGuiWindowFlags_NoDocking)) {
            ImGui::Header("Operation Creation");
            ImGui::Separator();
            m_OperationNameInputText.DisplayInputText(200.0f, "Name", "", false, 70.0f);
            ImGui::Separator();
            if (!m_OperationNameInputText.empty()) {
                if (ImGui::ContrastedButton("Ok")) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        DataBase::Instance()->AddOperation(m_OperationNameInputText.GetText());
                        DataBase::Instance()->CloseDBFile();
                    }
                    m_showOperationDialog = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showOperationDialog = false;
            }
            ImGui::EndPopup();
        }
    }
}

void DataBrokers::m_UpdateAccounts() {
    m_Datas.accounts.clear();
    m_Datas.accountNumbers.clear();
    DataBase::Instance()->GetAccounts(  //
        [this](const RowID& vRowID,
               const UserName& vUserName,
               const BankName& vBankName,
               const AccountType& vAccountType,
               const AccountName& vAccountName,
               const AccountNumber& vAccountNumber,
               const AccounBaseSolde& vBaseSolde,
               const TransactionsCount& vCount) {  //
            Account a;
            a.id = vRowID;
            a.user = vUserName;
            a.bank = vBankName;
            a.type = vAccountType;
            a.name = vAccountName;
            a.number = vAccountNumber;
            a.base_solde = vBaseSolde;
            a.count = vCount;
            m_Datas.accounts.push_back(a);
            m_Datas.accountNumbers.push_back(vAccountNumber);
            m_Accounts[vUserName][vBankName][vAccountNumber] = a;
        });
    m_AccountsCombo = ImWidgets::QuickStringCombo(0, m_Datas.accountNumbers);
    if (m_SelectedAccountIdx < m_Datas.accounts.size()) {
        m_UpdateTransactions(m_Datas.accounts.at(m_SelectedAccountIdx).id);
    }
}

void DataBrokers::m_drawAccountMenu(const Account& vAccount) {
    ImGui::PushID(vAccount.id);
    if (ImGui::BeginPopupContextItem(               //
            NULL,                     //
            ImGuiPopupFlags_NoOpenOverItems |       //
                ImGuiPopupFlags_MouseButtonRight |  //
                ImGuiPopupFlags_NoOpenOverExistingPopup)) {
        if (ImGui::MenuItem("Update")) {
            m_ShowAccountDialog(DialogMode::UPDATE_MODE, vAccount);
        }
        if (ImGui::MenuItem("Delete")) {
            m_ShowAccountDialog(DialogMode::DELETE_MODE, vAccount);
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void DataBrokers::m_ShowAccountDialog(const DialogMode& vDialogMode, const Account& vAccount) {
    m_showAccountDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateUsers();
    m_UpdateBanks();
    if (m_dialogMode == DialogMode::UPDATE_MODE) {
        m_UsersCombo.Select(vAccount.user);
        m_BanksCombo.Select(vAccount.bank);
        m_AccountNameInputText.SetText(vAccount.name);
        m_AccountTypeInputText.SetText(vAccount.type);
        m_AccountNumberInputText.SetText(vAccount.number);
        m_AccountBaseSoldeInputDouble = vAccount.base_solde;
        m_AccountToUpdate = vAccount;
    } else if (m_dialogMode == DialogMode::DELETE_MODE) {
        m_AccountToUpdate = vAccount;
    }
}

void DataBrokers::m_DrawAccountDialog(const ImVec2& vPos) {
    if (m_showAccountDialog) {
        if (m_dialogMode == DialogMode::DELETE_MODE) {
            ImGui::OpenPopup("AccountDeleteModalPopup");
            ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("AccountDeleteModalPopup",
                                       (bool*)nullptr,
                                       ImGuiWindowFlags_NoTitleBar |            //
                                           ImGuiWindowFlags_NoResize |          //
                                           ImGuiWindowFlags_AlwaysAutoResize |  //
                                           ImGuiWindowFlags_NoDocking)) {
                ImGui::Header("Account Deletion Confirmation");
                if (ImGui::ContrastedButton("Confirm Deletion")) {
                    DataBase::Instance()->DeleteAccount(m_AccountToUpdate.id);
                    m_showAccountDialog = false;
                    m_refreshDatas();
                }
                ImGui::SameLine();
                if (ImGui::ContrastedButton("Cancel Deletion")) {
                    m_showAccountDialog = false;
                }
                ImGui::EndPopup();
            }
        } else {
            ImGui::OpenPopup("AccountCreationModalPopup");
            ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("AccountCreationModalPopup",
                                       (bool*)nullptr,
                                       ImGuiWindowFlags_NoTitleBar |            //
                                           ImGuiWindowFlags_NoResize |          //
                                           ImGuiWindowFlags_AlwaysAutoResize |  //
                                           ImGuiWindowFlags_NoDocking)) {
                if (m_dialogMode == DialogMode::UPDATE_MODE) {
                    ImGui::Header("Account Update");
                } else if (m_dialogMode == DialogMode::CREATION_MODE) {
                    ImGui::Header("Account Creation");
                }
                ImGui::Separator();
                const float& align = 125.0f;
                const auto& width = 400.0f;
                m_UsersCombo.DisplayCombo(width, "User Name", align);
                m_BanksCombo.DisplayCombo(width, "Bank Name", align);
                m_AccountNameInputText.DisplayInputText(width, "Account Name", "", false, align);
                m_AccountTypeInputText.DisplayInputText(width, "Account Type", "", false, align);
                m_AccountNumberInputText.DisplayInputText(width, "Account Number", "", false, align);
                float px = ImGui::GetCursorPosX();
                ImGui::Text("Base Solde");
                ImGui::SameLine(align);
                const float w = width - (ImGui::GetCursorPosX() - px);
                ImGui::PushID(++ImGui::CustomStyle::pushId);
                ImGui::PushItemWidth(w);
                ImGui::InputDouble("##BaseSolde", &m_AccountBaseSoldeInputDouble);
                ImGui::PopItemWidth();
                ImGui::PopID();
                ImGui::Separator();
                if (!m_AccountNameInputText.empty() && !m_AccountTypeInputText.empty() && !m_AccountNumberInputText.empty()) {
                    if (ImGui::ContrastedButton("Ok")) {
                        if (DataBase::Instance()->OpenDBFile()) {
                            if (m_dialogMode == DialogMode::CREATION_MODE) {
                                DataBase::Instance()->AddAccount(      //
                                    m_UsersCombo.GetText(),            //
                                    m_BanksCombo.GetText(),            //
                                    m_AccountTypeInputText.GetText(),  //
                                    m_AccountNameInputText.GetText(),  //
                                    m_AccountNumberInputText.GetText(),
                                    m_AccountBaseSoldeInputDouble);
                            } else if (m_dialogMode == DialogMode::UPDATE_MODE) {
                                DataBase::Instance()->UpdateAccount(  //
                                    m_AccountToUpdate.id,
                                    m_UsersCombo.GetText(),            //
                                    m_BanksCombo.GetText(),            //
                                    m_AccountTypeInputText.GetText(),  //
                                    m_AccountNameInputText.GetText(),  //
                                    m_AccountNumberInputText.GetText(),
                                    m_AccountBaseSoldeInputDouble);
                            }
                            DataBase::Instance()->CloseDBFile();
                            m_refreshDatas();
                            m_showAccountDialog = false;
                        }
                    }
                    ImGui::SameLine();
                }
                if (ImGui::ContrastedButton("Cancel")) {
                    m_showAccountDialog = false;
                }
                ImGui::EndPopup();
            }
        }
    }
}

void DataBrokers::m_UpdateTransactions(const RowID& vAccountID) {
    m_Datas.transactions.clear();
    const auto& zero_based_account_id = vAccountID - 1;
    if (zero_based_account_id < m_Datas.accounts.size()) {
        double solde = m_CurrentBaseSolde = m_Datas.accounts.at(zero_based_account_id).base_solde;
        const auto& account_number = m_Datas.accounts.at(zero_based_account_id).number;
        DataBase::Instance()->GetTransactions(  //
            vAccountID,                         //
            [this, &solde, account_number](const RowID& vTransactionID,
                           const TransactionDate& vTransactionDate,
                           const TransactionDescription& vTransactionDescription,
                           const TransactionComment& vTransactionComment,
                           const CategoryName& vCategoryName,
                           const OperationName& vOperationName,
                           const TransactionAmount& vTransactionAmount,
                           const TransactionsDoublon& vDoublons,
                           const TransactionsConfirmed& vConfirmed) {  //
                solde += vTransactionAmount;
                Transaction t;
                t.id = vTransactionID;
                t.account = account_number;
                t.date = vTransactionDate;
                t.desc = vTransactionDescription;
                t.optimized[0] = ct::toLower(vTransactionDescription);
                t.comm = vTransactionComment;
                t.category = vCategoryName;
                t.optimized[1] = ct::toLower(vCategoryName);
                t.operation = vOperationName;
                t.optimized[2] = ct::toLower(vOperationName);
                t.amount = vTransactionAmount;
                t.doublons = vDoublons;
                t.confirmed = vConfirmed;
                t.solde = solde;
                assert(t.doublons != 0);  // must be one at least
                // i y a des operations qui sont en N doublons
                // donc on ajoute plusieurs fois l'operation
                for (uint32_t idx = 0; idx < t.doublons; ++idx) {
#ifdef _DEBUG
                    if (t.doublons > 1) {
                        t.desc = ct::toStr("[Doublon : %u] ", idx) + vTransactionDescription;  // just for lcoate them in debug mode
                        t.optimized[0] = ct::toLower(t.desc);
                    }
#endif
                    m_Datas.transactions.push_back(t);
                }
            });
    }
    m_RefreshFiltering();
}

void DataBrokers::m_drawTransactionMenu(const Transaction& vTransaction) {
    if (!m_SelectedTransactions.empty()) {
        const auto* ptr = &vTransaction;
        ImGui::PushID(ptr);
        if (ImGui::BeginPopupContextItem(               //
                NULL,                                   //
                ImGuiPopupFlags_NoOpenOverItems |       //
                    ImGuiPopupFlags_MouseButtonRight |  //
                    ImGuiPopupFlags_NoOpenOverExistingPopup)) {
            if (ImGui::MenuItem("Delete selection")) {
                m_ShowTransactionDialog(DialogMode::DELETE_MODE, vTransaction);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
}

void DataBrokers::m_ShowTransactionDialog(const DialogMode& vDialogMode, const Transaction& vTransaction) {
    m_showTransactionDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateAccounts();
    m_UpdateCategories();
    m_UpdateOperations();
    if (m_dialogMode == DialogMode::UPDATE_MODE) {
        m_AccountsCombo.Select(vTransaction.account);
        m_CategoriesCombo.Select(vTransaction.category);
        m_OperationsCombo.Select(vTransaction.operation);
        m_TransactionDateInputText.SetText(vTransaction.date);
        m_TransactionDescriptionInputText.SetText(vTransaction.desc);
        m_TransactionCommentInputText.SetText(vTransaction.comm);
        m_TransactionAmountInputDouble = vTransaction.amount;
        m_TransactionsDoublonInputUint = vTransaction.doublons;
        m_TransactionToUpdate = vTransaction;
    } else if (m_dialogMode == DialogMode::DELETE_MODE) {
        m_TransactionToUpdate = vTransaction;
        m_UpdateTransactionsToDelete();
    }
}

void DataBrokers::m_DrawTransactionDialog(const ImVec2& vPos) {
    if (m_showTransactionDialog) {
        if (m_dialogMode == DialogMode::DELETE_MODE) {
            m_DrawTranactionDeletionDialog(vPos);
        } else {
            m_DrawTranactionCreationDialog(vPos);
        }
    }
}
void DataBrokers::m_DrawTranactionCreationDialog(const ImVec2& vPos) {
    ImGui::OpenPopup("AccountTransactionCreationModalPopup");
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("AccountTransactionCreationModalPopup",
                               (bool*)nullptr,
                               ImGuiWindowFlags_NoTitleBar |            //
                                   ImGuiWindowFlags_NoResize |          //
                                   ImGuiWindowFlags_AlwaysAutoResize |  //
                                   ImGuiWindowFlags_NoDocking)) {
        if (m_dialogMode == DialogMode::CREATION_MODE) {
            ImGui::Header("Transaction Creation");
        } else if (m_dialogMode == DialogMode::UPDATE_MODE) {
            ImGui::Header("Transaction Update");
        }
        ImGui::Separator();
        const float& align = 125.0f;
        const auto& width = 400.0f;
        if (m_dialogMode == DialogMode::CREATION_MODE) {
            m_AccountsCombo.DisplayCombo(width, "Account Number", align);
        } else if (m_dialogMode == DialogMode::UPDATE_MODE) {
            float px = ImGui::GetCursorPosX();
            ImGui::Text("Account Number");
            ImGui::SameLine(align);
            const float w = width - (ImGui::GetCursorPosX() - px);
            ImGui::PushID(++ImGui::CustomStyle::pushId);
            ImGui::PushItemWidth(w);
            ImGui::Text("%s", m_AccountsCombo.GetText().c_str());
            ImGui::PopItemWidth();
            ImGui::PopID();
        }
        m_CategoriesCombo.DisplayCombo(width, "Category", align);
        m_OperationsCombo.DisplayCombo(width, "Operation", align);
        m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
        m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
        m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", false, align);
        float px = ImGui::GetCursorPosX();
        ImGui::Text("Amount");
        ImGui::SameLine(align);
        const float w = width - (ImGui::GetCursorPosX() - px);
        ImGui::PushID(++ImGui::CustomStyle::pushId);
        ImGui::PushItemWidth(w);
        ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble);
        ImGui::PopItemWidth();
        ImGui::PopID();
        ImGui::Text("Nombre de Doublons");
        ImGui::SameLine(align);
        ImGui::PushID(++ImGui::CustomStyle::pushId);
        ImGui::PushItemWidth(w);
        if (ImGui::InputInt("##Doublons", &m_TransactionsDoublonInputUint)) {
            if (m_TransactionsDoublonInputUint < 1) {
                m_TransactionsDoublonInputUint = 1;
            }
        }
        ImGui::PopItemWidth();
        ImGui::PopID();

        ImGui::Separator();
        if (!m_TransactionDateInputText.empty() && !m_TransactionDescriptionInputText.empty() && m_TransactionAmountInputDouble != 0.0) {
            if (ImGui::ContrastedButton("Ok")) {
                RowID account_id = 0U;
                if (DataBase::Instance()->GetAccount(m_AccountsCombo.GetText(), account_id)) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        const auto hash = ct::toStr(  //
                            "%s_%s_%f",               //
                            m_TransactionDateInputText.GetText().c_str(),
                            // un fichier ofc ne peut pas avoir des label de longueur > a 30
                            // alors on limite le hash a utiliser un label de 30
                            // comme cela un ofc ne rentrera pas un collision avec un autre type de fcihier comme les pdf par ex
                            m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                            m_TransactionAmountInputDouble);  // must be unique per oepration
                        if (m_dialogMode == DialogMode::CREATION_MODE) {
                            DataBase::Instance()->AddTransaction(             //
                                account_id,                                   //
                                m_CategoriesCombo.GetText(),                  //
                                m_OperationsCombo.GetText(),                  //
                                m_TransactionDateInputText.GetText(),         //
                                m_TransactionDescriptionInputText.GetText(),  //
                                m_TransactionCommentInputText.GetText(),      //
                                m_TransactionAmountInputDouble,
                                m_TransactionsDoublonInputUint,
                                false,
                                hash);
                        } else if (m_dialogMode == DialogMode::UPDATE_MODE) {
                            DataBase::Instance()->UpdateTransaction(          //
                                m_TransactionToUpdate.id,                     //
                                m_CategoriesCombo.GetText(),                  //
                                m_OperationsCombo.GetText(),                  //
                                m_TransactionDateInputText.GetText(),         //
                                m_TransactionDescriptionInputText.GetText(),  //
                                m_TransactionCommentInputText.GetText(),      //
                                m_TransactionAmountInputDouble,
                                m_TransactionsDoublonInputUint,
                                false,
                                hash);
                        }
                        DataBase::Instance()->CloseDBFile();
                    }
                }
                m_showTransactionDialog = false;
                m_refreshDatas();
            }
            ImGui::SameLine();
        }
        if (ImGui::ContrastedButton("Cancel")) {
            m_showTransactionDialog = false;
        }
        ImGui::EndPopup();
    }
}

void DataBrokers::m_DrawTranactionDeletionDialog(const ImVec2& vPos) {
    ImGui::OpenPopup("TransactionDeleteModalPopup");
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("TransactionDeleteModalPopup",
                               (bool*)nullptr,
                               ImGuiWindowFlags_NoTitleBar |            //
                                   ImGuiWindowFlags_NoResize |          //
                                   ImGuiWindowFlags_AlwaysAutoResize |  //
                                   ImGuiWindowFlags_NoDocking)) {
        ImGui::Header("Transaction Deletion Confirmation");
        const auto& displaySize = ImGui::GetIO().DisplaySize * 0.5f;
        static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
        if (ImGui::BeginTable("##TransactionsToDelete", 5, flags, displaySize)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            int32_t idx = 0;
            int32_t idx_to_delete = -1;
            const float& item_h = ImGui::GetTextLineHeightWithSpacing();
            const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
            const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
            m_TransactionsDeletionListClipper.Begin((int)m_Datas.transactions_to_delete.size(), item_h);
            while (m_TransactionsDeletionListClipper.Step()) {
                for (idx = m_TransactionsDeletionListClipper.DisplayStart; idx < m_TransactionsDeletionListClipper.DisplayEnd; ++idx) {
                    if (idx < 0) {
                        continue;
                    }

                    const auto& t = m_Datas.transactions_to_delete.at(idx);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    {
                        if (ImGui::SmallContrastedButton("-")) {
                            idx_to_delete = idx;
                        }
                    }

                    ImGui::TableNextColumn();
                    { ImGui::Text(t.date.c_str()); }

                    ImGui::TableNextColumn();
                    { ImGui::Text(t.desc.c_str()); }

                    ImGui::TableNextColumn();
                    {
                        if (t.amount < 0.0) {
                            ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                            ImGui::Text("%.2f", t.amount);
                            ImGui::PopStyleColor();
                        }
                    }

                    ImGui::TableNextColumn();
                    {
                        if (t.amount >= 0.0) {
                            ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                            ImGui::Text("%.2f", t.amount);
                            ImGui::PopStyleColor();
                        }
                    }
                }
            }
            m_TransactionsDeletionListClipper.End();
            ImGui::EndTable();

            if (idx_to_delete > -1) {
                const auto& t = m_Datas.transactions_to_delete.at(idx_to_delete);
                m_SelectOrDeselectRow(t);
                m_UpdateTransactionsToDelete();
            }
        } 

        if (ImGui::ContrastedButton("Confirm Deletion")) {
            m_DeleteHoveredOrSelectedRows();
            m_showTransactionDialog = false;
            m_refreshDatas();
        }
        ImGui::SameLine();
        if (ImGui::ContrastedButton("Cancel Deletion")) {
            m_showTransactionDialog = false;
        }
        ImGui::EndPopup();
    }
}

void DataBrokers::m_drawSearchRow() {
    bool change = false;
    bool reset = false;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    for (size_t idx = 0; idx < 8; ++idx) {
        ImGui::TableNextColumn();
        switch(idx) {
            case 0: {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (ImGui::ContrastedButton("Reset", nullptr, nullptr, ImGui::GetColumnWidth(idx))) {
                    reset = true;
                }
                ImGui::PopStyleVar();
            }
            break;
            case 1:
            case 2:
            case 3:{
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (m_SearchInputTexts.at(idx - 1).DisplayInputText(ImGui::GetColumnWidth(idx), "", "")) {
                    m_SearchTokens[idx - 1] = ct::toLower(m_SearchInputTexts.at(idx - 1).GetText());
                    change = true;
                }
                ImGui::PopStyleVar();
            }
            break;
            case 4:
                m_drawAmount(m_TotalDebit);
                break;
            case 5:
                m_drawAmount(m_TotalCredit);
                break;
            case 6:
                //m_drawAmount(m_CurrentBaseSolde);
                break;
            case 7:
                ImGui::Text("[%u]", (uint32_t)m_Datas.transactions_filtered.size());
                break;
            default:
                break;
        }
    }
    if (reset) {
        m_ResetFiltering();
    }
    if (change) {
        m_RefreshFiltering();
    }
}

void DataBrokers::m_drawAmount(const double& vAmount) {
    if (vAmount < 0.0) {
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
    }
}