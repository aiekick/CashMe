// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static AccountOutput Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/StatementsPane.h>

#include <cinttypes>  // printf zu

#include <Models/DataBase.h>
#include <Project/ProjectFile.h>
#include <Frontend/MainFrontend.h>
#include <Plugins/PluginManager.h>
#include <Panes/StatsPane.h>
#include <Systems/SettingsDialog.h>
#include <Panes/IncomesPane.h>
#include <Panes/AccountsPane.h>

StatementsPane::StatementsPane() = default;
StatementsPane::~StatementsPane() {
    Unit();
}

bool StatementsPane::Init() {
    bool ret = true;
    m_getAvailableDataBrokers();
    ret &= m_BankDialog.init();
    ret &= m_EntityDialog.init();
    ret &= m_AccountDialog.init();
    ret &= m_CategoryDialog.init();
    ret &= m_OperationDialog.init();
    ret &= m_TransactionsTable.init();
    return ret;
}

void StatementsPane::Unit() {
    m_BankDialog.unit();
    m_EntityDialog.unit();
    m_AccountDialog.unit();
    m_CategoryDialog.unit();
    m_OperationDialog.unit();
    m_TransactionsTable.unit();
    m_clear();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool StatementsPane::DrawPanes(const uint32_t& /*vCurrentFrame*/, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
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
                    m_TransactionsTable.drawMenu();
                    ImGui::EndMenuBar();
                }
                m_TransactionsTable.draw(ImGui::GetContentRegionAvail());
            }
        }

        ImGui::End();
    }
    return change;
}

bool StatementsPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool StatementsPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& vRect, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    const ImVec2 center = vRect.GetCenter();

    bool ret = false;

    ret |= m_BankDialog.draw(center);
    if (m_AccountDialog.draw(center)) {
        StatsPane::Instance()->Load();
        ret = true;
    }
    ret |= m_EntityDialog.draw(center);
    ret |= m_CategoryDialog.draw(center);
    ret |= m_OperationDialog.draw(center);
    ret |= m_TransactionsTable.getTransactionDialogRef().draw(center);
    
    m_ImportThread.drawDialog(center);

    if (ret) {
        m_TransactionsTable.refreshDatas();
        AccountsPane::Instance()->Load();
    }

    if (m_TransactionsTable.getIncomeDialogRef().draw(center)) {
        IncomesPane::Instance()->Load();
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

bool StatementsPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void StatementsPane::DoBackend() {
    m_ImportThread.finishIfNeeded();
}

void StatementsPane::Load() {
    m_TransactionsTable.refreshDatas();
}

void StatementsPane::m_drawCreationMenu() {
    if (ImGui::BeginMenu("Add")) {
        if (ImGui::MenuItem("Bank")) {
            m_BankDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Account")) {
            m_AccountDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("EntityOutput")) {
            m_EntityDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("CategoryOutput")) {
            m_CategoryDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("OperationOutput")) {
            m_OperationDialog.show(DataDialogMode::MODE_CREATION);
        }
        if (ImGui::MenuItem("Transaction")) {
            m_TransactionsTable.getTransactionDialogRef().show(DataDialogMode::MODE_CREATION);
        }
        ImGui::EndMenu();
    }
}

void StatementsPane::m_drawImportMenu(FrameActionSystem& vFrameActionSystem) {
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

void StatementsPane::m_importFromFiles(const std::vector<std::string>& vFiles) {
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

void StatementsPane::m_getAvailableDataBrokers() {
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

void StatementsPane::m_clear() {
    m_SelectedBroker.reset();  // must be reset before quit since point on the memroy of a plugin
    m_DataBrokerModules.clear();
    m_TransactionsTable.clear();
}

void StatementsPane::m_drawAccountMenu(const AccountOutput& vAccount) {
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
