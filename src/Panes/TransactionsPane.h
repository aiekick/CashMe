#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <imguipack.h>
#include <ezlibs/ezSingleton.hpp>
#include <Systems/FrameActionSystem.h>
#include <Threads/ImportWorkerThread.h>
#include <Frontend/Tables/TransactionsTable.h>

class ProjectFile;
class TransactionsPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(TransactionsPane)
private:
    TransactionsTable m_TransactionsTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:
    TransactionsPane();                              // Prevent construction
    TransactionsPane(const TransactionsPane&) = delete;  // Prevent construction by copying
    TransactionsPane& operator=(const TransactionsPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~TransactionsPane();  // Prevent unwanted destruction};
};
