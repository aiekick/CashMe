#include "BankDialog.h"
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>

BankDialog::BankDialog() : ADataDialog("BankModalPopup") {
}

bool BankDialog::init() {
    return true;
}

void BankDialog::unit() {

}

void BankDialog::setBank(const BankOutput& vBank) {
    m_Bank = vBank;
}

void BankDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
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

void BankDialog::m_prepare() {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_BankNameInputText.Clear();
            m_BankUrlInputText.Clear();
        } break;
        case DataDialogMode::MODE_DELETE_ONCE: 
        case DataDialogMode::MODE_DELETE_ALL: {
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_BankNameInputText.SetText(m_Bank.datas.name);
            m_BankUrlInputText.SetText(m_Bank.datas.url);
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

const char* BankDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: return "Bank Creation"; break;
        case DataDialogMode::MODE_DELETE_ONCE: return "Bank Deletion"; break;
        case DataDialogMode::MODE_DELETE_ALL: return "Banks Deletion"; break;
        case DataDialogMode::MODE_UPDATE_ONCE: return "Bank Update"; break;
        case DataDialogMode::MODE_UPDATE_ALL: return "Banks Update"; break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return "";
}

bool BankDialog::m_canConfirm() {
    return !m_BankNameInputText.empty();
}

void BankDialog::m_confirmDialog() {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
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

void BankDialog::m_cancelDialog() {
}

void BankDialog::m_confirmDialogCreation() {
    if (DataBase::Instance()->OpenDBFile()) {
        BankInput bi;
        bi.name = m_BankNameInputText.GetText();
        bi.url = m_BankUrlInputText.GetText();
        DataBase::Instance()->AddBank(bi);
        DataBase::Instance()->CloseDBFile();
    }
}

void BankDialog::m_drawContentCreation(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_BankNameInputText.DisplayInputText(width, "Bank Name", "", false, align, true);
    m_BankUrlInputText.DisplayInputText(width, "Bank Url", "", false, align);
}

void BankDialog::m_confirmDialogUpdate() {
    if (DataBase::Instance()->OpenDBFile()) {
        BankInput bi;
        bi.name = m_BankNameInputText.GetText();
        bi.url = m_BankUrlInputText.GetText();
        DataBase::Instance()->UpdateBank(m_Bank.id, bi);
        DataBase::Instance()->CloseDBFile();
    }
}

void BankDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_BankNameInputText.DisplayInputText(width, "Bank Name", "", false, align, true);
    m_BankUrlInputText.DisplayInputText(width, "Bank Url", "", false, align);
}

void BankDialog::m_confirmDialogDeletion() {
    EZ_TOOLS_DEBUG_BREAK;
}

void BankDialog::m_drawContentDeletion(const ImVec2& vPos) {
    EZ_TOOLS_DEBUG_BREAK;
}