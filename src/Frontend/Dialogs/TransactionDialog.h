#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.hpp>

class TransactionDialog : public ADataDialog<Transaction> {
private:
    Transaction m_TransactionToUpdate;

    // widgets : Read Only
    SourceName m_SourceName;
    SourceType m_SourceType;
    SourceSha1 m_SourceSha1;

    // widgets : Read / Write
    double m_TransactionAmountInputDouble = 0.0;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    ImWidgets::QuickStringCombo m_CategoriesCombo;
    ImWidgets::QuickStringCombo m_OperationsCombo;
    ImWidgets::InputText m_TransactionDateInputText;
    ImWidgets::InputText m_TransactionDescriptionInputText;
    ImWidgets::InputText m_TransactionCommentInputText;

public:
    TransactionDialog();
    bool init() override;
    void unit() override;

    void setTransaction(const Transaction& vTransaction);
    void setRowsToDelete(const std::vector<Transaction>& vTransactions);

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

    void m_confirmDialogDeletion();
    void m_drawContentDeletion(const ImVec2& vPos);
};
