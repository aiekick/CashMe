#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.h>

class EntityDialog : public ADataDialog {
private:
    Entity m_Entity;
    ImWidgets::InputText m_EntityNameInputText;

public:
    EntityDialog();
    bool init() override;
    void unit() override;
    void setEntity(const Entity& vEntity);

protected:
    void m_drawContent(const ImVec2& vPos) override;
    void m_prepare() override;
    const char* m_getTitle() const override;
    bool m_canConfirm() override;
    void m_confirmDialog() override;
    void m_cancelDialog() override;

    void m_confirmDialogCreation();
    void m_drawContentCreation(const ImVec2& vPos);

    void m_confirmDialogUpdate();
    void m_drawContentUpdate(const ImVec2& vPos);

    void m_confirmDialogDeletion();
    void m_drawContentDeletion(const ImVec2& vPos);
};
