#pragma once

#include <imguipack.h>
#include <cstdint>
#include <memory>
#include <string>
#include <ezlibs/ezSingleton.hpp>
#include <Frontend/Tables/CategoriesTable.h>

class ProjectFile;
class CategoriesPane : public AbstractPane {
    IMPLEMENT_SHARED_SINGLETON(CategoriesPane)
private:
    CategoriesTable m_CategoriesTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:
    CategoriesPane();                             // Prevent construction
    CategoriesPane(const CategoriesPane&) = delete;  // Prevent construction by copying
    CategoriesPane& operator=(const CategoriesPane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~CategoriesPane();  // Prevent unwanted destruction};
};
