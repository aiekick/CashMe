#pragma once

#include <ImGuiPack.h>
#include <Headers/DatasDef.h>

class ADataTable {
private:
    const char* m_TableName;
    const int32_t m_ColummCount;
    ImWidgets::QuickStringCombo m_AccountsCombo;
    std::vector<Account> m_Accounts;
    ImGuiListClipper m_ListClipper;
    float m_TextHeight = 0.0f;
    ImVec4 m_BadColor = ImVec4(1, 0, 0, 1);
    ImVec4 m_GoodColor = ImVec4(0, 1, 0, 1);

public:
    explicit ADataTable(const char* vTableName, const int32_t& vCloumnsCount);
    virtual ~ADataTable() = default;
    virtual bool load();
    virtual void unload();
    virtual bool drawMenu();
    void draw(const ImVec2& vSize);

protected:
    virtual double m_getAmount(const size_t& vIdx) = 0;
    virtual void m_drawContent(const size_t& vIdx, const double& vMaxAmount) = 0;
    virtual size_t m_getItemsCount() = 0;
    virtual void m_setupColumns() = 0;
    RowID m_getAccountID();
    void m_updateAccounts();
    void m_drawColumnDebit(const double& vDebit);
    void m_drawColumnCredit(const double& vCredit);
    void m_drawColumnAmount(const double& vAmount);
    void m_drawColumnBars(const double vAmount, const double vMaxAmount);

private:
    double m_computeMaxPrice();
};
