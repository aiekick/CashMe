#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.h>

class TransactionDialog : public ADataDialog {
private:
    TransactionOutput m_TransactionToUpdate;

    // widgets : Read Only
    SourceName m_SourceName;
    SourceType m_SourceType;
    SourceSha m_SourceSha;

    // widgets : Read / Write
    double m_TransactionAmountInputDouble = 0.0;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    ImWidgets::QuickStringEditCombo m_EntitiesCombo;
    ImWidgets::QuickStringEditCombo m_CategoriesCombo;
    ImWidgets::QuickStringEditCombo m_OperationsCombo;
    //ImWidgets::QuickStringEditCombo m_IncomesCombo;
    ImWidgets::InputText m_TransactionDateInputText;
    ImWidgets::InputText m_TransactionDescriptionInputText;
    ImWidgets::InputText m_TransactionCommentInputText;
    bool m_TransactionConfirmed = false;  // need to have a three state checkbox
    bool m_TransactionConfirmedManyValues = false;

    // transactions to update / delete
    std::vector<TransactionOutput> m_TransactionsToUpdate;
    std::vector<TransactionOutput> m_TransactionsToDelete;
    ImGuiListClipper m_TransactionsDeletionListClipper;

public:
    TransactionDialog();
    bool init() override;
    void unit() override;

    void setTransaction(const TransactionOutput& vTransaction);
    void setTransactionsToUpdate(const std::vector<TransactionOutput>& vTransactions);
    void setTransactionsToDelete(const std::vector<TransactionOutput>& vTransactions);

protected:
    void m_drawContent(const ImVec2& vPos) override;
    void m_prepare() override;
    const char* m_getTitle() const override;
    bool m_canConfirm() override;
    void m_confirmDialog() override;
    void m_cancelDialog() override;

    void m_confirmDialogCreation();
    void m_drawContentCreation(const ImVec2& vPos);

    void m_confirmDialogUpdateOnce();
    void m_confirmDialogUpdateAll();
    void m_drawContentUpdate(const ImVec2& vPos);

    void m_confirmDialogDeletion();
    void m_drawContentDeletion(const ImVec2& vPos);

    void m_updateAccounts();
    void m_updateEntities();
    void m_updateOperations();
    void m_updateCategories();
    void m_updateIncomes();
};
