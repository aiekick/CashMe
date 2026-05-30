#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezXmlConfig.hpp>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/BuySellTable.h>

class ProjectFile;
class BuySellPane : public AbstractPane, public ez::xml::Config {
    IMPLEMENT_SHARED_SINGLETON(BuySellPane)
private:
    BuySellTable m_buySellTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

public:
    BuySellPane();                             // Prevent construction
    BuySellPane(const BuySellPane&) = delete;  // Prevent construction by copying
    BuySellPane& operator=(const BuySellPane&) {
        return *this;
    };                        // Prevent assignment
    virtual ~BuySellPane();   // Prevent unwanted destruction};
};
