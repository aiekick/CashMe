// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Budget Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/BudgetPane.h>

#include <Models/DataBase.h>
#include <Project/ProjectFile.h>
#include <Frontend/MainFrontend.h>

BudgetPane::BudgetPane() = default;
BudgetPane::~BudgetPane() {
    Unit();
}

bool BudgetPane::Init() {
    bool ret = true;
    if (ProjectFile::ref()->IsProjectLoaded()) {
        ret &= m_BudgetTable.load();
    }
    return ret;
}

void BudgetPane::Unit() {
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool BudgetPane::DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
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

            if (ProjectFile::ref()->IsProjectLoaded()) {
                m_BudgetTable.draw(ImGui::GetContentRegionAvail());
            }
        }

        ImGui::End();
    }
    return change;
}

bool BudgetPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool BudgetPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);    
    return false;
}

bool BudgetPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

ez::xml::Nodes BudgetPane::getXmlNodes(const std::string& vUserDatas) {
    ez::xml::Node node;
    auto& budgetNode = node.addChild("budget");
    budgetNode.addChilds(m_BudgetTable.getXmlNodes());
    return node.getChildren();
}

bool BudgetPane::setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) {
    if (vNode.getName() == "budget") {
        m_BudgetTable.RecursParsingConfigChilds(vNode, vUserDatas);
    }
    return false;
}