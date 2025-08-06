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

void EntityDialog::setEntity(const EntityOutput& vEntity) {
    m_Entity = vEntity;
}

void EntityDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE: 
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_drawContentUpdate(vPos);
        } break;
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL:
        case DataDialogMode::MODE_MERGE_ALL: 
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void EntityDialog::m_prepare() {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_EntityNameInputText.SetText(m_Entity.datas.name);
        } break;
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL:
        case DataDialogMode::MODE_MERGE_ALL:
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
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_confirmDialogUpdate();
        } break;
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL:
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void EntityDialog::m_cancelDialog() {
}

void EntityDialog::m_confirmDialogUpdate() {
    if (DataBase::ref().OpenDBFile()) {
        EntityInput ei;
        ei.name = m_EntityNameInputText.GetText();
        DataBase::ref().UpdateEntity(m_Entity.id, ei);
        DataBase::ref().CloseDBFile();
    }
}

void EntityDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_EntityNameInputText.DisplayInputText(width, "Entity Name", "", false, align, true);
}
