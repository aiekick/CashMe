#pragma once

#include <ImGuiPack.h>

#include <array>

enum class DataDialogMode {
    MODE_NONE = 0,     //
    MODE_CREATION,     //
    MODE_UPDATE_ONCE,  //
    MODE_UPDATE_ALL,   //
    MODE_DELETE_ONCE,  //
    MODE_DELETE_ALL,   //
    MODE_Count         //
};

template<typename TYPE>
class ADataDialog {
public:

private:
    DataDialogMode m_CurrentMode = DataDialogMode::MODE_NONE;
    std::array<bool, static_cast<size_t>(DataDialogMode::MODE_Count)> m_ShowDialogMode = {};
    const char* m_PopupLabel = nullptr;

public:
    explicit ADataDialog(const char* vPopupLabel) : m_PopupLabel(vPopupLabel) {}
    virtual ~ADataDialog() = default;
    virtual bool init() = 0;
    virtual void unit() = 0;

    void show(const DataDialogMode& vMode) {
        m_CurrentMode = vMode;
        m_ShowDialogMode[static_cast<size_t>(vMode)] = true;
        m_prepare();
    }

    void hide(const DataDialogMode& vMode) {
        m_CurrentMode = DataDialogMode::MODE_NONE;
        m_ShowDialogMode[static_cast<size_t>(vMode)] = false;
    }

    const DataDialogMode& getCurrentMode() const {
        return m_CurrentMode;
    }

    const bool& isCurrentModeShown() const {
        return m_ShowDialogMode.at(static_cast<size_t>(m_CurrentMode));
    }

    void draw(const ImVec2& vPos) {
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
    }

protected:
    virtual void m_drawContent(const ImVec2& vPos) = 0;
    virtual void m_prepare() = 0;
    virtual const char* m_getTitle() const = 0;
    virtual bool m_canConfirm() = 0;
    virtual void m_confirmDialog() = 0;
    virtual void m_cancelDialog() = 0;

    void m_DisplayAlignedWidget(const float& vWidth, const std::string& vLabel, const float& vOffsetFromStart, std::function<void()> vWidget) {
        float px = ImGui::GetCursorPosX();
        ImGui::Text("%s", vLabel.c_str());
        ImGui::SameLine(vOffsetFromStart);
        const float w = vWidth - (ImGui::GetCursorPosX() - px);
        ImGui::PushID(++ImGui::CustomStyle::pushId);
        ImGui::PushItemWidth(w);
        if (vWidget != nullptr) {
            vWidget();
        }
        ImGui::PopItemWidth();
        ImGui::PopID();
    }
};
