// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Entities Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/EntitiesPane.h>

#include <cinttypes>  // printf zu

#include <Models/DataBase.h>

#include <Project/ProjectFile.h>

EntitiesPane::EntitiesPane() = default;
EntitiesPane::~EntitiesPane() {
    Unit();
}

bool EntitiesPane::Init() {
    return true;
}

void EntitiesPane::Unit() {
}

void EntitiesPane::Load() {
    m_EntitiesTable.load();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool EntitiesPane::DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
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
                m_EntitiesTable.drawMenu();
                m_EntitiesTable.draw(ImGui::GetContentRegionAvail());
            }
        }

        ImGui::End();
    }
    return change;
}

bool EntitiesPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool EntitiesPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);    
    return false;
}

bool EntitiesPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}
