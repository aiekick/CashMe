/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ProjectFile.h"

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>

#include <LayoutManager.h>
#include <Models/DataBase.h>
#include <Panes/AccountPane.h>
#include <Plugins/PluginManager.h>
#include <Systems/SettingsDialog.h>

ProjectFile::ProjectFile() = default;

ProjectFile::ProjectFile(const std::string& vFilePathName) {
    m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
    auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
    if (ps.isOk) {
        m_ProjectFileName = ps.name;
        m_ProjectFilePath = ps.path;
    }
}

ProjectFile::~ProjectFile() = default;

void ProjectFile::Clear() {
    m_ProjectFilePathName.clear();
    m_ProjectFileName.clear();
    m_ProjectFilePath.clear();
    m_IsLoaded = false;
    m_IsThereAnyChanges = false;
    Messaging::Instance()->Clear();
}

void ProjectFile::ClearDatas() {

}

void ProjectFile::New() {
    Clear();
    ClearDatas();
    m_IsLoaded = true;
    m_NeverSaved = true;
    SetProjectChange(true);
}

void ProjectFile::New(const std::string& vFilePathName) {
    Clear();
    ClearDatas();
    m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
    DataBase::Instance()->CreateDBFile(m_ProjectFilePathName);
    auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
    if (ps.isOk) {
        m_ProjectFileName = ps.name;
        m_ProjectFilePath = ps.path;
    }
    m_IsLoaded = true;
    SetProjectChange(false);
}

bool ProjectFile::Load() {
    return LoadAs(m_ProjectFilePathName);
}

// ils wanted to not pass the adress for re open case
// elwse, the clear will set vFilePathName to empty because with re open, target m_ProjectFilePathName
bool ProjectFile::LoadAs(const std::string vFilePathName) {
    if (!vFilePathName.empty()) {
        Clear();
        std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
        if (DataBase::Instance()->IsFileASqlite3DB(filePathName)) {
            if (DataBase::Instance()->OpenDBFile(filePathName)) {
                ClearDatas();
                auto xml_settings = DataBase::Instance()->GetSettingsXMLDatas();
                tinyxml2::XMLError xmlError = LoadConfigString(unEscapeXmlCode(xml_settings));
                if (xml_settings.empty() || xmlError == tinyxml2::XMLError::XML_SUCCESS) {
                    m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
                    auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
                    if (ps.isOk) {
                        m_ProjectFileName = ps.name;
                        m_ProjectFilePath = ps.path;
                    }
                    m_IsLoaded = true;
                    AccountPane::Instance()->load();
                    SetProjectChange(false);
                } else {
                    Clear();
                    auto errMsg = getTinyXml2ErrorMessage(xmlError);
                    LogVarError("The project file % s cant be loaded, Error : % s", filePathName.c_str(), errMsg.c_str());
                }
                DataBase::Instance()->CloseDBFile();
            }
        }
    }
    return m_IsLoaded;
}

bool ProjectFile::Save() {
    if (m_NeverSaved) {
        return false;
    }

    if (DataBase::Instance()->OpenDBFile(m_ProjectFilePathName)) {
        auto xml_settings = escapeXmlCode(SaveConfigString());
        if (DataBase::Instance()->SetSettingsXMLDatas(xml_settings)) {
            SetProjectChange(false);
            DataBase::Instance()->CloseDBFile();
            return true;
        }
        DataBase::Instance()->CloseDBFile();
    }	

    return false;
}

bool ProjectFile::SaveAs(const std::string& vFilePathName) {
    std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
    auto ps = FileHelper::Instance()->ParsePathFileName(filePathName);
    if (ps.isOk) {
        m_ProjectFilePathName = FileHelper::Instance()->ComposePath(ps.path, ps.name, PROJECT_EXT_DOT_LESS);
        m_ProjectFilePath = ps.path;
        m_NeverSaved = false;
        return Save();
    }
    return false;
}

bool ProjectFile::IsProjectLoaded() const {
    return m_IsLoaded;
}

bool ProjectFile::IsProjectNeverSaved() const {
    return m_NeverSaved;
}

bool ProjectFile::IsThereAnyProjectChanges() const {
    return m_IsThereAnyChanges;
}

void ProjectFile::SetProjectChange(bool vChange) {
    m_IsThereAnyChanges = vChange;
    m_WasJustSaved = true;
    m_WasJustSavedFrameCounter = 2U;
}

void ProjectFile::NewFrame() {
    if (m_WasJustSavedFrameCounter) {
        --m_WasJustSavedFrameCounter;
    } else {
        m_WasJustSaved = false;
    }
}

bool ProjectFile::WasJustSaved() {
    return m_WasJustSaved;
}

std::string ProjectFile::GetAbsolutePath(const std::string& vFilePathName) const {
    std::string res = vFilePathName;
    if (!vFilePathName.empty()) {
        if (!FileHelper::Instance()->IsAbsolutePath(vFilePathName))  
        {// relative
            res = FileHelper::Instance()->SimplifyFilePath(m_ProjectFilePath + FileHelper::Instance()->puSlashType + vFilePathName);
        }
    }
    return res;
}

std::string ProjectFile::GetRelativePath(const std::string& vFilePathName) const {
    std::string res = vFilePathName;
    if (!vFilePathName.empty()) {
        res = FileHelper::Instance()->GetRelativePathToPath(vFilePathName, m_ProjectFilePath);
    }
    return res;
}

std::string ProjectFile::GetProjectFilepathName() const {
    return m_ProjectFilePathName;
}

std::string ProjectFile::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
    std::string str;
    str += vOffset + "<project>\n";
    const std::string& project_tag = "project";
    str += LayoutManager::Instance()->getXml(vOffset + "\t", project_tag);
    str += AccountPane::Instance()->getXml(vOffset + "\t", project_tag);
    str += SettingsDialog::Instance()->getXml(vOffset + "\t", project_tag);
    str += vOffset + "</project>\n";
    return str;
}

bool ProjectFile::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
    const std::string& strName = vElem->Value();  // the std::string is important, because Value() return a const char*
    std::string strValue;
    if (vElem->GetText()) {
        strValue = vElem->GetText();
    }
    std::string strParentName;
    if (vParent != nullptr) {
        strParentName = vParent->Value();
    }

    if (strName == "config") {
        return true;
    } else if (strName == "project") {
        const std::string& project_tag = "project";
        LayoutManager::Instance()->RecursParsingConfig(vElem, vParent, project_tag);
        AccountPane::Instance()->RecursParsingConfig(vElem, vParent, project_tag);
        SettingsDialog::Instance()->RecursParsingConfig(vElem, vParent, project_tag);
    }

    return true;
}
