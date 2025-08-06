#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>

class ProjectFile;
class ConsolePane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(ConsolePane)
public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vRect,  ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;


public:
    ConsolePane();                              // Prevent construction
    ConsolePane(const ConsolePane&) = delete;  // Prevent construction by copying
    ConsolePane& operator=(const ConsolePane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~ConsolePane();  // Prevent unwanted destruction};
};
