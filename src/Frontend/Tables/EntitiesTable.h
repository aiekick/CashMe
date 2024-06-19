#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

class EntitiesTable : public ADataTable {
private:
    std::vector<Entity> m_Entities;

public:
    EntitiesTable();
    ~EntitiesTable();

    bool load() final;
    void unload() final;
    bool drawMenu() final;

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    double m_getItemBarAmount(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;

private:
    void m_updateEntities();
};
