/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "MainFrontend.h"

#include <imguipack.h>

#include <Backend/MainBackend.h>

#include <Project/ProjectFile.h>

#include <Plugins/PluginManager.h>

#include <Models/DataBase.h>

#include <Panes/BanksPane.h>
#include <Panes/BudgetPane.h>
#include <Panes/IncomesPane.h>
#include <Panes/ConsolePane.h>
#include <Panes/AccountsPane.h>
#include <Panes/EntitiesPane.h>
#include <Panes/CategoriesPane.h>
#include <Panes/OperationsPane.h>
#include <Panes/TransactionsPane.h>

#include <Systems/SettingsDialog.h>

#include <Helpers/TranslationHelper.h>

// panes
#define DEBUG_PANE_ICON ICON_SDFM_BUG
#define SCENE_PANE_ICON ICON_SDFM_FORMAT_LIST_BULLETED_TYPE
#define TUNING_PANE_ICON ICON_SDFM_TUNE
#define CONSOLE_PANE_ICON ICON_SDFMT_COMMENT_TEXT_MULTIPLE

// features
#define GRID_ICON ICON_SDFMT_GRID
#define MOUSE_ICON ICON_SDFMT_MOUSE
#define CAMERA_ICON ICON_SDFMT_CAMCORDER
#define GIZMO_ICON ICON_SDFMT_AXIS_ARROW

using namespace std::placeholders;

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainFrontend::sCentralWindowHovered = false;

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainFrontend::~MainFrontend() = default;

bool MainFrontend::init() {
    m_build_themes();

    ImGuiThemeHelper::initSingleton();
    ImGuiFileDialog::initSingleton();
    TranslationHelper::initSingleton();

    AccountsPane::initSingleton();
    BanksPane::initSingleton();
    BudgetPane::initSingleton();
    CategoriesPane::initSingleton();
    ConsolePane::initSingleton();
    EntitiesPane::initSingleton();
    IncomesPane::initSingleton();
    OperationsPane::initSingleton();
    TransactionsPane::initSingleton();

    LayoutManager::ref().Init("Layouts", "Default Layout");

    LayoutManager::ref().SetPaneDisposalRatio("LEFT", 0.25f);
    LayoutManager::ref().SetPaneDisposalRatio("RIGHT", 0.25f);
    LayoutManager::ref().SetPaneDisposalRatio("BOTTOM", 0.25f);

    // Views
    LayoutManager::ref().AddPane(TransactionsPane::ref(), "Transactions", "", "CENTRAL", 0.0f, true, true);
    LayoutManager::ref().AddPane(BudgetPane::ref(), "Budget", "", "CENTRAL", 0.0f, false, false);
    LayoutManager::ref().AddPane(ConsolePane::ref(), "Console", "", "BOTTOM", 0.25f, false, false);
    
    // Maintenance
    LayoutManager::ref().AddPane(BanksPane::ref(), "Banks", "Maintenance", "CENTRAL", 0.0f, false, false);
    LayoutManager::ref().AddPane(AccountsPane::ref(), "Accounts", "Maintenance", "CENTRAL", 0.0f, false, false);
    LayoutManager::ref().AddPane(EntitiesPane::ref(), "Entities", "Maintenance", "CENTRAL", 0.0f, false, false);
    LayoutManager::ref().AddPane(CategoriesPane::ref(), "Categories", "Maintenance", "CENTRAL", 0.0f, false, false);
    LayoutManager::ref().AddPane(OperationsPane::ref(), "Operations", "Maintenance", "CENTRAL", 0.0f, false, false);
    LayoutManager::ref().AddPane(IncomesPane::ref(), "Incomes", "Maintenance", "CENTRAL", 0.0f, false, false);
    
    // InitPanes is done in m_InitPanes, because a specific order is needed

    bool ret = true;
    ret &= m_BankDialog.init();
    ret &= m_EntityDialog.init();
    ret &= m_AccountDialog.init();
    ret &= m_CategoryDialog.init();
    ret &= m_OperationDialog.init();
    if (LayoutManager::ref().InitPanes()) {
        // a faire apres InitPanes() sinon ConsolePane::ref()->paneFlag vaudra 0 et changeras apres InitPanes()
        Messaging::ref().sMessagePaneId = ConsolePane::ref()->GetFlag();
        ret &= true;
    }
    return ret;
}

void MainFrontend::unit() {
    m_BankDialog.unit();
    m_EntityDialog.unit();
    m_AccountDialog.unit();
    m_CategoryDialog.unit();
    m_OperationDialog.unit();

    LayoutManager::ref().UnitPanes();
    const auto& pluginPanes = PluginManager::ref().GetPluginPanes();
    for (const auto& pluginPane : pluginPanes) {
        if (!pluginPane.pane.expired()) {
            LayoutManager::ref().RemovePane(pluginPane.name);
        }
    }

    ImGuiThemeHelper::unitSingleton();
    ImGuiFileDialog::unitSingleton();
    TranslationHelper::unitSingleton();

    AccountsPane::unitSingleton();
    BanksPane::unitSingleton();
    BudgetPane::unitSingleton();
    CategoriesPane::unitSingleton();
    ConsolePane::unitSingleton();
    EntitiesPane::unitSingleton();
    IncomesPane::unitSingleton();
    OperationsPane::unitSingleton();
    TransactionsPane::unitSingleton();
}

bool MainFrontend::isValid() const {
    return false;
}

bool MainFrontend::isThereAnError() const {
    return false;
}

void MainFrontend::Display(const uint32_t& vCurrentFrame, const ImRect& vRect) {
    const auto context_ptr = ImGui::GetCurrentContext();
    if (context_ptr != nullptr) {
        const auto& io = ImGui::GetIO();

        m_DisplayRect = vRect;

        MainFrontend::sCentralWindowHovered = (ImGui::GetCurrentContext()->HoveredWindow == nullptr);
        ImGui::CustomStyle::ResetCustomId();

        m_drawMainMenuBar();
        m_drawMainStatusBar();

        if (LayoutManager::ref().BeginDockSpace(ImGuiDockNodeFlags_PassthruCentralNode)) {
            /*if (MainBackend::ref().GetBackendDatasRef().canWeTuneGizmo) {
                const auto viewport = ImGui::GetMainViewport();
                ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
                ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                ImRect rc(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                DrawOverlays(vCurrentFrame, rc, context_ptr, {});
            }*/
            LayoutManager::ref().EndDockSpace();
        }

        if (LayoutManager::ref().DrawPanes(vCurrentFrame, context_ptr, {})) {
            ProjectFile::ref()->SetProjectChange();
        }

        DrawDialogsAndPopups(vCurrentFrame, m_DisplayRect, context_ptr, {});

        ImGuiThemeHelper::ref().Draw();
        LayoutManager::ref().InitAfterFirstDisplay(io.DisplaySize);
    }
}

bool MainFrontend::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    m_ActionSystem.RunActions();
    LayoutManager::ref().DrawDialogsAndPopups(vCurrentFrame, vRect, vContextPtr, vUserDatas);
    if (m_ShowImGui) {
        ImGui::ShowDemoWindow(&m_ShowImGui);
    }
    if (m_ShowImPlot) {
        ImPlot::ShowDemoWindow(&m_ShowImPlot);
    }
    if (m_ShowMetric) {
        ImGui::ShowMetricsWindow(&m_ShowMetric);
    }
    SettingsDialog::ref().Draw();
    ImVec2 max = vRect.GetSize();
    ImVec2 min = max * 0.5f;
    if (ImGuiFileDialog::ref().Display("Import Datas", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::ref().IsOk()) {
            const auto& selection = ImGuiFileDialog::ref().GetSelection();
            if (!selection.empty()) {
                std::vector<std::string> files;
                for (const auto& s : selection) {
                    files.push_back(s.second);
                }
                MainBackend::ref().importFromFiles(files);
            }
        }
        ImGuiFileDialog::ref().Close();
    }
    const auto center = vRect.GetCenter();
    MainBackend::ref().getImportWorkerThreadRef().drawDialog(center);

    const auto bankChange = MainFrontend::ref().getBankDialogRef().draw(center);
    const auto accountChange = MainFrontend::ref().getAccountDialogRef().draw(center);
    const auto entityChange = MainFrontend::ref().getEntityDialogRef().draw(center);
    const auto incomeChange = MainFrontend::ref().getIncomeDialogRef().draw(center);
    const auto categoryChange = MainFrontend::ref().getCategoryDialogRef().draw(center);
    const auto operationChange = MainFrontend::ref().getOperationDialogRef().draw(center);
    const auto transactionChange = MainFrontend::ref().getTransactionDialogRef().draw(center);

    if (bankChange || accountChange) {
        AccountsPane::ref()->Init();
        BanksPane::ref()->Init();
        BudgetPane::ref()->Init();
        CategoriesPane::ref()->Init();
        ConsolePane::ref()->Init();
        EntitiesPane::ref()->Init();
        OperationsPane::ref()->Init();
        TransactionsPane::ref()->Init();
        IncomesPane::ref()->Init();
    } else if (entityChange) {
        EntitiesPane::ref()->Init();
        TransactionsPane::ref()->Init();
        IncomesPane::ref()->Init();
    } else if (categoryChange) {
        CategoriesPane::ref()->Init();
        TransactionsPane::ref()->Init();
        IncomesPane::ref()->Init();
    } else if (operationChange) {
        OperationsPane::ref()->Init();
        TransactionsPane::ref()->Init();
        IncomesPane::ref()->Init();
    } else if (incomeChange) {
        BudgetPane::ref()->Init();
        IncomesPane::ref()->Init();
        TransactionsPane::ref()->Init();
    } else if (transactionChange) {
        TransactionsPane::ref()->Init();
    }

    return false;
}

void MainFrontend::OpenAboutDialog() {
    m_ShowAboutDialog = true;
}

BankDialog& MainFrontend::getBankDialogRef() {
    return m_BankDialog;
}

AccountDialog& MainFrontend::getAccountDialogRef() {
    return m_AccountDialog;
}

EntityDialog& MainFrontend::getEntityDialogRef() {
    return m_EntityDialog;
}

CategoryDialog& MainFrontend::getCategoryDialogRef() {
    return m_CategoryDialog;
}

OperationDialog& MainFrontend::getOperationDialogRef() {
    return m_OperationDialog;
}

IncomeDialog& MainFrontend::getIncomeDialogRef() {
    return m_IncomeDialog;
}

TransactionDialog& MainFrontend::getTransactionDialogRef() {
    return m_TransactionDialog;
}

FrameActionSystem& MainFrontend::GetActionSystemRef() {
    return m_ActionSystem;
}

void MainFrontend::IWantToCloseTheApp() {
    Action_Window_CloseApp();
}

void MainFrontend::JustDropFiles(int count, const char** paths) {
    assert(0);

    std::map<std::string, std::string> dicoFont;
    std::string prj;

    for (int i = 0; i < count; ++i) {
        // file
        auto f = std::string(paths[i]);

        // lower case
        auto f_opt = f;
        for (auto& c : f_opt)
            c = (char)std::tolower((int)c);

        // well known extention
        if (f_opt.find(".ttf") != std::string::npos     // truetype (.ttf)
            || f_opt.find(".otf") != std::string::npos  // opentype (.otf)
            //||	f_opt.find(".ttc") != std::string::npos		// ttf/otf collection for futur (.ttc)
        ) {
            dicoFont[f] = f;
        }
        if (f_opt.find(PROJECT_EXT) != std::string::npos) {
            prj = f;
        }
    }

    // priority to project file
    if (!prj.empty()) {
        MainBackend::ref().NeedToLoadProject(prj);
    }
}

ez::xml::Nodes MainFrontend::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    node.addChilds(ImGuiThemeHelper::ref().getXmlNodes("app"));
    node.addChilds(LayoutManager::ref().getXmlNodes("app"));
    node.addChilds(BudgetPane::ref()->getXmlNodes("app"));
    node.addChild("places").setContent(ImGuiFileDialog::ref().SerializePlaces());
    return node.getChildren();
}

bool MainFrontend::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    const auto& strName = vNode.getName();
    const auto& strValue = vNode.getContent();
    const auto& strParentName = vParent.getName();

    if (strName == "places") {
        ImGuiFileDialog::ref().DeserializePlaces(strValue);
    }

    ImGuiThemeHelper::ref().setFromXmlNodes(vNode, vParent, "app");
    LayoutManager::ref().setFromXmlNodes(vNode, vParent, "app");
    BudgetPane::ref()->setFromXmlNodes(vNode, vParent, "app");
    return true;
}

void MainFrontend::m_drawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(" Project")) {
            if (ImGui::MenuItem(" New")) {
                Action_Menu_NewProject();
            }

            if (ImGui::MenuItem(" Open")) {
                Action_Menu_OpenProject();
            }

            if (ProjectFile::ref()->IsProjectLoaded()) {
                ImGui::Separator();

                if (ImGui::MenuItem(" Re Open")) {
                    Action_Menu_ReOpenProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(" Save")) {
                    Action_Menu_SaveProject();
                }

                if (ImGui::MenuItem(" Save As")) {
                    Action_Menu_SaveAsProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(" Close")) {
                    Action_Menu_CloseProject();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem(" About")) {
                OpenAboutDialog();
            }

            ImGui::EndMenu();
        }

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
            if (ImGui::MenuItem("Income")) {
                m_IncomeDialog.show(DataDialogMode::MODE_CREATION);
            }
            if (ImGui::MenuItem("Transaction")) {
                m_TransactionDialog.show(DataDialogMode::MODE_CREATION);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Import")) {
            for (const auto& broker : MainBackend::ref().getDataBrockers()) {
                if (ImGui::BeginMenu(broker.first.c_str())) {
                    for (const auto& way : broker.second) {
                        if (ImGui::MenuItem(way.first.c_str())) {
                            if (way.second != nullptr) {
                                MainBackend::ref().selectDataBrocker(way.second);
                                m_ActionSystem.Clear();
                                m_ActionSystem.Add([&way]() {
                                    const auto& ext = way.second->getFileExt();
                                    IGFD::FileDialogConfig config;
                                    config.countSelectionMax = 0;
                                    config.flags = ImGuiFileDialogFlags_Modal;
                                    ImGuiFileDialog::ref().OpenDialog("Import Datas", "Import Datas from File", ext.c_str(), config);
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

        ImGui::Spacing();

        const auto& io = ImGui::GetIO();
        LayoutManager::ref().DisplayMenu(io.DisplaySize);

        ImGui::Spacing();

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Settings")) {
                SettingsDialog::ref().OpenDialog();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Styles")) {
                ImGuiThemeHelper::ref().DrawMenu();

                ImGui::Separator();

                ImGui::MenuItem("Show ImGui", "", &m_ShowImGui);
                ImGui::MenuItem("Show ImGui Metric/Debug", "", &m_ShowMetric);
                ImGui::MenuItem("Show ImPlot", "", &m_ShowImPlot);

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            if (ImGui::BeginMenu("Delete Tables")) {
                if (ImGui::MenuItem("Banks")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteBanks();
                        BanksPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("Accounts")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteAccounts();
                        AccountsPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("Entities")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteEntities();
                        EntitiesPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("Categories")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteCategories();
                        CategoriesPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("Operations")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteOperations();
                        OperationsPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("Incomes")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteIncomes();
                        IncomesPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("Transactions")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteTransactions();
                        TransactionsPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("Sources")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteSources();
                        // SourcesPane::ref()->Init();
                        return true;
                    });
                }
                if (ImGui::MenuItem("All Except Accounts and banks")) {
                    m_ActionSystem.Clear();
                    m_ActionSystem.Add([]() {
                        DataBase::ref().DeleteEntities();
                        DataBase::ref().DeleteCategories();
                        DataBase::ref().DeleteOperations();
                        DataBase::ref().DeleteIncomes();
                        DataBase::ref().DeleteTransactions();
                        DataBase::ref().DeleteSources();
                        EntitiesPane::ref()->Init();
                        CategoriesPane::ref()->Init();
                        OperationsPane::ref()->Init();
                        IncomesPane::ref()->Init();
                        TransactionsPane::ref()->Init();
                        return true;
                    });
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ProjectFile::ref()->IsThereAnyProjectChanges()) {
            ImGui::Spacing(200.0f);

            if (ImGui::MenuItem(" Save")) {
                Action_Menu_SaveProject();
            }
        }

        // ImGui Infos
        const auto label = ez::str::toStr("Dear ImGui %s (Docking)", ImGui::GetVersion());
        const auto size = ImGui::CalcTextSize(label.c_str());
        static float s_translation_menu_size = 0.0f;

        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - s_translation_menu_size - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", label.c_str());

        s_translation_menu_size = TranslationHelper::ref().DrawMenu();

        ImGui::EndMainMenuBar();
    }
}

void MainFrontend::m_drawMainStatusBar() {
    if (ImGui::BeginMainStatusBar()) {
        Messaging::ref().DrawStatusBar();

        //  ImGui Infos
        const auto& io = ImGui::GetIO();
        const auto fps = ez::str::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
        const auto size = ImGui::CalcTextSize(fps.c_str());
        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", fps.c_str());

        // MainFrontend::sAnyWindowsHovered |= ImGui::IsWindowHovered();

        ImGui::EndMainStatusBar();
    }
}

///////////////////////////////////////////////////////
//// SAVE DIALOG WHEN UN SAVED CHANGES ////////////////
///////////////////////////////////////////////////////

void MainFrontend::OpenUnSavedDialog() {
    // force close dialog if any dialog is opened
    ImGuiFileDialog::ref().Close();

    m_SaveDialogIfRequired = true;
}
void MainFrontend::CloseUnSavedDialog() {
    m_SaveDialogIfRequired = false;
}

bool MainFrontend::ShowUnSavedDialog() {
    bool res = false;

    if (m_SaveDialogIfRequired) {
        if (ProjectFile::ref()->IsProjectLoaded()) {
            if (ProjectFile::ref()->IsThereAnyProjectChanges()) {
                /*
                Unsaved dialog behavior :
                -	save :
                    -	insert action : save project
                -	save as :
                    -	insert action : save as project
                -	continue without saving :
                    -	quit unsaved dialog
                -	cancel :
                    -	clear actions
                */

                ImGui::CloseCurrentPopup();
                const char* label = "Save before closing ?";
                ImGui::OpenPopup(label);
                ImGui::SetNextWindowPos(m_DisplayRect.GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal(label, (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking)) {
                    const auto& width = ImGui::CalcTextSize("Continue without saving").x + ImGui::GetStyle().ItemInnerSpacing.x;
                    
                    if (ImGui::ContrastedButton("Save", nullptr, nullptr, width * 0.5f)) {
                        res = Action_UnSavedDialog_SaveProject();
                    }
                    ImGui::SameLine();
                    if (ImGui::ContrastedButton("Save As", nullptr, nullptr, width * 0.5f)) {
                        Action_UnSavedDialog_SaveAsProject();
                    }

                    if (ImGui::ContrastedButton("Continue without saving")) {
                        res = true;  // quit the action
                    }

                    if (ImGui::ContrastedButton("Cancel", nullptr, nullptr, width + ImGui::GetStyle().FramePadding.x)) {
                        Action_Cancel();
                    }

                    ImGui::EndPopup();
                }
            }
        }

        return res;  // quit if true, else continue on the next frame
    }

    return true;  // quit the action
}

///////////////////////////////////////////////////////
//// ACTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::Action_Menu_NewProject() {
    /*
    new project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : open dialog for new project file name
    -	saved :
        -	add action : open dialog for new project file name
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]() {
        CloseUnSavedDialog();
        IGFD::FileDialogConfig config;
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::ref().OpenDialog("NewProjectDlg", "New Project File", PROJECT_EXT, config);
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_NewProjectDialog(); });
}

void MainFrontend::Action_Menu_OpenProject() {
    /*
    open project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : open project
    -	saved :
        -	add action : open project
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]() {
        CloseUnSavedDialog();
        IGFD::FileDialogConfig config;
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::ref().OpenDialog("OpenProjectDlg", "Open Project File", PROJECT_EXT, config);
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_OpenProjectDialog(); });
}

void MainFrontend::Action_Menu_ImportDatas() {
    m_ActionSystem.Clear();
    m_ActionSystem.Add([this]() {
        CloseUnSavedDialog();
        IGFD::FileDialogConfig config;
        config.countSelectionMax = 0;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::ref().OpenDialog("Import Datas", "Import Datas from File", ".csv", config);
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_OpenProjectDialog(); });
}

void MainFrontend::Action_Menu_ReOpenProject() {
    /*
    re open project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : re open project
    -	saved :
        -	add action : re open project
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([]() {
        MainBackend::ref().NeedToLoadProject(ProjectFile::ref()->GetProjectFilepathName());
        return true;
    });
}

void MainFrontend::Action_Menu_SaveProject() {
    /*
    save project :
    -	never saved :
        -	add action : save as project
    -	saved in a file beofre :
        -	add action : save project
    */
    m_ActionSystem.Clear();
    m_ActionSystem.Add([this]() {
        if (!MainBackend::ref().SaveProject()) {
            CloseUnSavedDialog();
            IGFD::FileDialogConfig config;
            config.countSelectionMax = 1;
            config.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::ref().OpenDialog("SaveProjectDlg", "Save Project File", PROJECT_EXT, config);
        }
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_SaveProjectDialog(); });
}

void MainFrontend::Action_Menu_SaveAsProject() {
    /*
    save as project :
    -	add action : save as project
    */
    m_ActionSystem.Clear();
    m_ActionSystem.Add([this]() {
        CloseUnSavedDialog();
        IGFD::FileDialogConfig config;
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::ref().OpenDialog("SaveProjectDlg", "Save Project File", PROJECT_EXT, config);
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_SaveProjectDialog(); });
}

void MainFrontend::Action_Menu_CloseProject() {
    /*
    Close project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : Close project
    -	saved :
        -	add action : Close project
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([]() {
        MainBackend::ref().NeedToCloseProject();
        return true;
    });
}

void MainFrontend::Action_Window_CloseApp() {
    if (MainBackend::ref().IsNeedToCloseApp())
        return;  // block next call to close app when running
    /*
    Close app :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : Close app
    -	saved :
        -	add action : Close app
    */

    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([]() {
        MainBackend::ref().CloseApp();
        return true;
    });
}

void MainFrontend::Action_OpenUnSavedDialog_IfNeeded() {
    if (ProjectFile::ref()->IsProjectLoaded() && ProjectFile::ref()->IsThereAnyProjectChanges()) {
        OpenUnSavedDialog();
        m_ActionSystem.Add([this]() { return ShowUnSavedDialog(); });
    }
}

void MainFrontend::Action_Cancel() {
    /*
    -	cancel :
        -	clear actions
    */
    CloseUnSavedDialog();
    m_ActionSystem.Clear();
    MainBackend::ref().NeedToCloseApp(false);
}

bool MainFrontend::Action_UnSavedDialog_SaveProject() {
    bool res = MainBackend::ref().SaveProject();
    if (!res) {
        m_ActionSystem.Insert([this]() { return Display_SaveProjectDialog(); });
        m_ActionSystem.Insert([this]() {
            CloseUnSavedDialog();
            IGFD::FileDialogConfig config;
            config.countSelectionMax = 1;
            config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
            config.path = ".";
            ImGuiFileDialog::ref().OpenDialog("SaveProjectDlg", "Save Project File", PROJECT_EXT, config);
            return true;
        });
    }
    return res;
}

void MainFrontend::Action_UnSavedDialog_SaveAsProject() {
    m_ActionSystem.Insert([this]() { return Display_SaveProjectDialog(); });
    m_ActionSystem.Insert([this]() {
        CloseUnSavedDialog();
        IGFD::FileDialogConfig config;
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
        config.path = ".";
        ImGuiFileDialog::ref().OpenDialog("SaveProjectDlg", "Save Project File", PROJECT_EXT, config);
        return true;
    });
}

void MainFrontend::Action_UnSavedDialog_Cancel() {
    Action_Cancel();
}

///////////////////////////////////////////////////////
//// DIALOG FUNCS /////////////////////////////////////
///////////////////////////////////////////////////////

bool MainFrontend::Display_NewProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 max = m_DisplayRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::ref().Display("NewProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::ref().IsOk()) {
            CloseUnSavedDialog();
            auto file = ImGuiFileDialog::ref().GetFilePathName();
            MainBackend::ref().NeedToNewProject(file);
        } else  // cancel
        {
            Action_Cancel();  // we interrupts all actions
        }

        ImGuiFileDialog::ref().Close();

        return true;
    }

    return false;
}

bool MainFrontend::Display_OpenProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 max = m_DisplayRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::ref().Display("OpenProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::ref().IsOk()) {
            CloseUnSavedDialog();
            MainBackend::ref().NeedToLoadProject(ImGuiFileDialog::ref().GetFilePathName());
        } else  // cancel
        {
            Action_Cancel();  // we interrupts all actions
        }

        ImGuiFileDialog::ref().Close();

        return true;
    }

    return false;
}

bool MainFrontend::Display_SaveProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 max = m_DisplayRect.GetSize();
    ImVec2 min = max * 0.5f;

    if (ImGuiFileDialog::ref().Display("SaveProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::ref().IsOk()) {
            CloseUnSavedDialog();
            MainBackend::ref().SaveAsProject(ImGuiFileDialog::ref().GetFilePathName());
        } else  // cancel
        {
            Action_Cancel();  // we interrupts all actions
        }

        ImGuiFileDialog::ref().Close();

        return true;
    }

    return false;
}
