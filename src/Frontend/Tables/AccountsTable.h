#pragma once

#include <Frontend/Dialogs/AccountDialog.h>

#include <Frontend/Tables/abstract/ADataBarsTable.h>

class AccountsTable : public ADataBarsTable {
private:
    std::vector<AccountOutput> m_Accounts;
    AccountDialog m_AccountDialog;

public:
    AccountsTable();
    ~AccountsTable() = default;

    bool load() final;
    void unload() final;
    bool drawMenu() final;
    AccountDialog& getAccountDialogRef();

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    double m_getItemBarAmount(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;

private:
    void m_updateAccounts();
};
