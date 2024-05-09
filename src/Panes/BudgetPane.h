#pragma once

#include <ImGuiPack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class BudgetPane : public AbstractPane {
private:

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:  // singleton
    static std::shared_ptr<BudgetPane> Instance() {
        static std::shared_ptr<BudgetPane> _instance = std::make_shared<BudgetPane>();
        return _instance;
    }

public:
    BudgetPane();                              // Prevent construction
    BudgetPane(const BudgetPane&) = default;  // Prevent construction by copying
    BudgetPane& operator=(const BudgetPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~BudgetPane();  // Prevent unwanted destruction};
};
