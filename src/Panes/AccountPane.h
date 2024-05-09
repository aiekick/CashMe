#pragma once

#include <ImGuiPack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class AccountPane : public AbstractPane {
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
    static std::shared_ptr<AccountPane> Instance() {
        static std::shared_ptr<AccountPane> _instance = std::make_shared<AccountPane>();
        return _instance;
    }

public:
    AccountPane();                              // Prevent construction
    AccountPane(const AccountPane&) = default;  // Prevent construction by copying
    AccountPane& operator=(const AccountPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~AccountPane();  // Prevent unwanted destruction};
};
