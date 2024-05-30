#include "TransactionDialog.h"
#include <Models/DataBase.h>
#include <Models/DataBrokers.h>
#include <ctools/cTools.h>

TransactionDialog::TransactionDialog() : ADataDialog("TransactionModalPopup") {
}

bool TransactionDialog::init() {
    return true;
}

void TransactionDialog::unit() {

}

void TransactionDialog::setTransaction(const Transaction& vTransaction) {
    m_TransactionToUpdate = vTransaction;
}

void TransactionDialog::setRowsToDelete(const std::vector<Transaction>& vTransactions) {
    CTOOL_DEBUG_BREAK;
}

void TransactionDialog::m_drawContent(const ImVec2& vPos) {
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

void TransactionDialog::m_drawContentCreation(const ImVec2& vPos) {
    const auto align = 125.0f;
    const auto width = 400.0f;
    m_DisplayAlignedWidget(width, "Account Number", align, [this]() { ImGui::Text("%s", m_AccountsCombo.GetText().c_str()); });
    m_CategoriesCombo.DisplayCombo(width, "Category", align);
    m_OperationsCombo.DisplayCombo(width, "Operation", align);
    m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
    m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
    m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", false, align);
    m_DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble); });
}

void TransactionDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const auto align = 125.0f;
    const auto width = 400.0f;
    m_DisplayAlignedWidget(width, "Account Number", align, [this]() { ImGui::Text("%s", m_AccountsCombo.GetText().c_str()); });
    m_CategoriesCombo.DisplayCombo(width, "Category", align);
    m_OperationsCombo.DisplayCombo(width, "Operation", align);
    m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
    m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
    m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", false, align);
    m_DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble); });
}

void TransactionDialog::m_drawContentDeletion(const ImVec2& vPos) {
}

void TransactionDialog::m_prepare() {
    m_AccountsCombo.Select(m_TransactionToUpdate.account);
    m_CategoriesCombo.Select(m_TransactionToUpdate.category);
    m_OperationsCombo.Select(m_TransactionToUpdate.operation);
    m_TransactionDateInputText.SetText(m_TransactionToUpdate.date);
    m_TransactionDescriptionInputText.SetText(m_TransactionToUpdate.description);
    m_TransactionCommentInputText.SetText(m_TransactionToUpdate.comment);
    m_TransactionAmountInputDouble = m_TransactionToUpdate.amount;
}

const char* TransactionDialog::m_getTitle() const {
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

bool TransactionDialog::m_canConfirm() {
    return true;
}

void TransactionDialog::m_confirmDialog() {
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

void TransactionDialog::m_cancelDialog() {
}

void TransactionDialog::m_confirmDialogCreation() {
    RowID account_id = 0U;
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.GetText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            const auto hash = ct::toStr(  //
                "%s_%s_%f",               //
                m_TransactionDateInputText.GetText().c_str(),
                // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                // alors on limite le hash a utiliser un label de 30
                // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                m_TransactionAmountInputDouble);              // must be unique per oepration
            DataBase::Instance()->AddTransaction(             //
                account_id,                                   //
                m_OperationsCombo.GetText(),                  //
                m_CategoriesCombo.GetText(),                  //
                m_SourceName,                                 //
                m_SourceType,                                 //
                m_SourceSha1,                                 //
                m_TransactionDateInputText.GetText(),         //
                m_TransactionDescriptionInputText.GetText(),  //
                m_TransactionCommentInputText.GetText(),      //
                m_TransactionAmountInputDouble,               //
                false,                                        //
                hash);
            DataBase::Instance()->CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogUpdate() {
    RowID account_id = 0U;
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.GetText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            const auto hash = ct::toStr(  //
                "%s_%s_%f",               //
                m_TransactionDateInputText.GetText().c_str(),
                // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                // alors on limite le hash a utiliser un label de 30
                // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                m_TransactionAmountInputDouble);              // must be unique per operation
            DataBase::Instance()->UpdateTransaction(          //
                m_TransactionToUpdate.id,                     //
                m_OperationsCombo.GetText(),                  //
                m_CategoriesCombo.GetText(),                  //
                m_SourceName,                                 //
                m_TransactionDateInputText.GetText(),         //
                m_TransactionDescriptionInputText.GetText(),  //
                m_TransactionCommentInputText.GetText(),      //
                m_TransactionAmountInputDouble,               //
                false,                                        //
                hash);
            DataBase::Instance()->CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogDeletion() {
    CTOOL_DEBUG_BREAK;
}
