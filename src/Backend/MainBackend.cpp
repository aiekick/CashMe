#include "MainBackend.h"
#include <ezlibs/ezOS.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#ifdef WINDOWS_OS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <Headers/CashMeBuild.h>

#define IMGUI_IMPL_API
#include <3rdparty/imgui_docking/backends/imgui_impl_opengl3.h>
#include <3rdparty/imgui_docking/backends/imgui_impl_glfw.h>

#include <cstdio>     // printf, fprintf
#include <chrono>     // timer
#include <cstdlib>    // abort
#include <fstream>    // std::ifstream
#include <iostream>   // std::cout
#include <algorithm>  // std::min, std::max
#include <stdexcept>  // std::exception

#include <Plugins/PluginManager.h>
#include <Project/ProjectFile.h>

#include <LayoutManager.h>

#include <imguipack.h>

#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezLog.hpp>
#include <ezlibs/ezFile.hpp>
#include <Frontend/MainFrontend.h>

#include <Panes/ConsolePane.h>

#include <Models/DataBase.h>

#include <Systems/SettingsDialog.h>
#include <Panes/TransactionsPane.h>

// we include the cpp just for embedded fonts
#include <Res/fontIcons.cpp>
#include <Res/Roboto_Medium.cpp> 

#define INITIAL_WIDTH 1700
#define INITIAL_HEIGHT 700

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

static void glfw_error_callback(int error, const char* description) {
    LogVarError("glfw error %i : %s", error, description);
}

static void glfw_window_close_callback(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, GLFW_FALSE);  // block app closing
    MainFrontend::ref().Action_Window_CloseApp();
}

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainBackend::~MainBackend() = default;

void MainBackend::run(const ez::App& vApp) {
    if (init(vApp)) {
        m_MainLoop();
        unit();
    }
}

// todo : to refactor ! i dont like that
bool MainBackend::init(const ez::App& vApp) {
#ifdef _DEBUG
    SetConsoleVisibility(true);
#else
    SetConsoleVisibility(false);
#endif
    m_InitModels();
    if (m_InitWindow() && m_InitImGui()) {
        m_InitPlugins(vApp);
        m_InitSystems();
        m_InitPanes();
        m_InitSettings();
        LoadConfigFile("config.xml", "app");
        return true;
    }
    return false;
}

// todo : to refactor ! i dont like that
void MainBackend::unit() {
    SaveConfigFile("config.xml", "app", "config");
    m_UnitSystems();
    m_UnitSettings();  // before plugins and gui since containing weak ptr to plugins
    m_UnitImGui();     // before plugins since containing weak ptr to plugins
    m_UnitPlugins();
    m_UnitWindow();
    m_UnitModels();
}

bool MainBackend::isThereAnError() const {
    return false;
}

void MainBackend::NeedToNewProject(const std::string& vFilePathName) {
    m_NeedToNewProject = true;
    m_ProjectFileToLoad = vFilePathName;
}

void MainBackend::NeedToLoadProject(const std::string& vFilePathName) {
    m_NeedToLoadProject = true;
    m_ProjectFileToLoad = vFilePathName;
}

void MainBackend::NeedToCloseProject() {
    m_NeedToCloseProject = true;
}

bool MainBackend::SaveProject() {
    return ProjectFile::ref()->Save();
}

void MainBackend::SaveAsProject(const std::string& vFilePathName) {
    ProjectFile::ref()->SaveAs(vFilePathName);
}

// actions to do after rendering
void MainBackend::PostRenderingActions() {
    if (m_NeedToNewProject) {
        ProjectFile::ref()->Clear();
        ProjectFile::ref()->New(m_ProjectFileToLoad);
        m_ProjectFileToLoad.clear();
        m_NeedToNewProject = false;
    }

    if (m_NeedToLoadProject) {
        if (!m_ProjectFileToLoad.empty()) {
            if (ProjectFile::ref()->LoadAs(m_ProjectFileToLoad)) {
                setAppTitle(m_ProjectFileToLoad);
                ProjectFile::ref()->SetProjectChange(false);
            } else {
                LogVarError("Failed to load project %s", m_ProjectFileToLoad.c_str());
            }
        }

        m_ProjectFileToLoad.clear();
        m_NeedToLoadProject = false;
    }

    if (m_NeedToCloseProject) {
        ProjectFile::ref()->Clear();
        m_NeedToCloseProject = false;
    }

    // Backend operation, cannot be blocked if a imgui item is not displayed
    m_importThread.finishIfNeeded();
}

bool MainBackend::IsNeedToCloseApp() {
    return m_NeedToCloseApp;
}

void MainBackend::NeedToCloseApp(const bool& vFlag) {
    m_NeedToCloseApp = vFlag;
}

void MainBackend::CloseApp() {
    // will escape the main loop
    glfwSetWindowShouldClose(m_MainWindowPtr, 1);
}

void MainBackend::setAppTitle(const std::string& vFilePathName) {
    auto ps = ez::file::parsePathFileName(vFilePathName);
    if (ps.isOk) {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "CashMe Beta %s - Project : %s", CashMe_BuildId, vFilePathName.c_str());
        glfwSetWindowTitle(m_MainWindowPtr, bufTitle);
    } else {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "CashMe Beta %s", CashMe_BuildId);
        glfwSetWindowTitle(m_MainWindowPtr, bufTitle);
    }
}

ez::dvec2 MainBackend::GetMousePos() {
    ez::dvec2 mp;
    glfwGetCursorPos(m_MainWindowPtr, &mp.x, &mp.y);
    return mp;
}

int MainBackend::GetMouseButton(int vButton) {
    return glfwGetMouseButton(m_MainWindowPtr, vButton);
}

void MainBackend::importFromFiles(const std::vector<std::string>& vFiles) {
    m_importThread.start(  //
        "Import Datas",
        m_selectedBroker,
        vFiles,
        [this]() { TransactionsPane::ref()->Init(); },
        nullptr);
}

void MainBackend::selectDataBrocker(const Cash::BankStatementModuleWeak& vDatabrocker) {
    m_selectedBroker = vDatabrocker;
}

const DataBrockerContainer& MainBackend::getDataBrockers() {
    return m_dataBrokerModules;
}

ImportWorkerThread& MainBackend::getImportWorkerThreadRef() {
    return m_importThread;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONSOLE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainBackend::SetConsoleVisibility(const bool& vFlag) {
    m_ConsoleVisiblity = vFlag;

    if (m_ConsoleVisiblity) {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
    } else {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    }
}

void MainBackend::SwitchConsoleVisibility() {
    m_ConsoleVisiblity = !m_ConsoleVisiblity;
    SetConsoleVisibility(m_ConsoleVisiblity);
}

bool MainBackend::GetConsoleVisibility() {
    return m_ConsoleVisiblity;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

ez::xml::Nodes MainBackend::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    node.addChilds(MainFrontend::ref().getXmlNodes(vUserDatas));
    node.addChilds(SettingsDialog::ref().getXmlNodes(vUserDatas));
    node.addChild("project").setContent(ProjectFile::ref()->GetProjectFilepathName());
    return node.getChildren();
}

bool MainBackend::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    const auto& strName = vNode.getName();
    const auto& strValue = vNode.getContent();
    const auto& strParentName = vParent.getName();
    if (strName == "project") {
        NeedToLoadProject(strValue);
    }
    MainFrontend::ref().setFromXmlNodes(vNode, vParent, vUserDatas);
    SettingsDialog::ref().setFromXmlNodes(vNode, vParent, vUserDatas);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// RENDER ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainBackend::m_RenderOffScreen() {
    // m_DisplaySizeQuadRendererPtr->SetImageInfos(m_MergerRendererPtr->GetBackDescriptorImageInfo(0U));
}

void MainBackend::m_getAvailableDataBrokers() {
    m_clearDataBrokers();
    auto modules = PluginManager::ref().GetPluginModulesInfos();
    for (const auto& mod : modules) {
        if (mod.type == Cash::PluginModuleType::DATA_BROKER) {
            auto ptr = std::dynamic_pointer_cast<Cash::BankStatementImportModule>(PluginManager::ref().CreatePluginModule(mod.label));
            if (ptr != nullptr) {
                m_dataBrokerModules[mod.path][mod.label] = ptr;
            }
        }
    }
}

void MainBackend::m_clearDataBrokers() {  // must be reset before quit since point on the memory of a plugin
    m_selectedBroker.reset();
    m_dataBrokerModules.clear();
}

void MainBackend::m_MainLoop() {
    int display_w, display_h;
    ImRect viewRect;
    while (!glfwWindowShouldClose(m_MainWindowPtr)) {
        ProjectFile::ref()->NewFrame();

        // maintain active, prevent user change via imgui dialog
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
        //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Disable Viewport

        glfwPollEvents();

        glfwGetFramebufferSize(m_MainWindowPtr, &display_w, &display_h);

        m_update();  // to do absolutly before imgui rendering

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport) {
            viewRect.Min = viewport->WorkPos;
            viewRect.Max = viewRect.Min + viewport->WorkSize;
        } else {
            viewRect.Max = ImVec2((float)display_w, (float)display_h);
        }

        MainFrontend::ref().Display(m_CurrentFrame, viewRect);

        ImGui::Render();

        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        auto* backup_current_context = glfwGetCurrentContext();

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste
        // this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        glfwMakeContextCurrent(backup_current_context);

        glfwSwapBuffers(m_MainWindowPtr);

        // mainframe post actions
        PostRenderingActions();

        ++m_CurrentFrame;

        // will pause the view until we move the mouse or press keys
        // glfwWaitEvents();
    }
}

void MainBackend::m_update() {

}

void MainBackend::m_IncFrame() {
    ++m_CurrentFrame;
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainBackend::m_InitWindow() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    // GL 3.0 + GLSL 130
    m_GlslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with graphics context
    m_MainWindowPtr = glfwCreateWindow(1280, 720, "CashMe", nullptr, nullptr);
    if (m_MainWindowPtr == nullptr) {
        std::cout << "Fail to create the window" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(m_MainWindowPtr);
    glfwSwapInterval(1);  // Enable vsync

    if (gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }

    glfwSetWindowCloseCallback(m_MainWindowPtr, glfw_window_close_callback);

#ifdef WINDOWS_OS
    auto icon_h_inst = LoadIconA(GetModuleHandle(NULL), "IDI_ICON1");
    SetClassLongPtrA(glfwGetWin32Window(m_MainWindowPtr), GCLP_HICON, (LONG_PTR)icon_h_inst);
#endif

    return true;
}

void MainBackend::m_UnitWindow() {
    glfwDestroyWindow(m_MainWindowPtr);
    glfwTerminate();
}

bool MainBackend::m_InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Viewport
    io.FontAllowUserScaling = true;                      // activate zoom feature with ctrl + mousewheel
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
    io.ConfigViewportsNoDecoration = false;  // toujours mettre une frame aux fenetres enfants
#endif

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // fonts
    {
        {  // main font
            if (ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, 15.0f) == nullptr) {
                assert(0);  // failed to load font
            }
        }
        {  // icon font
            static const ImWchar icons_ranges[] = {ICON_MIN_STRO, ICON_MAX_STRO, 0};
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            if (ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_STRO, 15.0f, &icons_config, icons_ranges) == nullptr) {
                assert(0);  // failed to load font
            }
        }
    }    

    MainFrontend::initSingleton();

    // Setup Platform/Renderer bindings
    if (ImGui_ImplGlfw_InitForOpenGL(m_MainWindowPtr, true) &&  //
        ImGui_ImplOpenGL3_Init(m_GlslVersion)) {

        // ui init
        if (MainFrontend::ref().init()) {
            return true;
        }
    }
    return false;
}

void MainBackend::m_InitPlugins(const ez::App& vApp) {
    PluginManager::initSingleton();
    PluginManager::ref().LoadPlugins(vApp);
    auto pluginPanes = PluginManager::ref().GetPluginPanes();
    for (auto& pluginPane : pluginPanes) {
        if (!pluginPane.pane.expired()) {
            LayoutManager::ref().AddPane(  //
                pluginPane.pane,
                pluginPane.name,
                pluginPane.category,
                pluginPane.disposal,
                pluginPane.disposalRatio,
                pluginPane.openedDefault,
                pluginPane.focusedDefault);
            auto plugin_ptr = std::dynamic_pointer_cast<Cash::PluginPane>(pluginPane.pane.lock());
            if (plugin_ptr != nullptr) {
                plugin_ptr->SetProjectInstance(ProjectFile::ref());
            }
        }
    }
    m_getAvailableDataBrokers();
}

void MainBackend::m_InitModels() {
    DataBase::initSingleton();
}

void MainBackend::m_UnitModels() {
    DataBase::unitSingleton();
}

void MainBackend::m_UnitPlugins() {
    m_clearDataBrokers();
    PluginManager::ref().Clear();
    PluginManager::unitSingleton();
}

void MainBackend::m_InitSystems() {
    
}

void MainBackend::m_UnitSystems() {
}

void MainBackend::m_InitPanes() {
    if (LayoutManager::ref().InitPanes()) {
        // a faire apres InitPanes() sinon ConsolePane::ref()->paneFlag vaudra 0 et changeras apres InitPanes()
        Messaging::ref().sMessagePaneId = ConsolePane::ref()->GetFlag();
    }
}

void MainBackend::m_UnitPanes() {
}

void MainBackend::m_InitSettings() {
    SettingsDialog::initSingleton();
    SettingsDialog::ref().init();
}

void MainBackend::m_UnitSettings() {
    SettingsDialog::ref().unit();
    SettingsDialog::unitSingleton();
}

void MainBackend::m_UnitImGui() {
    MainFrontend::ref().unit();
    LayoutManager::ref().Unit();
    MainFrontend::unitSingleton();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

