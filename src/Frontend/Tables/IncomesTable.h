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
        std::vector<IncomeOutput> incomes;
        double minAmount = 0.0;
        double maxAmount = 0.0;
        void clear() {
            accounts.clear();
            accountNumbers.clear();
            incomes.clear();
            minAmount = 0.0;
            maxAmount = 0.0;
        }
    } m_Datas;
    // accounts display
    std::map<BankName, std::map<BankAgency, std::map<AccountNumber, AccountOutput>>> m_Accounts;

public:
    IncomesTable();
    ~IncomesTable() = default;

    bool init();
    void unit();
    void clear();

    void refreshDatas() final;

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;
    bool m_drawMenu() final;
};
