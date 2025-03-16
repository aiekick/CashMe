#pragma once

#include <imguipack.h>
#include <Headers/DatasDef.h>
#include <Frontend/Tables/abstract/ADataTable.h>

class ADataBarsTable : public ADataTable {
public:
    ADataBarsTable(const char* vTableName, const int32_t& vCloumnsCount);
    virtual ~ADataBarsTable() = default;

protected:
    virtual double m_getItemBarAmount(const size_t& vIdx) const = 0;

protected:
    double m_computeMaxPrice() final;
    void m_drawColumnBars(const double vAmount, const double vMaxAmount, const float vColumNWidth = -1.0f);
};
