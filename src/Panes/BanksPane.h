#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/BanksTable.h>

class ProjectFile;
class BanksPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(BanksPane)
private:
    BanksTable m_BanksTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:
    BanksPane();                             // Prevent construction
    BanksPane(const BanksPane&) = delete;  // Prevent construction by copying
    BanksPane& operator=(const BanksPane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~BanksPane();  // Prevent unwanted destruction};
};
