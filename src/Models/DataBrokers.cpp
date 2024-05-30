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
    bool ret = true;
    m_GetAvailableDataBrokers();
    ret &= m_BankDialog.init();
    ret &= m_AccountDialog.init();
    ret &= m_CategoryDialog.init();
    ret &= m_OperationDialog.init();
    ret &= m_TransactionDialog.init();
    return ret;
}

void DataBrokers::unit() {
    m_BankDialog.init();
    m_AccountDialog.init();
    m_CategoryDialog.init();
    m_OperationDialog.init();
    m_TransactionDialog.unit();
    m_Clear();
}

void DataBrokers::load() {
    RefreshDatas();
}

bool DataBrokers::draw() {
    bool change = false;
    return change;
}

void DataBrokers::drawDialogs(const ImVec2& vPos, const ImVec2& vSize) {
    const ImVec2 center = vPos + vSize * 0.5f;
    m_BankDialog.draw(center);
    m_AccountDialog.draw(center);
    m_CategoryDialog.draw(center);
    m_OperationDialog.draw(center);
    m_TransactionDialog.draw(center);
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

void DataBrokers::DisplayTransactions() {
    ImGui::Header("Transactions");
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 30.0f);
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##Transactions", 10, flags)) {
        ImGui::TableSetupScrollFreeze(0, 2);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Comments", ImGuiTableColumnFlags_WidthStretch);
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
                    ImGui::Selectable(t.description.c_str(), &is_selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
                    if (ImGui::IsItemHovered()) {
                        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                            m_ResetSelection();
                            m_SelectOrDeselectRow(t);
                            m_TransactionDialog.setTransaction(t);
                            m_TransactionDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
                        } else if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                            m_SelectOrDeselectRow(t);
                        }
                    }
                    m_HideByFilledRectForHiddenMode("%s", t.description.c_str());
                    ImGui::PopID();
                    m_drawTransactionMenu(t); 
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.comment.c_str());
                    m_HideByFilledRectForHiddenMode("%s", t.comment.c_str());
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.category.c_str());
                    m_HideByFilledRectForHiddenMode("%s", t.category.c_str());
                }

                ImGui::TableNextColumn();
                {
                    ImGui::Text(t.operation.c_str());
                    m_HideByFilledRectForHiddenMode("%s", t.operation.c_str());
                }

                ImGui::TableNextColumn();
                {
                    if (t.amount < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.amount);
                        m_HideByFilledRectForHiddenMode("%.2f", t.amount);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.amount >= 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.amount);
                        m_HideByFilledRectForHiddenMode("%.2f", t.amount);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.solde < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.solde);
                        m_HideByFilledRectForHiddenMode("%.2f", t.solde);
                        ImGui::PopStyleColor();
                    } else if (t.solde > 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.solde);
                        m_HideByFilledRectForHiddenMode("%.2f", t.solde);
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::Text("%.2f", t.solde);
                        m_HideByFilledRectForHiddenMode("%.2f", t.solde);
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
    ImGui::PopStyleVar();
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

void DataBrokers::m_drawDebugMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Debug")) {
        if (ImGui::MenuItem("Refresh")) {
            RefreshDatas();
        }
        ImGui::Separator();
        ImGui::MenuItem("Hidden Mode", nullptr, &m_HiddenMode);
        ImGui::Separator();
        if (ImGui::BeginMenu("Delete Tables")) {
            if (ImGui::MenuItem("Banks")) {
                DataBase::Instance()->DeleteBanks();
                RefreshDatas();
            }
            if (ImGui::MenuItem("Accounts")) {
                DataBase::Instance()->DeleteAccounts();
                RefreshDatas();
            }
            if (ImGui::MenuItem("Categories")) {
                DataBase::Instance()->DeleteCategories();
                RefreshDatas();
            }
            if (ImGui::MenuItem("Operations")) {
                DataBase::Instance()->DeleteOperations();
                RefreshDatas();
            }
            if (ImGui::MenuItem("Transactions")) {
                DataBase::Instance()->DeleteTransactions();
                RefreshDatas();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
}

void DataBrokers::m_drawAccountsMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Accounts")) {
        for (const auto& bank : m_Accounts) {
            if (ImGui::BeginMenu(bank.first.c_str())) { // bank name
                for (const auto& agency : bank.second) {
                    if (ImGui::BeginMenu(agency.first.c_str())) { // bank agency
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

void DataBrokers::m_drawCreationMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("Bank")) {
            m_BankDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Account")) {
            m_AccountDialog.show(DataDialogMode::MODE_CREATION);
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

void DataBrokers::m_drawUpdateMenu(FrameActionSystem& vFrameActionSystem) {
    /*if (ImGui::BeginMenu("Update")) {
        if (ImGui::MenuItem("User")) {
            m_ShowUserDialog(DialogMode::UPDATE_ONCE_MODE);
        }
        if (ImGui::MenuItem("Bank")) {
            m_ShowBankDialog(DialogMode::UPDATE_ONCE_MODE);
        }
        if (ImGui::MenuItem("Account")) {
            m_ShowAccountDialog(DialogMode::UPDATE_ONCE_MODE);
        }
        if (ImGui::MenuItem("Category")) {
            m_ShowCategoryDialog(DialogMode::UPDATE_ONCE_MODE);
        }
        if (ImGui::MenuItem("Operation")) {
            m_ShowOperationDialog(DialogMode::UPDATE_ONCE_MODE);
        }
        if (ImGui::MenuItem("Transaction")) {
            m_ShowTransactionDialog(DialogMode::UPDATE_ONCE_MODE);
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

void DataBrokers::RefreshDatas() {
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
    auto ptr = m_SelectedBroker.lock();
    if (ptr != nullptr) {
        for (const auto& file : vFiles) {
            const auto& stmt = ptr->importBankStatement(file);
            if (!stmt.statements.empty()) {
                RowID account_id = 0U;
                if (DataBase::Instance()->GetAccount(stmt.account.number, account_id)) {
                    if (DataBase::Instance()->BeginTransaction()) {
                        for (const auto& s : stmt.statements) {
                            DataBase::Instance()->AddTransaction(  //
                                account_id,                        //
                                s.operation,
                                s.category,
                                s.source,
                                s.source_type,
                                s.source_sha1,
                                s.date,
                                s.description,
                                s.comment,
                                s.amount,
                                s.confirmed,
                                s.hash); 
                        }
                        DataBase::Instance()->CommitTransaction();
                    }
                } else {
                    LogVarError("Import interrupted, no account found for %s", stmt.account.number.c_str());
                    break;
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
    m_Datas.transactions_filtered_rowids.clear();
    bool use = false;
    double solde = m_CurrentBaseSolde;
    m_TotalDebit = 0.0;
    m_TotalCredit = 0.0;
    for (auto tr : m_Datas.transactions) {
        use = true;
        for (size_t idx = 0; idx < 5; ++idx) {
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

void DataBrokers::m_SelectCurrentRows() {
    m_SelectedTransactions = m_Datas.transactions_filtered_rowids;
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
        CTOOL_DEBUG_BREAK;
        //DataBase::Instance()->DeleteTransaction(m_TransactionToUpdate.id);
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

bool DataBrokers::m_IsHiddenMode() {
    return
#ifdef _DEBUG
        m_HiddenMode;
#else
        false;
#endif
}

void DataBrokers::m_HideByFilledRectForHiddenMode(const char* fmt, ...) {
    if (m_IsHiddenMode()) {
        va_list args;
        va_start(args, fmt);
        const char *text, *text_end;
        ImFormatStringToTempBufferV(&text, &text_end, fmt, args);
        if (strlen(text) != 0) {
            const float& item_h = ImGui::GetTextLineHeightWithSpacing();
            const auto min = ImGui::GetCursorScreenPos() - ImVec2(0, item_h);
            const auto max = min + ImGui::CalcTextSize(text, text_end);
            auto drawListPtr = ImGui::GetWindowDrawList();
            drawListPtr->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_Text));
        }
        va_end(args);
    }
}

void DataBrokers::m_DisplayAlignedWidget(const float& vWidth, const std::string& vLabel, const float& vOffsetFromStart, std::function<void()> vWidget) {
    float px = ImGui::GetCursorPosX();
    ImGui::Text("%s", vLabel.c_str());
    ImGui::SameLine(vOffsetFromStart);
    const float w = vWidth - (ImGui::GetCursorPosX() - px);
    ImGui::PushID(++ImGui::CustomStyle::pushId);
    ImGui::PushItemWidth(w);
    if (vWidget != nullptr) {
        vWidget();
    }
    ImGui::PopItemWidth();
    ImGui::PopID();
}

void DataBrokers::m_UpdateBanks() {
    m_Datas.bankNames.clear();
    DataBase::Instance()->GetBanks(                                       //
        [this](const BankName& vUserName, const std::string& /*vUrl*/) {  //
            m_Datas.bankNames.push_back(vUserName);
        });
}

void DataBrokers::m_UpdateCategories() {
    m_Datas.categoryNames.clear();
    DataBase::Instance()->GetCategories(             //
        [this](const CategoryName& vCategoryName) {  //
            m_Datas.categoryNames.push_back(vCategoryName);
        });
}

void DataBrokers::m_UpdateOperations() {
    m_Datas.operationNames.clear();
    DataBase::Instance()->GetOperations(               //
        [this](const OperationName& vOperationName) {  //
            m_Datas.operationNames.push_back(vOperationName);
        });
}

void DataBrokers::m_UpdateAccounts() {
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

void DataBrokers::m_drawAccountMenu(const Account& vAccount) {
    ImGui::PushID(vAccount.id);
    if (ImGui::BeginPopupContextItem(               //
            NULL,                     //
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

/*void DataBrokers::m_ShowAccountDialog(DialogMode vDialogMode, const Account& vAccount) {
    m_showAccountDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateBanks();
    if (m_dialogMode & DIALOG_UPDATE_ONCE) {
        m_BanksCombo.Select(vAccount.bank);
        m_AccountNameInputText.SetText(vAccount.name);
        m_AccountTypeInputText.SetText(vAccount.type);
        m_AccountNumberInputText.SetText(vAccount.number);
        m_AccountBaseSoldeInputDouble = vAccount.base_solde;
        m_AccountToUpdate = vAccount;
    } else if (m_dialogMode & DIALOG_MODE_DELETE) {
        m_AccountToUpdate = vAccount;
    }
}

void DataBrokers::m_DrawAccountDialog(const ImVec2& vPos) {
    if (m_showAccountDialog) {
        if (m_dialogMode & DIALOG_MODE_DELETE) {
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
                    RefreshDatas();
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
                if (m_dialogMode & DIALOG_MODE_UPDATE) {
                    ImGui::Header("Account Update");
                } else if (m_dialogMode & DIALOG_MODE_CREATION) {
                    ImGui::Header("Account Creation");
                }
                ImGui::Separator();
                const float& align = 125.0f;
                const auto& width = 400.0f;
                m_BanksCombo.DisplayCombo(width, "Bank Name", align);
                m_AccountNameInputText.DisplayInputText(width, "Account Name", "", false, align);
                m_AccountTypeInputText.DisplayInputText(width, "Account Type", "", false, align);
                m_AccountNumberInputText.DisplayInputText(width, "Account Number", "", false, align);
                m_DisplayAlignedWidget(width, "Base Solde", align, [this]() { ImGui::InputDouble("##BaseSolde", &m_AccountBaseSoldeInputDouble); });
                ImGui::Separator();
                if (!m_AccountNameInputText.empty() && !m_AccountTypeInputText.empty() && !m_AccountNumberInputText.empty()) {
                    if (ImGui::ContrastedButton("Ok")) {
                        if (DataBase::Instance()->OpenDBFile()) {
                            if (m_dialogMode & DIALOG_MODE_CREATION) {
                                DataBase::Instance()->AddAccount(        //
                                    m_BanksCombo.GetText(),              //
                                    m_BankAgencyInputText.GetText(),     //
                                    m_AccountTypeInputText.GetText(),    //
                                    m_AccountNameInputText.GetText(),    //
                                    m_AccountNumberInputText.GetText(),  //
                                    m_AccountBaseSoldeInputDouble);
                            } else if (m_dialogMode & DIALOG_UPDATE_ONCE) {
                                DataBase::Instance()->UpdateAccount(  //
                                    m_AccountToUpdate.id,
                                    m_BanksCombo.GetText(),            //
                                    m_BankAgencyInputText.GetText(),   //
                                    m_AccountTypeInputText.GetText(),  //
                                    m_AccountNameInputText.GetText(),  //
                                    m_AccountNumberInputText.GetText(),
                                    m_AccountBaseSoldeInputDouble);
                            }
                            DataBase::Instance()->CloseDBFile();
                            RefreshDatas();
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
}*/

void DataBrokers::m_UpdateTransactions(const RowID& vAccountID) {
    m_Datas.transactions.clear();
    const auto& zero_based_account_id = vAccountID - 1;
    if (zero_based_account_id < m_Datas.accounts.size()) {
        double solde = m_CurrentBaseSolde = m_Datas.accounts.at(zero_based_account_id).base_solde;
        const auto& account_number = m_Datas.accounts.at(zero_based_account_id).number;
        DataBase::Instance()->GetTransactions(  //
            vAccountID,                         //
            [this, &solde, account_number](     //
                const RowID& vTransactionID,
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
                t.category = vCategoryName;
                t.optimized[3] = ct::toLower(vCategoryName);
                t.operation = vOperationName;
                t.optimized[4] = ct::toLower(vOperationName);
                t.hash = vHash;
                t.source = vSourceName;
                t.amount = vTransactionAmount;
                t.confirmed = vConfirmed;
                t.solde = solde;
                m_Datas.transactions.push_back(t);
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
            if (m_SelectedTransactions.size() > 1U) {
                if (ImGui::MenuItem("Update selection")) {
                    CTOOL_DEBUG_BREAK;
                    //m_ShowTransactionDialog(DIALOG_UPDATE_ALL, {});
                }
            }
            if (ImGui::MenuItem("Delete selection")) {
                CTOOL_DEBUG_BREAK;
                //m_ShowTransactionDialog(DIALOG_MODE_DELETE, vTransaction);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
}

/*void DataBrokers::m_ShowTransactionDialog(DialogMode vDialogMode, const Transaction& vTransaction) {
    m_showTransactionDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateAccounts();
    m_UpdateCategories();
    m_UpdateOperations();
    if (m_dialogMode & DIALOG_MODE_UPDATE) {
        if (m_dialogMode & DIALOG_MODE_ONCE) {
            m_AccountsCombo.Select(vTransaction.account);
            m_CategoriesCombo.Select(vTransaction.category);
            m_OperationsCombo.Select(vTransaction.operation);
            m_TransactionDateInputText.SetText(vTransaction.date);
            m_TransactionDescriptionInputText.SetText(vTransaction.description);
            m_TransactionCommentInputText.SetText(vTransaction.comment);
            m_TransactionAmountInputDouble = vTransaction.amount;
            m_TransactionsDoublonInputUint = vTransaction.doublons;
            m_TransactionToUpdate = vTransaction;
        } else if (m_dialogMode & DIALOG_MODE_ALL) {
            // we must determine if for each items we have only one value of many ?
            // is one value we can display it
            // if many we cant display if, so just display a word like "many values"

            m_CategoryNameInputText.SetText("");
            m_OperationNameInputText.SetText("");
            m_AccountsCombo.Select(vTransaction.account);
            m_CategoriesCombo.Select(vTransaction.category);
            m_OperationsCombo.Select(vTransaction.operation);
            m_TransactionDateInputText.SetText(vTransaction.date);
            m_TransactionDescriptionInputText.SetText(vTransaction.description);
            m_TransactionCommentInputText.SetText(vTransaction.comment);
            m_TransactionAmountInputDouble = vTransaction.amount;
            m_TransactionsDoublonInputUint = vTransaction.doublons;
            m_TransactionToUpdate = vTransaction;
        }
    } else if (m_dialogMode & DIALOG_MODE_DELETE) {
        m_TransactionToUpdate = vTransaction;
        m_UpdateTransactionsToDelete();
    }
}

void DataBrokers::m_DrawTransactionDialog(const ImVec2& vPos) {
    if (m_showTransactionDialog) {
        if (m_dialogMode & DIALOG_MODE_DELETE) {
            m_DrawTranactionDeletionDialog(vPos);
        } else if (m_dialogMode & DIALOG_MODE_CREATION) {
            m_DrawTranactionCreationDialog(vPos);
        } else if (m_dialogMode & DIALOG_MODE_UPDATE) {
            if (m_dialogMode & DIALOG_MODE_ONCE) {
                m_DrawTranactionUpdateOnceDialog(vPos);
            } else if (m_dialogMode & DIALOG_MODE_ALL) {
                m_DrawTranactionUpdateAllDialog(vPos);
            }
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
        ImGui::Header("Transaction Creation");
        ImGui::Separator();
        const float& align = 125.0f;
        const auto& width = 400.0f;
        m_AccountsCombo.DisplayCombo(width, "Account Number", align);
        m_CategoriesCombo.DisplayCombo(width, "Category", align);
        m_OperationsCombo.DisplayCombo(width, "Operation", align);
        m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
        m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
        m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", false, align);
        m_DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble); });
        m_DisplayAlignedWidget(width, "Nombre de Doublons", align, [this]() {
            if (ImGui::InputInt("##Doublons", &m_TransactionsDoublonInputUint)) {
                if (m_TransactionsDoublonInputUint < 1) {
                    m_TransactionsDoublonInputUint = 1;
                }
            }
        });
        ImGui::Separator();
        if (!m_TransactionDateInputText.empty() && !m_TransactionDescriptionInputText.empty() && m_TransactionAmountInputDouble != 0.0) {
            if (ImGui::ContrastedButton("Ok")) {
                RowID account_id = 0U;
                if (DataBase::Instance()->GetAccount(m_AccountsCombo.GetText(), account_id)) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        const auto hash = ct::toStr(  //
                            "%s_%s_%f",               //
                            m_TransactionDateInputText.GetText().c_str(),
                            // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                            // alors on limite le hash a utiliser un label de 30
                            // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                            m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                            m_TransactionAmountInputDouble);              // must be unique per oepration
                        DataBase::Instance()->AddTransaction(             //
                            account_id,                                   //
                            m_OperationsCombo.GetText(),                  //
                            m_CategoriesCombo.GetText(),                  //
                            m_SourceName,                                 //
                            m_SourceType,                                 //
                            m_TransactionDateInputText.GetText(),         //
                            m_TransactionDescriptionInputText.GetText(),  //
                            m_TransactionCommentInputText.GetText(),      //
                            m_TransactionAmountInputDouble,               //
                            m_TransactionsDoublonInputUint,               //
                            false,                                        //
                            hash);
                        DataBase::Instance()->CloseDBFile();
                    }
                }
                m_showTransactionDialog = false;
                RefreshDatas();
            }
            ImGui::SameLine();
        }
        if (ImGui::ContrastedButton("Cancel")) {
            m_showTransactionDialog = false;
        }
        ImGui::EndPopup();
    }
}

void DataBrokers::m_DrawTranactionUpdateOnceDialog(const ImVec2& vPos) {
    ImGui::OpenPopup("TranactionUpdateOnceDialogPopup");
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("TranactionUpdateOnceDialogPopup",
                               (bool*)nullptr,
                               ImGuiWindowFlags_NoTitleBar |            //
                                   ImGuiWindowFlags_NoResize |          //
                                   ImGuiWindowFlags_AlwaysAutoResize |  //
                                   ImGuiWindowFlags_NoDocking)) {
        ImGui::Header("Transaction Update");
        ImGui::Separator();
        const float& align = 125.0f;
        const auto& width = 400.0f;
        m_DisplayAlignedWidget(width, "Account Number", align, [this]() { ImGui::Text("%s", m_AccountsCombo.GetText().c_str()); });
        m_CategoriesCombo.DisplayCombo(width, "Category", align);
        m_OperationsCombo.DisplayCombo(width, "Operation", align);
        m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
        m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
        m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", false, align);
        m_DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble); });
        m_DisplayAlignedWidget(width, "Nombre de Doublons", align, [this]() {
            if (ImGui::InputInt("##Doublons", &m_TransactionsDoublonInputUint)) {
                if (m_TransactionsDoublonInputUint < 1) {
                    m_TransactionsDoublonInputUint = 1;
                }
            }
        });
        ImGui::Separator();
        if (!m_TransactionDateInputText.empty() && !m_TransactionDescriptionInputText.empty() && m_TransactionAmountInputDouble != 0.0) {
            if (ImGui::ContrastedButton("Ok")) {
                RowID account_id = 0U;
                if (DataBase::Instance()->GetAccount(m_AccountsCombo.GetText(), account_id)) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        const auto hash = ct::toStr(  //
                            "%s_%s_%f",               //
                            m_TransactionDateInputText.GetText().c_str(),
                            // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                            // alors on limite le hash a utiliser un label de 30
                            // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                            m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                            m_TransactionAmountInputDouble);              // must be unique per operation
                        DataBase::Instance()->UpdateTransaction(          //
                            m_TransactionToUpdate.id,                     //
                            m_OperationsCombo.GetText(),                  //
                            m_CategoriesCombo.GetText(),                  //
                            m_SourceName,                                 //
                            m_TransactionDateInputText.GetText(),         //
                            m_TransactionDescriptionInputText.GetText(),  //
                            m_TransactionCommentInputText.GetText(),      //
                            m_TransactionAmountInputDouble,               //
                            m_TransactionsDoublonInputUint,               //
                            false,                                        //
                            hash);
                        DataBase::Instance()->CloseDBFile();
                    }
                }
                m_showTransactionDialog = false;
                RefreshDatas();
            }
            ImGui::SameLine();
        }
        if (ImGui::ContrastedButton("Cancel")) {
            m_showTransactionDialog = false;
        }
        ImGui::EndPopup();
    }
}

void DataBrokers::m_DrawTranactionUpdateAllDialog(const ImVec2& vPos) {
    ImGui::OpenPopup("TranactionUpdateAllDialogPopup");
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("TranactionUpdateAllDialogPopup",
                               (bool*)nullptr,
                               ImGuiWindowFlags_NoTitleBar |            //
                                   ImGuiWindowFlags_NoResize |          //
                                   ImGuiWindowFlags_AlwaysAutoResize |  //
                                   ImGuiWindowFlags_NoDocking)) {
        ImGui::Header("Transaction Update");
        ImGui::Separator();
        const float& align = 125.0f;
        const auto& width = 400.0f;
        m_DisplayAlignedWidget(width, "Account Number", align, [this]() { ImGui::Text("%s", m_AccountsCombo.GetText().c_str()); });
        m_CategoriesCombo.DisplayCombo(width, "Category", align);
        m_OperationsCombo.DisplayCombo(width, "Operation", align);
        m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
        m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
        m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", false, align);
        m_DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble); });
        m_DisplayAlignedWidget(width, "Nombre de Doublons", align, [this]() {
            if (ImGui::InputInt("##Doublons", &m_TransactionsDoublonInputUint)) {
                if (m_TransactionsDoublonInputUint < 1) {
                    m_TransactionsDoublonInputUint = 1;
                }
            }
        });
        ImGui::Separator();
        if (!m_TransactionDateInputText.empty() && !m_TransactionDescriptionInputText.empty() && m_TransactionAmountInputDouble != 0.0) {
            if (ImGui::ContrastedButton("Ok")) {
                RowID account_id = 0U;
                if (DataBase::Instance()->GetAccount(m_AccountsCombo.GetText(), account_id)) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        const auto hash = ct::toStr(  //
                            "%s_%s_%f",               //
                            m_TransactionDateInputText.GetText().c_str(),
                            // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                            // alors on limite le hash a utiliser un label de 30
                            // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                            m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                            m_TransactionAmountInputDouble);              // must be unique per operation
                        DataBase::Instance()->UpdateTransaction(          //
                            m_TransactionToUpdate.id,                     //
                            m_OperationsCombo.GetText(),                  //
                            m_CategoriesCombo.GetText(),                  //
                            m_SourceName,                                 //
                            m_TransactionDateInputText.GetText(),         //
                            m_TransactionDescriptionInputText.GetText(),  //
                            m_TransactionCommentInputText.GetText(),      //
                            m_TransactionAmountInputDouble,               //
                            m_TransactionsDoublonInputUint,               //
                            false,                                        //
                            hash);
                        DataBase::Instance()->CloseDBFile();
                    }
                }
                m_showTransactionDialog = false;
                RefreshDatas();
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
                    { ImGui::Text(t.description.c_str()); }

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

        if (ImGui::ContrastedButton("Confirm")) {
            m_DeleteHoveredOrSelectedRows();
            m_showTransactionDialog = false;
            RefreshDatas();
        }
        ImGui::SameLine();
        if (ImGui::ContrastedButton("Cancel")) {
            m_showTransactionDialog = false;
        }
        ImGui::EndPopup();
    }
}
*/

void DataBrokers::m_drawSearchRow() {
    bool change = false;
    bool reset = false;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    if (ImGui::ContrastedButton("R", nullptr, nullptr, ImGui::GetColumnWidth(0))) {
        reset = true;
    }
    ImGui::PopStyleVar();
    for (size_t idx = 0; idx < 8; ++idx) {
        ImGui::TableNextColumn();
        switch (idx) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4: {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (m_SearchInputTexts.at(idx).DisplayInputText(ImGui::GetColumnWidth(idx), "", "")) {
                    m_SearchTokens[idx] = ct::toLower(m_SearchInputTexts.at(idx).GetText());
                    change = true;
                }
                ImGui::PopStyleVar();
            } break;
            case 5: {
                m_drawAmount(m_TotalDebit);
            } break;
            case 6: {
                m_drawAmount(m_TotalCredit);
            } break;
            case 7: {
                // m_drawAmount(m_CurrentBaseSolde);
            } break;
            case 8: ImGui::Text("[%u]", (uint32_t)m_Datas.transactions_filtered.size()); break;
            default: break;
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
        m_HideByFilledRectForHiddenMode("%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
        ImGui::Text("%.2f", vAmount);
        m_HideByFilledRectForHiddenMode("%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
        m_HideByFilledRectForHiddenMode("%.2f", vAmount);
    }
}
