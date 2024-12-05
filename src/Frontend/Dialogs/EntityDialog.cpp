#include "EntityDialog.h"
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>

EntityDialog::EntityDialog() : ADataDialog("EntityModalPopup") {
}

bool EntityDialog::init() {
    return true;
}

void EntityDialog::unit() {

}

void EntityDialog::setEntity(const Entity& vEntity) {
    m_Entity = vEntity;
}

void EntityDialog::setEntitiesToMerge(const std::vector<Entity>& vEntities) {
    m_EntitiesToMerge = vEntities;
}

void EntityDialog::setEntitiesToUpdate(const std::vector<Entity>& vEntities) {
    m_EntitiesToUpdate = vEntities;
}

void EntityDialog::setEntitiesToDelete(const std::vector<Entity>& vEntities) {
    m_EntitiesToDelete = vEntities;
}

void EntityDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: m_drawContentCreation(vPos); break;
        case DataDialogMode::MODE_DELETE_ONCE: m_drawContentDeletion(vPos); break;
        case DataDialogMode::MODE_DELETE_ALL: m_drawContentDeletion(vPos); break;
        case DataDialogMode::MODE_UPDATE_ONCE: m_drawContentUpdate(vPos); break;
        case DataDialogMode::MODE_UPDATE_ALL: m_drawContentUpdate(vPos); break;
        case DataDialogMode::MODE_MERGE_ALL: m_drawContentMerging(vPos); break;
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void EntityDialog::m_prepare() {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_EntityNameInputText.Clear();
        } break;
        case DataDialogMode::MODE_DELETE_ONCE: 
        case DataDialogMode::MODE_DELETE_ALL: {
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL:
        case DataDialogMode::MODE_MERGE_ALL: {
            m_EntityNameInputText.SetText(m_Entity.name);
        } break; 
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

const char* EntityDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: return "Entities Creation"; break;
        case DataDialogMode::MODE_DELETE_ONCE: return "Entities Deletion"; break;
        case DataDialogMode::MODE_DELETE_ALL: return "Entities Deletion"; break;
        case DataDialogMode::MODE_UPDATE_ONCE: return "Entities Update"; break;
        case DataDialogMode::MODE_UPDATE_ALL: return "Entities Update"; break;
        case DataDialogMode::MODE_MERGE_ALL: return "Entities Merging"; break;
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return "";
}

bool EntityDialog::m_canConfirm() {
    return !m_EntityNameInputText.empty();
}

void EntityDialog::m_confirmDialog() {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_confirmDialogCreation();
        } break;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: {
            m_confirmDialogDeletion();
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_confirmDialogUpdate();
        } break;
        case DataDialogMode::MODE_MERGE_ALL: {
            m_confirmDialogMerging();
        } break;
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void EntityDialog::m_cancelDialog() {
}

void EntityDialog::m_confirmDialogCreation() {
    if (DataBase::Instance()->OpenDBFile()) {
        DataBase::Instance()->AddEntity(        //
            m_EntityNameInputText.GetText());
        DataBase::Instance()->CloseDBFile();
    }
}

void EntityDialog::m_drawContentCreation(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_EntityNameInputText.DisplayInputText(width, "Entity Name", "", false, align, true);
}

void EntityDialog::m_confirmDialogUpdate() {
    if (DataBase::Instance()->OpenDBFile()) {
        DataBase::Instance()->UpdateEntity(     //
            m_Entity.id,                        //
            m_EntityNameInputText.GetText());
        DataBase::Instance()->CloseDBFile();
    }
}

void EntityDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_EntityNameInputText.DisplayInputText(width, "Entity Name", "", false, align, true);
}

void EntityDialog::m_drawContentMerging(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_EntityNameInputText.DisplayInputText(width, "Entity Name", "", false, align, true);
}

void EntityDialog::m_confirmDialogMerging() {
    EZ_TOOLS_DEBUG_BREAK;
    if (DataBase::Instance()->OpenDBFile()) {
        /*DataBase::Instance()->MergeEntities(  //
            m_Entity.id,                      //
            m_EntityNameInputText.GetText());*/
        DataBase::Instance()->CloseDBFile();
    }
}

void EntityDialog::m_confirmDialogDeletion() {
    EZ_TOOLS_DEBUG_BREAK;
}

void EntityDialog::m_drawContentDeletion(const ImVec2& vPos) {
    EZ_TOOLS_DEBUG_BREAK;
}
