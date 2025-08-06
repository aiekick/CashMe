#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/IncomesTable.h>

class ProjectFile;
class IncomesPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(IncomesPane)
private:
    IncomesTable m_IncomesTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:
    IncomesPane();                             // Prevent construction
    IncomesPane(const IncomesPane&) = delete;  // Prevent construction by copying
    IncomesPane& operator=(const IncomesPane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~IncomesPane();  // Prevent unwanted destruction};
};
