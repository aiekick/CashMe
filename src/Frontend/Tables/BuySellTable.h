#pragma once

#include <vector>
#include <Frontend/Tables/abstract/ADataTable.h>
#include <Frontend/Graph/BuySellBarsGraph.h>
#include <Frontend/Graph/BuySellPieGraph.h>
#include <ezlibs/ezXmlConfig.hpp>

class BuySellTable : public ADataTable, public ez::xml::Config {
private:
    BuySellBarsGraph m_barsGraph;
    BuySellPieGraph m_pieGraph;
    std::vector<BuySellStatItem> m_items;
    SearchColumns m_field{SEARCH_COLUMN_CATEGORY};
    ImWidgets::InputText m_filterInput;
    bool m_useDayRange{false};
    int32_t m_dayStart{1};
    int32_t m_dayEnd{31};
    int32_t m_monthsWindow{6};
    int32_t m_sortColumn{-1};
    bool m_sortAscending{true};

public:
    BuySellTable();
    ~BuySellTable() = default;

    void clear();
    void refreshDatas() final;

    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_draw(const ImVec2& vSize) final;
    ImGuiTableFlags m_getTableFlags() const final;
    void m_sortDatas(ImGuiTableSortSpecs* vSortSpecs) final;

private:
    bool m_drawControls();
    void m_applySort();
};
