#pragma once

#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.hpp>

class AccountDialog : public ADataDialog<Account> {
private:
    Account m_Account;
    ImWidgets::QuickStringEditCombo m_BanksCombo;
    ImWidgets::InputText m_BankAgencyInputText;
    ImWidgets::InputText m_AccountNameInputText;
    ImWidgets::InputText m_AccountTypeInputText;
    ImWidgets::InputText m_AccountNumberInputText;
    double m_AccountBaseSoldeInputDouble = 0.0;
    std::vector<BankName> m_BankNames;

public:
    AccountDialog();
    bool init() override;
    void unit() override;
    void setAccount(const Account& vAccount);

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

    void m_UpdateBanks();
};
