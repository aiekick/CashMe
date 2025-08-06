#pragma once

#include <imguipack.h>
#include <Headers/DatasDef.h>

class ADataTable {
private:
    const char* m_TableName;
    const int32_t m_ColummCount;
    ImGuiListClipper m_ListClipper;
    float m_TextHeight = 0.0f;
    std::set<RowID> m_selectedItems;
    int32_t m_CurrSelectedItemIdx = -1;
    int32_t m_LastSelectedItemIdx = -1;
    uint32_t m_accountID = 0; // un rowid dans sqlite est a 1
    AccountOutput m_currentAccount;
    std::map<BankName, std::map<BankAgency, std::map<AccountNumber, AccountOutput>>> m_Accounts;

public:
    explicit ADataTable(const char* vTableName, const int32_t& vCloumnsCount);
    virtual ~ADataTable() = default;

    virtual bool init();
    virtual void unit();
    virtual bool load();
    virtual void unload();
    virtual bool drawMenu();
    virtual void refreshDatas();
    
    void draw(const ImVec2& vSize);

protected:
    virtual size_t m_getItemsCount() const = 0;
    virtual RowID m_getItemRowID(const size_t& vIdx) const = 0;
    virtual void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) = 0;
    virtual void m_setupColumns() = 0;
    virtual void m_drawContextMenuContent();
    virtual void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID);
    virtual double m_computeMaxPrice();
    virtual bool m_drawMenu();
    virtual bool m_drawDebugMenu();
    virtual void m_updateDatas(const RowID& vAccountID);
    virtual void m_draw(const ImVec2& vSize);

protected:
    int32_t m_getColumnCount() const;
    RowID m_getAccountID() const;
    void m_updateAccounts();
    bool m_drawAccountMenu();
    void m_drawColumnSelectable(const size_t& vIdx, const RowID& vRowID, const std::string& vText);
    void m_drawColumnText(const std::string& vText);
    void m_drawColumnDebit(const double& vDebit);
    void m_drawColumnCredit(const double& vCredit);
    void m_drawColumnAmount(const double& vAmount);
    void m_drawColumnInt(const int32_t& vValue);
    void m_drawAmount(const double& vAmount);
    const std::set<RowID>& m_getSelectedRows();
    void m_selectRows(const size_t& vStartIdx, const size_t& vEndIdx);
    void m_ResetSelection();
    void m_selectRow(const RowID& vRowID);
    bool m_isRowSelected(const RowID& vRowID) const;
    const ImGuiListClipper& m_GetListClipper() const;
    float m_GetTextHeight() const;
    const AccountOutput& m_getAccount() const;
    AccountOutput m_getAccount(const BankName& vBankName, const BankAgency& vBankAgency, const AccountNumber& vAccountNumber) const;

private:
    void m_selectOrDeselectRow(const RowID& vRowID);
    void m_showContextMenu(const size_t& vIdx);
};
