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
    m_DrawUserCreationDialog(center);
    m_DrawBankCreationDialog(center);
    m_DrawCategoryCreationDialog(center);
    m_DrawOperationCreationDialog(center);
    m_DrawAccountCreationDialog(center);
    m_DrawTransactionCreationDialog(center);
    if (ImGuiFileDialog::Instance()->Display("Import Datas")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            /*auto files = ImGuiFileDialog::Instance()->GetSelection();
            ImportTypeEnum pType = (ImportTypeEnum)(uintptr_t)ImGuiFileDialog::Instance()->GetUserDatas();
            for (const auto& file : files) {
                m_ImportFromFiles(file.second, pType);
            }
            m_RefreshSymbols();*/
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void DataBrokers::drawMenu(FrameActionSystem& vFrameActionSystem) {
    if (ProjectFile::Instance()->IsProjectLoaded()) {
        if (ImGui::BeginMenuBar()) {
            m_drawRefreshMenu(vFrameActionSystem);
            m_drawCreationMenu(vFrameActionSystem);
            m_drawImportMenu(vFrameActionSystem);
            ImGui::EndMenuBar();
        }
    }
}

void DataBrokers::DisplayAccounts() {
    ImGui::Header("Accounts");
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("##Accounts", 6, flags)) {
        ImGui::TableSetupColumn("Users");
        ImGui::TableSetupColumn("Banks");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Number");
        ImGui::TableSetupColumn("Load");
        ImGui::TableHeadersRow();
        for (const auto& a : m_Datas.accounts) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(a.user.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(a.bank.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(a.type.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text(a.name.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text(a.number.c_str());

            ImGui::TableSetColumnIndex(5);
            if (ImGui::SmallContrastedButton(">")) {
                m_UpdateTransactions(a.id);
            }
        }
        ImGui::EndTable();
    }
}

void DataBrokers::DisplayTransactions() {
    ImGui::Header("Transactions");
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
    if (ImGui::BeginTable("##Transactions", 6, flags)) {
        ImGui::TableSetupColumn("Dates");
        ImGui::TableSetupColumn("Descriptions");
        ImGui::TableSetupColumn("Category");
        ImGui::TableSetupColumn("Operation");
        ImGui::TableSetupColumn("Credit");
        ImGui::TableSetupColumn("Debit");
        ImGui::TableHeadersRow();
        for (const auto& t : m_Datas.transactions) {
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
            if (t.amount >= 0.0) {
                ImGui::Text("%f", t.amount);
            }

            ImGui::TableSetColumnIndex(5);
            if (t.amount < 0.0) {
                ImGui::Text("%f", t.amount);
            }
        }
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
            m_ShowUserCreationDialog();
        }
        if (ImGui::MenuItem("Bank")) {
            m_ShowBankCreationDialog();
        }
        if (ImGui::MenuItem("Account")) {
            m_ShowAccountCreationDialog();
        }
        if (ImGui::MenuItem("Category")) {
            m_ShowCategoryCreationDialog();
        }
        if (ImGui::MenuItem("Operation")) {
            m_ShowOperationCreationDialog();
        }
        if (ImGui::MenuItem("Transaction")) {
            m_ShowTransactionCreationDialog();
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

void DataBrokers::m_UpdateUsers() {
    m_Datas.userNames.clear();
    DataBase::Instance()->GetUsers(          //
        [this](const UserName& vUserName) {  //
            m_Datas.userNames.push_back(vUserName);
        });
    m_UsersCombo = ImWidgets::QuickStringCombo(0, m_Datas.userNames);
}

void DataBrokers::m_ShowUserCreationDialog() {
    m_showUserCreation = true;
}

void DataBrokers::m_DrawUserCreationDialog(const ImVec2& vPos) {
    if (m_showUserCreation) {
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
                    m_showUserCreation = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showUserCreation = false;
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

void DataBrokers::m_ShowBankCreationDialog() {
    m_showBankCreation = true;
}

void DataBrokers::m_DrawBankCreationDialog(const ImVec2& vPos) {
    if (m_showBankCreation) {
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
                    m_showBankCreation = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showBankCreation = false;
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

void DataBrokers::m_ShowCategoryCreationDialog() {
    m_showCategoryCreation = true;
    m_UpdateCategories();
}

void DataBrokers::m_DrawCategoryCreationDialog(const ImVec2& vPos) {
    if (m_showCategoryCreation) {
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
                    m_showCategoryCreation = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showCategoryCreation = false;
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

void DataBrokers::m_ShowOperationCreationDialog() {
    m_showOperationCreation = true;
    m_UpdateOperations();
}

void DataBrokers::m_DrawOperationCreationDialog(const ImVec2& vPos) {
    if (m_showOperationCreation) {
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
                    m_showOperationCreation = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showOperationCreation = false;
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
               const AccountNumber& vAccountNumber) {  //
            Account a;
            a.id = vRowID;
            a.user = vUserName;
            a.bank = vBankName;
            a.type = vAccountType;
            a.name = vAccountName;
            a.number = vAccountNumber;
            m_Datas.accounts.push_back(a);
            m_Datas.accountNumbers.push_back(vAccountNumber);
        });
    m_AccountsCombo = ImWidgets::QuickStringCombo(0, m_Datas.accountNumbers);
}

void DataBrokers::m_ShowAccountCreationDialog() {
    m_showAccountCreation = true;
    m_UpdateUsers();
    m_UpdateBanks();
}

void DataBrokers::m_DrawAccountCreationDialog(const ImVec2& vPos) {
    if (m_showAccountCreation) {
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
            ImGui::Separator();
            if (!m_AccountNameInputText.empty() && !m_AccountTypeInputText.empty() && !m_AccountNumberInputText.empty()) {
                if (ImGui::ContrastedButton("Ok")) {
                    if (DataBase::Instance()->OpenDBFile()) {
                        DataBase::Instance()->AddAccount(      //
                            m_UsersCombo.GetText(),            //
                            m_BanksCombo.GetText(),            //
                            m_AccountTypeInputText.GetText(),  //
                            m_AccountNameInputText.GetText(),  //
                            m_AccountNumberInputText.GetText());
                        DataBase::Instance()->CloseDBFile();
                    }
                    m_showAccountCreation = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showAccountCreation = false;
            }
            ImGui::EndPopup();
        }
    }
}

void DataBrokers::m_UpdateTransactions(const RowID& vAccountID) {
    m_Datas.transactions.clear();
    DataBase::Instance()->GetTransactions(  //
        vAccountID,                         //
        [this](const TransactionDate& vTransactionDate,
               const TransactionDescription& vTransactionDescription,
               const CategoryName& vCategoryName,
               const OperationName& vOperationName,
               const TransactionAmount& vTransactionAmount) {  //
            Transaction t;
            t.date = vTransactionDate;
            t.desc = vTransactionDescription;
            t.category = vCategoryName;
            t.operation = vOperationName;
            t.amount = vTransactionAmount;
            m_Datas.transactions.push_back(t);
        });
}

void DataBrokers::m_ShowTransactionCreationDialog() {
    m_showTransactionCreation = true;
    m_UpdateAccounts();
    m_UpdateCategories();
    m_UpdateOperations();
}

void DataBrokers::m_DrawTransactionCreationDialog(const ImVec2& vPos) {
    if (m_showTransactionCreation) {
        ImGui::OpenPopup("AccountTransactionModalPopup");
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("AccountTransactionModalPopup",
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
                                m_TransactionAmountInputDouble);
                            DataBase::Instance()->CloseDBFile();
                        }
                    }
                    m_showTransactionCreation = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_showTransactionCreation = false;
            }
            ImGui::EndPopup();
        }
    }
}
