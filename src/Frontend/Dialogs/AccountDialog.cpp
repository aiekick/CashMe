#include "AccountDialog.h"
#include <Models/DataBase.h>
#include <Models/DataBrokers.h>
#include <ctools/cTools.h>

AccountDialog::AccountDialog() : ADataDialog("AccountModalPopup") {
}

bool AccountDialog::init() {
    return true;
}

void AccountDialog::unit() {

}

void AccountDialog::setAccount(const Account& vAccount) {
    m_Account = vAccount;
}

void AccountDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: m_drawContentCreation(vPos); break;
        case DataDialogMode::MODE_DELETE_ONCE: m_drawContentDeletion(vPos); break;
        case DataDialogMode::MODE_DELETE_ALL: m_drawContentDeletion(vPos); break;
        case DataDialogMode::MODE_UPDATE_ONCE: m_drawContentUpdate(vPos); break;
        case DataDialogMode::MODE_UPDATE_ALL: m_drawContentUpdate(vPos); break;
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void AccountDialog::m_prepare() {
    m_UpdateBanks();
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_BanksCombo.Select(m_Account.bank);
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
            m_BanksCombo.Select(m_Account.bank);
            m_AccountNameInputText.SetText(m_Account.name);
            m_AccountTypeInputText.SetText(m_Account.type);
            m_AccountNumberInputText.SetText(m_Account.number);
            m_AccountBaseSoldeInputDouble = m_Account.base_solde;
        } break;
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

const char* AccountDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    switch (mode) {
        case DataDialogMode::MODE_CREATION: return "Account Creation"; break;
        case DataDialogMode::MODE_DELETE_ONCE: return "Account Deletion"; break;
        case DataDialogMode::MODE_DELETE_ALL: return "Accounts Deletion"; break;
        case DataDialogMode::MODE_UPDATE_ONCE: return "Account Update"; break;
        case DataDialogMode::MODE_UPDATE_ALL: return "Accounts Update"; break;
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return "";
}

bool AccountDialog::m_canConfirm() {
    return !m_AccountNameInputText.empty() &&  //
        !m_AccountTypeInputText.empty() &&     //
        !m_AccountNumberInputText.empty();
}

void AccountDialog::m_confirmDialog() {
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
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    DataBrokers::Instance()->RefreshDatas();
}

void AccountDialog::m_cancelDialog() {
}

void AccountDialog::m_confirmDialogCreation() {
    if (DataBase::Instance()->OpenDBFile()) {
        DataBase::Instance()->AddAccount(        //
            m_BanksCombo.GetText(),              //
            m_BankAgencyInputText.GetText(),     //
            m_AccountTypeInputText.GetText(),    //
            m_AccountNameInputText.GetText(),    //
            m_AccountNumberInputText.GetText(),  //
            m_AccountBaseSoldeInputDouble);
        DataBase::Instance()->CloseDBFile();
        DataBrokers::Instance()->RefreshDatas();
    }
}

void AccountDialog::m_drawContentCreation(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_BanksCombo.DisplayCombo(width, "Bank Name", align);
    m_AccountNameInputText.DisplayInputText(width, "Account Name", "", false, align, true);
    m_AccountTypeInputText.DisplayInputText(width, "Account Type", "", false, align, true);
    m_AccountNumberInputText.DisplayInputText(width, "Account Number", "", false, align, true);
    m_DisplayAlignedWidget(width, "Base Solde", align, [this]() { ImGui::InputDouble("##BaseSolde", &m_AccountBaseSoldeInputDouble); });
}

void AccountDialog::m_confirmDialogUpdate() {
    if (DataBase::Instance()->OpenDBFile()) {
        DataBase::Instance()->UpdateAccount(     //
            m_Account.id,                        //
            m_BanksCombo.GetText(),              //
            m_BankAgencyInputText.GetText(),     //
            m_AccountTypeInputText.GetText(),    //
            m_AccountNameInputText.GetText(),    //
            m_AccountNumberInputText.GetText(),  //
            m_AccountBaseSoldeInputDouble);
        DataBase::Instance()->CloseDBFile();
        DataBrokers::Instance()->RefreshDatas();
    }
}

void AccountDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const float& align = 125.0f;
    const auto& width = 400.0f;
    m_BanksCombo.DisplayCombo(width, "Bank Name", align);
    m_AccountNameInputText.DisplayInputText(width, "Account Name", "", false, align, true);
    m_AccountTypeInputText.DisplayInputText(width, "Account Type", "", false, align, true);
    m_AccountNumberInputText.DisplayInputText(width, "Account Number", "", false, align, true);
    m_DisplayAlignedWidget(width, "Base Solde", align, [this]() { ImGui::InputDouble("##BaseSolde", &m_AccountBaseSoldeInputDouble); });
}

void AccountDialog::m_confirmDialogDeletion() {
    CTOOL_DEBUG_BREAK;
}

void AccountDialog::m_drawContentDeletion(const ImVec2& vPos) {
    CTOOL_DEBUG_BREAK;
}

void AccountDialog::m_UpdateBanks() {
    m_BanksCombo.Clear();
    DataBase::Instance()->GetBanks(                                       //
        [this](const BankName& vUserName, const std::string& /*vUrl*/) {  //
            m_BanksCombo.GetArrayRef().push_back(vUserName);
        });
    m_BanksCombo.GetIndexRef() = 0;
}