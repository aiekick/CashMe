#pragma once

#include <Frontend/Tables/abstract/ADataBarsTable.h>
#include <Frontend/Dialogs/EntityDialog.h>

class EntitiesTable : public ADataBarsTable {
private:
    std::vector<Entity> m_Entities;
    EntityDialog m_EntityDialog;

public:
    EntitiesTable();
    ~EntitiesTable() = default;

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
