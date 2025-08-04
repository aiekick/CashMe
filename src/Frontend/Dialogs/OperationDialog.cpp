#include "OperationDialog.h"
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>

OperationDialog::OperationDialog() : ADataDialog("OperationModalPopup") {
}

bool OperationDialog::init() {
    return true;
}

void OperationDialog::unit() {
}

void OperationDialog::setOperation(const Operation& vOperation) {
    m_Operation = vOperation;
}

void OperationDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: m_drawContentCreation(vPos); break;
        case DataDialogMode::MODE_DELETE_ONCE: m_drawContentDeletion(vPos); break;
        case DataDialogMode::MODE_DELETE_ALL: m_drawContentDeletion(vPos); break;
        case DataDialogMode::MODE_UPDATE_ONCE: m_drawContentUpdate(vPos); break;
        case DataDialogMode::MODE_UPDATE_ALL: m_drawContentUpdate(vPos); break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void OperationDialog::m_prepare() {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_OperationNameInputText.Clear();
        } break;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: {
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_OperationNameInputText.SetText(m_Operation.name);
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

const char* OperationDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: return "Operation Creation"; break;
        case DataDialogMode::MODE_DELETE_ONCE: return "Operation Deletion"; break;
        case DataDialogMode::MODE_DELETE_ALL: return "Operations Deletion"; break;
        case DataDialogMode::MODE_UPDATE_ONCE: return "Operation Update"; break;
        case DataDialogMode::MODE_UPDATE_ALL: return "Operations Update"; break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return "";
}

bool OperationDialog::m_canConfirm() {
    return !m_OperationNameInputText.empty();
}

void OperationDialog::m_confirmDialog() {
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
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void OperationDialog::m_cancelDialog() {
}

void OperationDialog::m_confirmDialogCreation() {
    if (DataBase::ref().OpenDBFile()) {
        DataBase::ref().AddOperation(  //
            m_OperationNameInputText.GetText());
        DataBase::ref().CloseDBFile();
    }
}

void OperationDialog::m_drawContentCreation(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_OperationNameInputText.DisplayInputText(width, "Operation Name", "", false, align, true);
}

void OperationDialog::m_confirmDialogUpdate() {
    if (DataBase::ref().OpenDBFile()) {
        DataBase::ref().UpdateOperation(  //
            m_Operation.id,                     //
            m_OperationNameInputText.GetText());
        DataBase::ref().CloseDBFile();
    }
}

void OperationDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_OperationNameInputText.DisplayInputText(width, "Operation Name", "", false, align, true);
}

void OperationDialog::m_confirmDialogDeletion() {
    EZ_TOOLS_DEBUG_BREAK;
}

void OperationDialog::m_drawContentDeletion(const ImVec2& vPos) {
    EZ_TOOLS_DEBUG_BREAK;
}
