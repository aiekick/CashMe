#pragma once

#include <vector>
#include <string>
#include <Frontend/Tables/abstract/ADataTable.h>

// read-only list of categorization rules (one row per rule), stored in the sqlite database.
// editing is done in the modal RuleDialog ; this table only displays + dispatches actions.
class RulesTable : public ADataTable {
private:
    std::vector<CategorizationRule> m_rules;  // cache loaded from the database
    bool m_needRefresh = false;               // deferred reload (avoids mutating the cache mid-draw)

public:
    RulesTable();
    ~RulesTable() = default;

    void clear();
    void refreshDatas() final;

    void addRule(const CategorizationRule& vRule);     // inserts into the database
    void updateRule(const CategorizationRule& vRule);  // updates by id in the database
    void removeRule(const RowID& vId);                 // deletes by id in the database
    const std::vector<CategorizationRule>& getRules() const;

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_draw(const ImVec2& vSize) final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;
    bool m_drawMenu() final;

private:
    void m_runPreview(const std::vector<CategorizationRule>& vRules);
    std::string m_amountRangeText(const CategorizationRule& vRule) const;
};
