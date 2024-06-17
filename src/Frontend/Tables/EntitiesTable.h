#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

class EntitiesTable : public ADataTable {
private:
    std::vector<Entity> m_Entities;

public:
    EntitiesTable();
    ~EntitiesTable();

    bool load();
    void unload();
    bool drawMenu();

protected:
    double m_getAmount(const size_t& vIdx) final;
    void m_drawContent(const size_t& vIdx, const double& vMaxAmount) final;
    size_t m_getItemsCount() final;
    void m_setupColumns() final;

private:
    void m_updateEntities();
};
