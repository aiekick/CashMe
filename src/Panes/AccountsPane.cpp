// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Accounts Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/AccountsPane.h>

#include <cinttypes>  // printf zu

#include <Models/DataBase.h>
#include <Panes/StatementsPane.h>
#include <Project/ProjectFile.h>

AccountsPane::AccountsPane() = default;
AccountsPane::~AccountsPane() {
    Unit();
}

bool AccountsPane::Init() {
    return true;
}

void AccountsPane::Unit() {
}

void AccountsPane::Load() {
    m_AccountsTable.load();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool AccountsPane::DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    if (vOpened != nullptr && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus /*| ImGuiWindowFlags_MenuBar*/;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus /*| ImGuiWindowFlags_MenuBar*/;
#endif

            if (ProjectFile::Instance()->IsProjectLoaded()) {
                m_AccountsTable.drawMenu();
                m_AccountsTable.draw(ImGui::GetContentRegionAvail());
            }
        }

        ImGui::End();
    }
    return change;
}

bool AccountsPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool AccountsPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImRect& vRect, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    const ImVec2 center = vRect.GetCenter();

    bool ret = false;

    ret |= m_AccountsTable.getAccountDialogRef().draw(center);

    if (ret) {
        m_AccountsTable.load();
        StatementsPane::Instance()->Load();
    }

    return ret;  
}

bool AccountsPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}
