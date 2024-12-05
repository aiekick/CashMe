#pragma once

#include <ezlibs/ezXmlConfig.hpp>
#include <apis/CashMePluginApi.h>

class SettingsDialog : public ez::xml::Config {
private:
    std::map<Cash::SettingsCategoryPath, Cash::ISettingsWeak> m_SettingsPerCategoryPath;
    bool m_ShowDialog = false;
    Cash::SettingsCategoryPath m_SelectedCategoryPath;
    bool m_IsHiddenMode = false;

public:
    bool init();
    void unit();

    void OpenDialog();
    void CloseDialog();

    bool Draw();

    ez::xml::Nodes getXmlNodes(const std::string& vUserDatas = "") override;
    bool setFromXmlNodes(const ez::xml::Node& vNode, const ez::xml::Node& vParent, const std::string& vUserDatas) override;

    const bool& isHiddenMode();

private:
    void m_DrawCategoryPanes();
    void m_DrawContentPane();
    void m_DrawButtonsPane();
    bool m_Load();
    bool m_Save();

public:  // singleton
    static SettingsDialog* Instance() {
        static SettingsDialog _instance;
        return &_instance;
    }
};
