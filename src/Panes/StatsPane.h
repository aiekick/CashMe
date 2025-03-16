#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <imguipack.h>

#include <Headers/DatasDef.h>

class ProjectFile;
class StatsPane : public AbstractPane {
private:
    std::vector<Account> m_Accounts;
    std::vector<Entity> m_Entities;
    std::vector<Category> m_Categories;
    std::vector<Operation> m_Operations;

    size_t m_SelectedAccountIdx = 0U;

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
    void Load();

private:
    void m_refreshDatas();
    void m_drawMenu();
    void m_updateAccounts();
    void m_drawEntitiesStats();
    void m_updateEntities();
    void m_drawCategoriesStats();
    void m_updateCategories();
    void m_drawOperationsStats();
    void m_updateOperations();

public:  // singleton
    static std::shared_ptr<StatsPane> Instance() {
        static auto _instance = std::make_shared<StatsPane>();
        return _instance;
    }

public:
    StatsPane();                              // Prevent construction
    StatsPane(const StatsPane&) = delete;  // Prevent construction by copying
    StatsPane& operator=(const StatsPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~StatsPane();  // Prevent unwanted destruction};
};
