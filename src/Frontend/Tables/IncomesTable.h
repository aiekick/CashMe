#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

#include <Systems/FrameActionSystem.h>

#include <Threads/ImportWorkerThread.h>

#include <Frontend/Dialogs/IncomeDialog.h>

class IncomesTable : public ADataTable {
private:
    struct Datas {
        std::vector<AccountOutput> accounts;
        std::vector<AccountNumber> accountNumbers;
        std::vector<Income> incomes;
        void clear() {
            accounts.clear();
            accountNumbers.clear();
            incomes.clear();
        }
    } m_Datas;
    IncomeDialog m_IncomeDialog;
    // accounts display
    std::map<BankName, std::map<BankAgency, std::map<AccountNumber, AccountOutput>>> m_Accounts;

public:
    IncomesTable();
    ~IncomesTable() = default;

    bool init();
    void unit();

    bool load() final;
    void unload() final;
    bool drawMenu() final;

    IncomeDialog& getIncomeDialogRef();

    void clear();
    void refreshDatas();
    void drawDebugMenu(FrameActionSystem& vFrameActionSystem);
    void drawAccountsMenu(FrameActionSystem& vFrameActionSystem);

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;

    void m_UpdateAccounts();
    void m_UpdateIncomes(const RowID& vAccountID);
};
