#pragma once

#include <ImGuiPack.h>
#include <cstdint>
#include <memory>
#include <string>

#include <Frontend/Tables/EntitiesTable.h>

class ProjectFile;
class EntitiesPane : public AbstractPane {
private:
    EntitiesTable m_EntitiesTable;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void Load();

public:  // singleton
    static std::shared_ptr<EntitiesPane> Instance() {
        static std::shared_ptr<EntitiesPane> _instance = std::make_shared<EntitiesPane>();
        return _instance;
    }

public:
    EntitiesPane();                             // Prevent construction
    EntitiesPane(const EntitiesPane&) = delete;  // Prevent construction by copying
    EntitiesPane& operator=(const EntitiesPane&) {
        return *this;
    };                      // Prevent assignment
    virtual ~EntitiesPane();  // Prevent unwanted destruction};
};
