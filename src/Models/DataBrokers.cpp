#include <Models/DataBrokers.h>

#include <Plugins/PluginManager.h>

#include <ImGuiPack.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <csv/csv.h>

#include <Project/ProjectFile.h>

#include <Models/DataBase.h>

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

std::time_t convertToEpochTime(const std::string& vIsoDateTime, const char* format) {
    struct std::tm time = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::istringstream ss(vIsoDateTime);
    ss >> std::get_time(&time, format);
    if (ss.fail()) {
        std::cerr << "ERROR: Cannot parse date string (" << vIsoDateTime << "); required format %Y-%m-%d" << std::endl;
        exit(1);
    }
    time.tm_hour = 0;
    time.tm_min = 0;
    time.tm_sec = 0;
#ifdef _WIN32
    return _mkgmtime(&time);
#else
    return timegm(&time);
#endif
}

std::string convertToISO8601(const std::time_t& vEpochTime) {
    auto tp = std::chrono::system_clock::from_time_t(vEpochTime);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    auto* timeinfo = std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%Y-%m-%d");
    return oss.str();
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
            m_drawRefreshMenu(vFrameActionSystem);
            m_drawCreationMenu(vFrameActionSystem);
            m_drawUpdateMenu(vFrameActionSystem);
            m_drawImportMenu(vFrameActionSystem);
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

            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(a.name.c_str(), m_SelectedAccountIdx == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                m_UpdateTransactions(a.id);
                m_SelectedAccountIdx = idx;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(a.bank.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(a.type.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text(a.name.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text(a.number.c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%u", a.count);

            ++idx;
        }
        ImGui::EndTable();
    }
}

void DataBrokers::DisplayTransactions() {
    ImGui::Header("Transactions");
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##Transactions", 8, flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Solde", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        const float& bar_column_width = 50.0f;
        double max_price = DBL_MIN;
        double min_price = DBL_MAX;
        auto drawListPtr = ImGui::GetWindowDrawList();
        const float text_h = ImGui::GetTextLineHeight();
        const float item_h = ImGui::GetTextLineHeightWithSpacing();
        m_TransactionsListClipper.Begin((int)m_Datas.transactions.size(), item_h);
        while (m_TransactionsListClipper.Step()) {
            max_price = 0.0;
            for (int i = m_TransactionsListClipper.DisplayStart; i < m_TransactionsListClipper.DisplayEnd; i++) {
                if (i < 0) {
                    continue;
                }
                const auto& s = std::abs(m_Datas.soldes.at(i));
                if (s > max_price) {
                    max_price = s;
                }
            }
            for (int i = m_TransactionsListClipper.DisplayStart; i < m_TransactionsListClipper.DisplayEnd; i++) {
                if (i < 0) {
                    continue;
                }

                const auto& t = m_Datas.transactions.at(i);
                const auto& s = m_Datas.soldes.at(i);

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text(t.date.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(t.desc.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text(t.category.c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text(t.operation.c_str());

                ImGui::TableSetColumnIndex(4);
                if (t.amount < 0.0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                    ImGui::Text("%.2f", t.amount);
                    ImGui::PopStyleColor();
                }

                ImGui::TableSetColumnIndex(5);
                if (t.amount >= 0.0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                    ImGui::Text("%.2f", t.amount);
                    ImGui::PopStyleColor();
                }

                ImGui::TableSetColumnIndex(6);
                if (s < 0.0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                    ImGui::Text("%.2f", s);
                    ImGui::PopStyleColor();
                } else if (s > 0.0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                    ImGui::Text("%.2f", s);
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Text("%.2f", s);
                }

                ImGui::TableSetColumnIndex(7);
                auto cursor = ImGui::GetCursorScreenPos();

                ImVec2 pMin(cursor.x, cursor.y + text_h * 0.25f);
                ImVec2 pMax(cursor.x + bar_column_width, cursor.y + text_h * 0.75f);
                float pMidX = (pMin.x + pMax.x) * 0.5f;
                ImU32 color = ImGui::ColorConvertFloat4ToU32((ImVec4)ImColor::HSV((i % 7) / 7.0f, 1.0f, 1.0f));

                ImGui::SetCursorScreenPos(pMin);
                float bw = bar_column_width * 0.5f * std::abs(s) / (float)max_price;
                if (s < 0.0) {
                    drawListPtr->AddRectFilled(ImVec2(pMidX - bw, pMin.y), ImVec2(pMidX, pMax.y), bad_color);
                } else if (s > 0.0) {
                    drawListPtr->AddRectFilled(ImVec2(pMidX, pMin.y), ImVec2(pMidX + bw, pMax.y), good_color);
                }
                ImGui::SetCursorScreenPos(pMax);
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

void DataBrokers::m_drawRefreshMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::MenuItem("Refresh")) {
        m_refreshDatas();
    }
}

void DataBrokers::m_drawCreationMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("User")) {
            m_ShowUserDialog(DialogMode::CREATION);
        }
        if (ImGui::MenuItem("Bank")) {
            m_ShowBankDialog(DialogMode::CREATION);
        }
        if (ImGui::MenuItem("Account")) {
            m_ShowAccountDialog(DialogMode::CREATION);
        }
        if (ImGui::MenuItem("Category")) {
            m_ShowCategoryDialog(DialogMode::CREATION);
        }
        if (ImGui::MenuItem("Operation")) {
            m_ShowOperationDialog(DialogMode::CREATION);
        }
        if (ImGui::MenuItem("Transaction")) {
            m_ShowTransactionDialog(DialogMode::CREATION);
        }
        ImGui::EndMenu();
    }
}

void DataBrokers::m_drawUpdateMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Update")) {
        if (ImGui::MenuItem("User")) {
            m_ShowUserDialog(DialogMode::UPDATE);
        }
        if (ImGui::MenuItem("Bank")) {
            m_ShowBankDialog(DialogMode::UPDATE);
        }
        if (ImGui::MenuItem("Account")) {
            m_ShowAccountDialog(DialogMode::UPDATE);
        }
        if (ImGui::MenuItem("Category")) {
            m_ShowCategoryDialog(DialogMode::UPDATE);
        }
        if (ImGui::MenuItem("Operation")) {
            m_ShowOperationDialog(DialogMode::UPDATE);
        }
        if (ImGui::MenuItem("Transaction")) {
            m_ShowTransactionDialog(DialogMode::UPDATE);
        }
        ImGui::EndMenu();
    }
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
    // msut be cleared even if they are some smart pointers
    // since the pointers come from a plugin loaded memory
    // when the plugin is destoryed, an automatic reset of weak pointer will cause a crash
    // the same for all pointers type who are coming from the plugin.
    // so we must do this manually here
    // pointer on a internal plugin data
    // thoses contain weak ptrs
    m_SelectedBroker.reset();
    // this dico save plugin ptr, we clear it at end
    m_DataBrokerModules.clear();
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
                                    s.category, s.operation, s.date, s.label, s.comment, s.amount, s.hash);
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
        });
    m_AccountsCombo = ImWidgets::QuickStringCombo(0, m_Datas.accountNumbers);
    if (m_SelectedAccountIdx < m_Datas.accounts.size()) {
        m_UpdateTransactions(m_Datas.accounts.at(m_SelectedAccountIdx).id);
    }
}

void DataBrokers::m_ShowAccountDialog(const DialogMode& vDialogMode) {
    m_showAccountDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateUsers();
    m_UpdateBanks();
}

void DataBrokers::m_DrawAccountDialog(const ImVec2& vPos) {
    if (m_showAccountDialog) {
        ImGui::OpenPopup("AccountCreationModalPopup");
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("AccountCreationModalPopup",
                                   (bool*)nullptr,
                                   ImGuiWindowFlags_NoTitleBar |            //
                                       ImGuiWindowFlags_NoResize |          //
                                       ImGuiWindowFlags_AlwaysAutoResize |  //
                                       ImGuiWindowFlags_NoDocking)) {
            ImGui::Header("Account Creation");
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
                        DataBase::Instance()->AddAccount(      //
                            m_UsersCombo.GetText(),            //
                            m_BanksCombo.GetText(),            //
                            m_AccountTypeInputText.GetText(),  //
                            m_AccountNameInputText.GetText(),  //
                            m_AccountNumberInputText.GetText(),
                            m_AccountBaseSoldeInputDouble);
                        DataBase::Instance()->CloseDBFile();
                    }
                    m_showAccountDialog = false;
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

void DataBrokers::m_UpdateTransactions(const RowID& vAccountID) {
    m_Datas.transactions.clear();
    m_Datas.soldes.clear();
    const auto& zero_based_account_id = vAccountID - 1;
    if (zero_based_account_id < m_Datas.accounts.size()) {
        auto solde = m_Datas.accounts.at(zero_based_account_id).base_solde;
        DataBase::Instance()->GetTransactions(  //
            vAccountID,                         //
            [this, &solde](const TransactionDate& vTransactionDate,
                           const TransactionDescription& vTransactionDescription,
                           const TransactionComment& vTransactionComment,
                           const CategoryName& vCategoryName,
                           const OperationName& vOperationName,
                           const TransactionAmount& vTransactionAmount) {  //
                Transaction t;
                t.date = vTransactionDate;
                t.desc = vTransactionDescription;
                t.comm = vTransactionComment;
                t.category = vCategoryName;
                t.operation = vOperationName;
                t.amount = vTransactionAmount;
                m_Datas.transactions.push_back(t);
                solde += t.amount;
                m_Datas.soldes.push_back(solde);
            });
    }
}

void DataBrokers::m_ShowTransactionDialog(const DialogMode& vDialogMode) {
    m_showTransactionDialog = true;
    m_dialogMode = vDialogMode;
    m_UpdateAccounts();
    m_UpdateCategories();
    m_UpdateOperations();
}

void DataBrokers::m_DrawTransactionDialog(const ImVec2& vPos) {
    if (m_showTransactionDialog) {
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
            float px = ImGui::GetCursorPosX();
            ImGui::Text("Amount");
            ImGui::SameLine(align);
            const float w = width - (ImGui::GetCursorPosX() - px);
            ImGui::PushID(++ImGui::CustomStyle::pushId);
            ImGui::PushItemWidth(w);
            ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble);
            ImGui::PopItemWidth();
            ImGui::PopID();

            ImGui::Separator();
            if (!m_TransactionDateInputText.empty() && !m_TransactionDescriptionInputText.empty() && m_TransactionAmountInputDouble != 0.0) {
                if (ImGui::ContrastedButton("Ok")) {
                    RowID account_id = 0U;
                    if (DataBase::Instance()->GetAccount(m_AccountsCombo.GetText(), account_id)) {
                        if (DataBase::Instance()->OpenDBFile()) {
                            DataBase::Instance()->AddTransaction(             //
                                account_id,                                   //
                                m_CategoriesCombo.GetText(),                  //
                                m_OperationsCombo.GetText(),                  //
                                m_TransactionDateInputText.GetText(),         //
                                m_TransactionDescriptionInputText.GetText(),  //
                                m_TransactionCommentInputText.GetText(),      //
                                m_TransactionAmountInputDouble,
                                "");
                            DataBase::Instance()->CloseDBFile();
                        }
                    }
                    m_showTransactionDialog = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showTransactionDialog = false;
            }
            ImGui::EndPopup();
        }
    }
}
