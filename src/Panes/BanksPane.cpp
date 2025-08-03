// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Banks Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/BanksPane.h>

#include <Models/DataBase.h>
#include <Project/ProjectFile.h>
#include <Panes/StatementsPane.h>

#include <cinttypes>  // printf zu

BanksPane::BanksPane() = default;
BanksPane::~BanksPane() {
    Unit();
}

bool BanksPane::Init() {
    return true;
}

void BanksPane::Unit() {
}

void BanksPane::Load() {
    m_BanksTable.load();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool BanksPane::DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    if (vOpened != nullptr && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus /* | ImGuiWindowFlags_MenuBar*/;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus /* | ImGuiWindowFlags_MenuBar*/;
#endif

            if (ProjectFile::Instance()->IsProjectLoaded()) {
                m_BanksTable.drawMenu();
                m_BanksTable.draw(ImGui::GetContentRegionAvail());
            }
        }

        ImGui::End();
    }
    return change;
}

bool BanksPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool BanksPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& vRect, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    const ImVec2 center = vRect.GetCenter();

    bool ret = false;

    ret |= m_BanksTable.getBankDialogRef().draw(center);

    if (ret) {
        m_BanksTable.load();
        StatementsPane::Instance()->Load();
    }

    return ret;
}

bool BanksPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}
