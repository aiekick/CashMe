#include "AccountDialog.h"
#include <ezlibs/ezTools.hpp>
#include <Models/DataBase.h>

AccountDialog::AccountDialog() : ADataDialog("AccountModalPopup") {
}

bool AccountDialog::init() {
    return true;
}

void AccountDialog::unit() {

}

void AccountDialog::setAccount(const AccountOutput& vAccount) {
    m_Account = vAccount;
}

void AccountDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL); // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_drawContentCreation(vPos);
        } break;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: {
            m_drawContentDeletion(vPos);
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_drawContentUpdate(vPos);
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void AccountDialog::m_prepare() {
    m_UpdateBanks();
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_BankAgencyInputText.Clear();
            m_AccountNameInputText.Clear();
            m_AccountTypeInputText.Clear();
            m_AccountNumberInputText.Clear();
            m_AccountBaseSoldeInputDouble = 0.0;
        } break;
        case DataDialogMode::MODE_DELETE_ONCE: 
        case DataDialogMode::MODE_DELETE_ALL: {
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_BanksCombo.select(m_Account.bankName);
            m_BankAgencyInputText.SetText(m_Account.datas.bank_agency);
            m_AccountNameInputText.SetText(m_Account.datas.name);
            m_AccountTypeInputText.SetText(m_Account.datas.type);
            m_AccountNumberInputText.SetText(m_Account.datas.number);
            m_AccountBaseSoldeInputDouble = m_Account.datas.base_solde;
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

const char* AccountDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: return "Account Creation"; break;
        case DataDialogMode::MODE_DELETE_ONCE: return "Account Deletion"; break;
        case DataDialogMode::MODE_DELETE_ALL: return "Accounts Deletion"; break;
        case DataDialogMode::MODE_UPDATE_ONCE: return "Account Update"; break;
        case DataDialogMode::MODE_UPDATE_ALL: return "Accounts Update"; break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return "";
}

bool AccountDialog::m_canConfirm() {
    return !m_AccountNameInputText.empty() &&  //
        !m_AccountTypeInputText.empty() &&     //
        !m_AccountNumberInputText.empty() &&   //
        !m_BankAgencyInputText.empty();
}

void AccountDialog::m_confirmDialog() {
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

void AccountDialog::m_cancelDialog() {
}

void AccountDialog::m_confirmDialogCreation() {
    if (DataBase::ref().OpenDBFile()) {
        AccountInput ai;
        ai.bank_agency = m_BankAgencyInputText.GetText();
        ai.type = m_AccountTypeInputText.GetText();
        ai.name = m_AccountNameInputText.GetText();
        ai.number = m_AccountNumberInputText.GetText();
        ai.base_solde = m_AccountBaseSoldeInputDouble;
        DataBase::ref().AddAccount(m_BanksCombo.getText(), ai);
        DataBase::ref().CloseDBFile();
    }
}

void AccountDialog::m_drawContentCreation(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_BanksCombo.displayWithColumn(width, "Bank Name", align);
    m_BankAgencyInputText.DisplayInputText(width, "bank Agency", "", false, align, true);
    m_AccountNameInputText.DisplayInputText(width, "Account Name", "", false, align, true);
    m_AccountTypeInputText.DisplayInputText(width, "Account Type", "", false, align, true);
    m_AccountNumberInputText.DisplayInputText(width, "Account Number", "", false, align, true);
    ImGui::DisplayAlignedWidget(width, "Base Solde", align, [this]() { ImGui::InputDouble("##BaseSolde", &m_AccountBaseSoldeInputDouble); });
}

void AccountDialog::m_confirmDialogUpdate() {
    if (DataBase::ref().OpenDBFile()) {
        AccountInput ai;
        ai.bank_agency = m_BankAgencyInputText.GetText();
        ai.type = m_AccountTypeInputText.GetText();
        ai.name = m_AccountNameInputText.GetText();
        ai.number = m_AccountNumberInputText.GetText();
        ai.base_solde = m_AccountBaseSoldeInputDouble;
        DataBase::ref().UpdateAccount(m_Account.id, m_BanksCombo.getText(), ai);
        DataBase::ref().CloseDBFile();
    }
}

void AccountDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_BanksCombo.displayWithColumn(width, "Bank Name", align);
    m_AccountNameInputText.DisplayInputText(width, "Account Name", "", false, align, true);
    m_AccountTypeInputText.DisplayInputText(width, "Account Type", "", false, align, true);
    m_AccountNumberInputText.DisplayInputText(width, "Account Number", "", false, align, true);
    ImGui::DisplayAlignedWidget(width, "Base Solde", align, [this]() { ImGui::InputDouble("##BaseSolde", &m_AccountBaseSoldeInputDouble); });
}

void AccountDialog::m_confirmDialogDeletion() {
    EZ_TOOLS_DEBUG_BREAK;
}

void AccountDialog::m_drawContentDeletion(const ImVec2& vPos) {
    EZ_TOOLS_DEBUG_BREAK;
}

void AccountDialog::m_UpdateBanks() {
    m_BanksCombo.clear();
    DataBase::ref().GetBanks(                                       //
        [this](const BankOutput& vBankOutput) {  //
            m_BanksCombo.getArrayRef().push_back(vBankOutput.datas.name);
        });
    m_BanksCombo.getIndexRef() = 0;
    m_BanksCombo.finalize();
}