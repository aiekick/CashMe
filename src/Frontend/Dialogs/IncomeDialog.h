#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.h>

class IncomeDialog : public ADataDialog {
private:
    IncomeOutput m_IncomeToUpdate;

    // widgets : Read Only
    SourceName m_SourceName;
    SourceType m_SourceType;
    SourceSha m_SourceSha;

    // widgets : Read / Write
    ImWidgets::InputValue<double> m_IncomeMinAmountInputDouble;
    ImWidgets::InputValue<double> m_IncomeMaxAmountInputDouble;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    ImWidgets::QuickStringEditCombo m_EntitiesCombo;
    ImWidgets::QuickStringEditCombo m_CategoriesCombo;
    ImWidgets::QuickStringEditCombo m_OperationsCombo;
    ImWidgets::InputText m_IncomeNameInputText;
    ImWidgets::InputText m_IncomeStartDateInputText;
    ImWidgets::InputText m_IncomeEndDateInputText;
    ImWidgets::InputValue<int32_t> m_IncomeMinDayInputInt32;
    ImWidgets::InputValue<int32_t> m_IncomeMaxDayInputInt32;
    ImWidgets::InputText m_IncomeDescriptionInputText;
    bool m_IncomeOptional = false;

    // transactions to add as incomes
    std::vector<TransactionOutput> m_TransactionToAddAsIncomes;

    // transactions to update / delete
    std::vector<IncomeOutput> m_IncomesToUpdate;
    std::vector<IncomeOutput> m_IncomesToDelete;
    ImGuiListClipper m_IncomesDeletionListClipper;

public:
    IncomeDialog();
    bool init() override;
    void unit() override;

    void setTransactions(const std::vector<TransactionOutput>& vTransactions);
    void setIncome(const IncomeOutput& vIncome);
    void setIncomesToUpdate(const std::vector<IncomeOutput>& vIncomes);
    void setIncomesToDelete(const std::vector<IncomeOutput>& vIncomes);

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
    void m_drawContentUpdate(const ImVec2& vPos);

    void m_confirmDialogDeletion();
    void m_drawContentDeletion(const ImVec2& vPos);

    void m_updateAccounts();
    void m_updateEntities();
    void m_updateOperations();
    void m_updateCategories();
};
