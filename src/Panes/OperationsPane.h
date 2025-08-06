#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/OperationsTable.h>

class ProjectFile;
class OperationsPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(OperationsPane)
private:
    OperationsTable m_OperationsTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;


public:
    OperationsPane();                             // Prevent construction
    OperationsPane(const OperationsPane&) = delete;  // Prevent construction by copying
    OperationsPane& operator=(const OperationsPane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~OperationsPane();  // Prevent unwanted destruction};
};
