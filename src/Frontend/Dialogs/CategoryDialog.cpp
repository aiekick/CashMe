#include "CategoryDialog.h"
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>

CategoryDialog::CategoryDialog() : ADataDialog("CategoryModalPopup") {
}

bool CategoryDialog::init() {
    return true;
}

void CategoryDialog::unit() {

}

void CategoryDialog::setCategory(const CategoryOutput& vCategory) {
    m_Category = vCategory;
}

void CategoryDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE: {
            m_drawContentUpdate(vPos);
        } break;
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: 
        case DataDialogMode::MODE_UPDATE_ALL: 
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void CategoryDialog::m_prepare() {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE: {
            m_CategoryNameInputText.SetText(m_Category.datas.name);
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

const char* CategoryDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE: return "Category Update"; break;
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

bool CategoryDialog::m_canConfirm() {
    return !m_CategoryNameInputText.empty();
}

void CategoryDialog::m_confirmDialog() {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_UPDATE_ONCE:{
            m_confirmDialogUpdate();
        } break;
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL:
        case DataDialogMode::MODE_UPDATE_ALL: 
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void CategoryDialog::m_cancelDialog() {
}

void CategoryDialog::m_confirmDialogUpdate() {
    if (DataBase::ref().OpenDBFile()) {
        CategoryInput ci;
        ci.name = m_CategoryNameInputText.GetText();
        DataBase::ref().UpdateCategory(m_Category.id, ci);
        DataBase::ref().CloseDBFile();
    }
}

void CategoryDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_CategoryNameInputText.DisplayInputText(width, "Category Name", "", false, align, true);
}
