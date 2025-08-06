#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.h>

class OperationDialog : public ADataDialog {
private:
    OperationOutput m_Operation;
    ImWidgets::InputText m_OperationNameInputText;

public:
    OperationDialog();
    bool init() override;
    void unit() override;
    void setOperation(const OperationOutput& vOperation);

protected:
    void m_drawContent(const ImVec2& vPos) override;
    void m_prepare() override;
    const char* m_getTitle() const override;
    bool m_canConfirm() override;
    void m_confirmDialog() override;
    void m_cancelDialog() override;

    void m_confirmDialogUpdate();
    void m_drawContentUpdate(const ImVec2& vPos);
};
