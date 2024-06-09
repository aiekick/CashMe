#pragma once

#include <ImGuiPack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class BuySellPane : public AbstractPane {
private:
    ImWidgets::QuickStringEditCombo m_EditCombo;

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

public:  // singleton
    static std::shared_ptr<BuySellPane> Instance() {
        static std::shared_ptr<BuySellPane> _instance = std::make_shared<BuySellPane>();
        return _instance;
    }

public:
    BuySellPane();                              // Prevent construction
    BuySellPane(const BuySellPane&) = default;  // Prevent construction by copying
    BuySellPane& operator=(const BuySellPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~BuySellPane();  // Prevent unwanted destruction};
};
