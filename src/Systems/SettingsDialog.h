#pragma once

#include <ezlibs/ezXmlConfig.hpp>
#include <apis/CashMePluginApi.h>
#include <ezlibs/ezSingleton.hpp>

class SettingsDialog : public ez::xml::Config {
    IMPLEMENT_SINGLETON(SettingsDialog)
private:
    std::map<Cash::SettingsCategoryPath, Cash::ISettingsWeak> m_SettingsPerCategoryPath;
    bool m_ShowDialog = false;
    Cash::SettingsCategoryPath m_selectedCategoryPath;
    bool m_isHiddenMode = false;

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
};
