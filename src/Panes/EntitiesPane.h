#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/EntitiesTable.h>

class ProjectFile;
class EntitiesPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(EntitiesPane)
private:
    EntitiesTable m_EntitiesTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:
    EntitiesPane();                             // Prevent construction
    EntitiesPane(const EntitiesPane&) = delete;  // Prevent construction by copying
    EntitiesPane& operator=(const EntitiesPane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~EntitiesPane();  // Prevent unwanted destruction};
};
