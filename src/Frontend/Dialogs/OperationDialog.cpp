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

void OperationDialog::setOperation(const OperationOutput& vOperation) {
    m_Operation = vOperation;
}

void OperationDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ALL:
        case DataDialogMode::MODE_UPDATE_ONCE: {
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

void OperationDialog::m_prepare() {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE: {
            m_OperationNameInputText.SetText(m_Operation.datas.name);
        } break;
        case DataDialogMode::MODE_UPDATE_ALL:
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL:
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

const char* OperationDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE: return "Operation Update"; break;
        case DataDialogMode::MODE_UPDATE_ALL:
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: 
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
        case DataDialogMode::MODE_UPDATE_ONCE: {
            m_confirmDialogUpdate();
        } break;
        case DataDialogMode::MODE_UPDATE_ALL:
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: 
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void OperationDialog::m_cancelDialog() {
}

void OperationDialog::m_confirmDialogUpdate() {
    if (DataBase::ref().OpenDBFile()) {
        OperationInput oi;
        oi.name = m_OperationNameInputText.GetText();
        DataBase::ref().UpdateOperation(m_Operation.id, oi);
        DataBase::ref().CloseDBFile();
    }
}

void OperationDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_OperationNameInputText.DisplayInputText(width, "OperationOutput Name", "", false, align, true);
}
