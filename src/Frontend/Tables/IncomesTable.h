#pragma once

#include <Frontend/Tables/abstract/ADataTable.h>

#include <Systems/FrameActionSystem.h>

#include <Threads/ImportWorkerThread.h>

#include <Frontend/Dialogs/IncomeDialog.h>

class IncomesTable : public ADataTable {
private:
    std::vector<Income> m_Incomes;

    IncomeDialog m_IncomeDialog;

public:
    IncomesTable();
    ~IncomesTable() = default;

    bool init();
    void unit();

    bool load() final;
    void unload() final;
    bool drawMenu() final;

    IncomeDialog& getIncomeDialogRef();

    void clear();    
    void refreshDatas();
    void drawDebugMenu(FrameActionSystem& vFrameActionSystem);

protected:
    size_t m_getItemsCount() const final;
    RowID m_getItemRowID(const size_t& vIdx) const final;
    void m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) final;
    void m_setupColumns() final;
    void m_drawContextMenuContent() final;
    void m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) final;
};
