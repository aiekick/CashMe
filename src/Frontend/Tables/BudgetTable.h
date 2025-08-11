#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>
#include <Frontend/Graph/BudgetGraph.h>
#include <Models/BudgetComputer.h>
#include <ezlibs/ezXmlConfig.hpp>

class BudgetTable : public ADataTable, public ez::xml::Config {
private:
    BudgetGraph m_budgetGraph;
    BudgetComputer m_budgetComputer;
    bool m_showTable{true};
    bool m_showGraph{true};
    struct Params {
        int32_t pastMonthOffset{0};
        uint32_t futurMonthOffset{3};
        bool useOptional{true};
    } m_params;

public:
    BudgetTable();
    ~BudgetTable() = default;

    void clear();
    void refreshDatas() final;

    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    bool m_drawMenu() final;
    void m_draw(const ImVec2& vSize) final;
};
