#include <Systems/SettingsDialog.h>

#include <ctools/cTools.h>

#include <Plugins/PluginManager.h>

#include <ImGuiPack/ImGuiPack.h>

#include <Project/ProjectFile.h>

bool SettingsDialog::init() {
    const auto& pluginSettings = PluginManager::Instance()->GetPluginSettings();
    for (const auto& s : pluginSettings) {
        auto ptr = s.settings.lock();
        if (ptr != nullptr) {
            m_SettingsPerCategoryPath[ptr->GetCategory()] = s.settings;
        }
    }
    return true;
}

void SettingsDialog::unit() {
    m_SettingsPerCategoryPath.clear();
}

void SettingsDialog::OpenDialog() {
    if (m_ShowDialog) {
        return;
    }
    m_Load();
    m_ShowDialog = true;
}

void SettingsDialog::CloseDialog() {
    m_ShowDialog = false;
}

bool SettingsDialog::Draw() {
    if (m_ShowDialog) {
        ImGui::Begin("Settings");
        {
            ImGui::Separator();
            m_DrawCategoryPanes();
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();
            m_DrawContentPane();
            ImGui::Separator();
            m_DrawButtonsPane();
        }
        ImGui::End();
        return true;
    }

    return false;
}
const bool& SettingsDialog::isHiddenMode() {
    return m_IsHiddenMode;
}

void SettingsDialog::m_DrawCategoryPanes() {
    const auto size = ImGui::GetContentRegionMax() - ImVec2(100, 68);

    ImGui::BeginChild("Categories", ImVec2(100, size.y));

    ImGui::EndChild();
}

void SettingsDialog::m_DrawContentPane() {
    auto size = ImGui::GetContentRegionMax() - ImVec2(100, 68);

    if (!ImGui::GetCurrentWindow()->ScrollbarY) {
        size.x -= ImGui::GetStyle().ScrollbarSize;
    }

    ImGui::BeginChild("##Content", size);

    for (const auto& cat : m_SettingsPerCategoryPath) {
        auto ptr = cat.second.lock();
        if (ptr != nullptr) {
            ptr->DrawSettings();
        }
    }

    ImGui::EndChild();
}

void SettingsDialog::m_DrawButtonsPane() {
    if (ImGui::ContrastedButton("Ok")) {
        m_Save();
        CloseDialog();
    }
    ImGui::SameLine();
    if (ImGui::ContrastedButton("Cancel")) {
        CloseDialog();
    }
}

bool SettingsDialog::m_Load() {
    for (const auto& cat : m_SettingsPerCategoryPath) {
        auto ptr = cat.second.lock();
        if (ptr != nullptr) {
            ptr->LoadSettings();
        }
    }
    return false;
}

bool SettingsDialog::m_Save() {
    for (const auto& cat : m_SettingsPerCategoryPath) {
        auto ptr = cat.second.lock();
        if (ptr != nullptr) {
            ptr->SaveSettings();
        }
    }
    ProjectFile::Instance()->SetProjectChange();
    return false;
}

std::string SettingsDialog::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    str += vOffset + "<plugins>\n";

    for (const auto& cat : m_SettingsPerCategoryPath) {
        auto ptr = cat.second.lock();
        if (ptr != nullptr) {
            if (vUserDatas == "app") {
                str += ptr->GetXmlSettings(vOffset + "\t", Cash::ISettingsType::APP);
            } else if (vUserDatas == "project") {
                str += ptr->GetXmlSettings(vOffset + "\t", Cash::ISettingsType::PROJECT);
            } else {
                CTOOL_DEBUG_BREAK; // ERROR
            }
        }
    }

    str += vOffset + "</plugins>\n";

    return str;
}

bool SettingsDialog::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName = "";
    std::string strValue = "";
    std::string strParentName = "";

    strName = vElem->Value();
    if (vElem->GetText()) {
        strValue = vElem->GetText();
    }
    if (vParent != nullptr) {
        strParentName = vParent->Value();
    }

    for (const auto& cat : m_SettingsPerCategoryPath) {
        auto ptr = cat.second.lock();
        if (ptr != nullptr) {
            if (vUserDatas == "app") {
                ptr->SetXmlSettings(strName, strParentName, strValue, Cash::ISettingsType::APP);
                RecursParsingConfigChilds(vElem, vUserDatas);
            } else if (vUserDatas == "project") {
                ptr->SetXmlSettings(strName, strParentName, strValue, Cash::ISettingsType::PROJECT);
                RecursParsingConfigChilds(vElem, vUserDatas);
            } else {
                CTOOL_DEBUG_BREAK;  // ERROR
            }
        }
    }

    return false;
}