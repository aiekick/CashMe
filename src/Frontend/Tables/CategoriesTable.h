#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

class CategoriesTable : public ADataTable {
private:
    std::vector<Category> m_Categories;

public:
    CategoriesTable();
    ~CategoriesTable();

    bool load();
    void unload();
    bool drawMenu();

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    double m_getItemAmount(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick() final;

private:
    void m_updateCategories();
};
