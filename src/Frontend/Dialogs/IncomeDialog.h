#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.h>

class IncomeDialog : public ADataDialog {
private:
    Income m_IncomeToUpdate;

    // widgets : Read Only
    SourceName m_SourceName;
    SourceType m_SourceType;
    SourceSha m_SourceSha;

    // widgets : Read / Write
    IncomeAmount m_IncomeMinAmountInputDouble = 0.0;
    IncomeAmount m_IncomeMaxAmountInputDouble = 0.0;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    ImWidgets::QuickStringEditCombo m_EntitiesCombo;
    ImWidgets::QuickStringEditCombo m_CategoriesCombo;
    ImWidgets::QuickStringEditCombo m_OperationsCombo;
    ImWidgets::InputText m_IncomeNameInputText;
    ImWidgets::InputText m_IncomeStartDateInputText;
    ImWidgets::InputText m_IncomeEndDateInputText;
    IncomeDelayDays m_IncomeMinDayInputInt32 = 0;
    IncomeDelayDays m_IncomeMaxDayInputInt32 = 0;
    ImWidgets::InputText m_IncomeDescriptionInputText;
    ImWidgets::InputText m_IncomeCommentInputText;

    // transactions to add as incomes
    std::vector<Transaction> m_TransactionToAddAsIncomes;

    // transactions to update / delete
    std::vector<Income> m_IncomesToUpdate;
    std::vector<Income> m_IncomesToDelete;
    ImGuiListClipper m_IncomesDeletionListClipper;

public:
    IncomeDialog();
    bool init() override;
    void unit() override;

    void setTransactions(const std::vector<Transaction>& vTransactions);
    void setIncome(const Income& vIncome);
    void setIncomesToUpdate(const std::vector<Income>& vIncomes);
    void setIncomesToDelete(const std::vector<Income>& vIncomes);

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

    void m_UpdateAccounts();
    void m_UpdateEntities();
    void m_UpdateOperations();
    void m_UpdateCategories();
};
