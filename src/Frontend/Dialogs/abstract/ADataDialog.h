#pragma once

#include <imguipack.h>

#include <array>

enum class DataDialogMode {
    MODE_NONE = 0,     //
    MODE_CREATION,     //
    MODE_UPDATE_ONCE,  //
    MODE_UPDATE_ALL,   //
    MODE_DELETE_ONCE,  //
    MODE_DELETE_ALL,   //
    MODE_MERGE_ALL,    //
    MODE_Count         //
};

class ADataDialog {
private:
    DataDialogMode m_CurrentMode = DataDialogMode::MODE_NONE;
    std::array<bool, static_cast<size_t>(DataDialogMode::MODE_Count)> m_ShowDialogMode = {};
    const char* m_PopupLabel = nullptr;

public:
    explicit ADataDialog(const char* vPopupLabel);
    virtual ~ADataDialog() = default;
    virtual bool init() = 0;
    virtual void unit() = 0;
    void show(const DataDialogMode& vMode);
    void hide(const DataDialogMode& vMode);
    const DataDialogMode& getCurrentMode() const;
    const bool& isCurrentModeShown() const;
    bool draw(const ImVec2& vPos);

protected:
    virtual void m_drawContent(const ImVec2& vPos) = 0;
    virtual void m_prepare() = 0;
    virtual const char* m_getTitle() const = 0;
    virtual bool m_canConfirm() = 0;
    virtual void m_confirmDialog() = 0;
    virtual void m_cancelDialog() = 0;
};
