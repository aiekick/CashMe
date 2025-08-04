#pragma once

#include <imguipack.h>
#include <Headers/DatasDef.h>

class ADataTable {
private:
    const char* m_TableName;
    const int32_t m_ColummCount;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    std::vector<AccountOutput> m_Accounts;
    ImGuiListClipper m_ListClipper;
    float m_TextHeight = 0.0f;
    std::set<RowID> m_SelectedItems;
    int32_t m_CurrSelectedItemIdx = -1;
    int32_t m_LastSelectedItemIdx = -1;

public:
    explicit ADataTable(const char* vTableName, const int32_t& vCloumnsCount);
    virtual ~ADataTable() = default;

    virtual bool init();
    virtual void unit();
    virtual bool load();
    virtual void unload();
    virtual bool drawMenu() = 0;
    
    void draw(const ImVec2& vSize);

protected:
    virtual size_t m_getItemsCount() const = 0;
    virtual RowID m_getItemRowID(const size_t& vIdx) const = 0;
    virtual void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) = 0;
    virtual void m_setupColumns() = 0;
    virtual void m_drawContextMenuContent() = 0;
    virtual void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) = 0;
    virtual double m_computeMaxPrice();

protected:
    RowID m_getAccountID();
    void m_updateAccounts();
    bool m_drawAccountMenu();
    void m_drawColumnSelectable(const size_t& vIdx, const RowID& vRowID, const std::string& vText);
    void m_drawColumnText(const std::string& vText);
    void m_drawColumnDebit(const double& vDebit);
    void m_drawColumnCredit(const double& vCredit);
    void m_drawColumnAmount(const double& vAmount);
    void m_drawColumnInt(const int32_t& vValue);
    const std::set<RowID>& m_getSelectedRows();
    void m_selectRows(const size_t& vStartIdx, const size_t& vEndIdx);
    ImWidgets::QuickStringCombo& m_getAccountComboRef();
    void m_ResetSelection();
    void m_SelectRow(const RowID& vRowID);
    bool m_IsRowSelected(const RowID& vRowID) const;
    const ImGuiListClipper& m_GetListClipper() const;
    float m_GetTextHeight() const;

private:
    void m_SelectOrDeselectRow(const RowID& vRowID);
    void m_showContextMenu(const size_t& vIdx);
};
