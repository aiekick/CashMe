// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Account Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/AccountPane.h>

#include <cinttypes>  // printf zu

#include <Models/DataBase.h>
#include <Project/ProjectFile.h>
#include <Frontend/MainFrontend.h>
#include <Plugins/PluginManager.h>
#include <Panes/StatsPane.h>
#include <Systems/SettingsDialog.h>

AccountPane::AccountPane() = default;
AccountPane::~AccountPane() {
    Unit();
}

bool AccountPane::Init() {
    bool ret = true;
    m_getAvailableDataBrokers();
    ret &= m_BankDialog.init();
    ret &= m_AccountDialog.init();
    ret &= m_CategoryDialog.init();
    ret &= m_OperationDialog.init();
    ret &= m_TransactionsTable.Init();
    return ret;
}

void AccountPane::Unit() {
    m_BankDialog.init();
    m_AccountDialog.init();
    m_CategoryDialog.init();
    m_OperationDialog.init();
    m_TransactionsTable.Unit();
    m_clear();
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
                if (ImGui::BeginMenuBar()) {
                    auto& actionSystemRef = MainFrontend::Instance()->GetActionSystemRef();
                    m_TransactionsTable.drawAccountsMenu(actionSystemRef);
                    m_drawCreationMenu();
                    m_drawImportMenu(actionSystemRef);
                    m_TransactionsTable.drawSelectMenu(actionSystemRef);
                    m_TransactionsTable.drawGroupingMenu(actionSystemRef);
                    m_TransactionsTable.drawDebugMenu(actionSystemRef);
                    ImGui::EndMenuBar();
                }
                m_TransactionsTable.draw(ImGui::GetContentRegionAvail());
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
        StatsPane::Instance()->Load();
        ret = true;
    }
    ret |= m_CategoryDialog.draw(center);
    ret |= m_OperationDialog.draw(center);
    ret |= m_TransactionsTable.getTransactionDialogRef().draw(center);
    
    m_ImportThread.drawDialog(center);

    if (ret) {
        m_TransactionsTable.refreshDatas();
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
                m_importFromFiles(files);
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
    m_TransactionsTable.refreshDatas();
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

void AccountPane::m_drawCreationMenu() {
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
            m_TransactionsTable.getTransactionDialogRef().show(DataDialogMode::MODE_CREATION);
        }
        ImGui::EndMenu();
    }
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

void AccountPane::m_importFromFiles(const std::vector<std::string>& vFiles) {
    m_ImportThread.start(  //
        "Import Datas",
        m_SelectedBroker,
        vFiles,
        [this]() {
            m_TransactionsTable.refreshDatas();
            m_TransactionsTable.refreshFiltering();
        },
        nullptr);
}

void AccountPane::m_getAvailableDataBrokers() {
    m_clear();
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

void AccountPane::m_clear() {
    m_SelectedBroker.reset();  // must be reset before quit since point on the memroy of a plugin
    m_DataBrokerModules.clear();
    m_TransactionsTable.clear();
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
