#pragma once

#include <Frontend/Dialogs/BankDialog.h>

#include <Frontend/Tables/abstract/ADataBarsTable.h>

class BanksTable : public ADataBarsTable {
private:
    std::vector<BankOutput> m_Banks;

public:
    BanksTable();
    ~BanksTable() = default;

    bool drawMenu() final;
    void refreshDatas() final;

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    double m_getItemBarAmount(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;
};
