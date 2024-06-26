#include <Frontend/Dialogs/abstract/ADataDialog.h>

ADataDialog::ADataDialog(const char* vPopupLabel) : m_PopupLabel(vPopupLabel) {
}

void ADataDialog::show(const DataDialogMode& vMode) {
    m_CurrentMode = vMode;
    m_ShowDialogMode[static_cast<size_t>(vMode)] = true;
    m_prepare();
}

void ADataDialog::hide(const DataDialogMode& vMode) {
    m_CurrentMode = DataDialogMode::MODE_NONE;
    m_ShowDialogMode[static_cast<size_t>(vMode)] = false;
}

const DataDialogMode& ADataDialog::getCurrentMode() const {
    return m_CurrentMode;
}

const bool& ADataDialog::isCurrentModeShown() const {
    return m_ShowDialogMode.at(static_cast<size_t>(m_CurrentMode));
}

bool ADataDialog::draw(const ImVec2& vPos) {
    bool ret = false;
    if (isCurrentModeShown()) {
        ImGui::OpenPopup(m_PopupLabel);
        ImGui::SetNextWindowPos(vPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal(m_PopupLabel,                            //
                                   (bool*)nullptr,                          //
                                   ImGuiWindowFlags_NoTitleBar |            //
                                       ImGuiWindowFlags_NoResize |          //
                                       ImGuiWindowFlags_AlwaysAutoResize |  //
                                       ImGuiWindowFlags_NoDocking)) {
            ImGui::Header(m_getTitle());
            ImGui::Separator();
            m_drawContent(vPos);
            ImGui::Separator();
            if (m_canConfirm()) {
                if (ImGui::ContrastedButton("Ok")) {
                    m_confirmDialog();
                    hide(getCurrentMode());
                    ret = true;
                }
                ImGui::SameLine();
            }
            if (ImGui::ContrastedButton("Cancel")) {
                m_cancelDialog();
                hide(getCurrentMode());
            }
            ImGui::EndPopup();
        }
    }
    return ret;
}
