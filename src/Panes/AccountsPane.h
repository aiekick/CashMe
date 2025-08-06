#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/AccountsTable.h>

class ProjectFile;
class AccountsPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(AccountsPane)
private:
    AccountsTable m_AccountsTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:
    AccountsPane();                             // Prevent construction
    AccountsPane(const AccountsPane&) = delete;  // Prevent construction by copying
    AccountsPane& operator=(const AccountsPane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~AccountsPane();  // Prevent unwanted destruction};
};
