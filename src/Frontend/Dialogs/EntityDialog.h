#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.h>

class EntityDialog : public ADataDialog {
private:
    EntityOutput m_Entity;
    std::vector<EntityOutput> m_EntitiesToMerge;
    std::vector<EntityOutput> m_EntitiesToUpdate;
    std::vector<EntityOutput> m_EntitiesToDelete;
    ImWidgets::InputText m_EntityNameInputText;

public:
    EntityDialog();
    bool init() override;
    void unit() override;
    void setEntity(const EntityOutput& vEntity);
    void setEntitiesToMerge(const std::vector<EntityOutput>& vEntities);
    void setEntitiesToUpdate(const std::vector<EntityOutput>& vEntities);
    void setEntitiesToDelete(const std::vector<EntityOutput>& vEntities);

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

    void m_confirmDialogMerging();
    void m_drawContentMerging(const ImVec2& vPos);

    void m_confirmDialogDeletion();
    void m_drawContentDeletion(const ImVec2& vPos);
};
