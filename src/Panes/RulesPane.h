#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/RulesTable.h>

class ProjectFile;
class RulesPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(RulesPane)
private:
    RulesTable m_rulesTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    RulesTable& getRulesTableRef();

public:
    RulesPane();                           // Prevent construction
    RulesPane(const RulesPane&) = delete;  // Prevent construction by copying
    RulesPane& operator=(const RulesPane&) {
        return *this;
    };                     // Prevent assignment
    virtual ~RulesPane();  // Prevent unwanted destruction
};
