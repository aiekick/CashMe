/*
Copyright 2021-2023 Stephane Cuillerdier (aka aiekick)

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

#include "TranslationHelper.h"

#include <ctools/cTools.h>
#include <ImGuiPack/ImGuiPack.h>


///////////////////////////////////////////////////////
//// STATIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

LanguageEnum TranslationHelper::s_HelpLanguage = LanguageEnum::FR;

const char* TranslationHelper::layout_menu_name = nullptr;
const char* TranslationHelper::layout_menu_help = nullptr;

const char* TranslationHelper::mainframe_menubar_project = nullptr;
const char* TranslationHelper::mainframe_menubar_project_open = nullptr;
const char* TranslationHelper::mainframe_menubar_project_reload = nullptr;
const char* TranslationHelper::mainframe_menubar_project_close = nullptr;
const char* TranslationHelper::mainframe_menubar_settings = nullptr;

///////////////////////////////////////////////////////
//// CTOR /////////////////////////////////////////////
///////////////////////////////////////////////////////

TranslationHelper::TranslationHelper() {
    DefineLanguage(LanguageEnum::EN, true); // Default
}

///////////////////////////////////////////////////////
//// CHANGE LANGUAGE //////////////////////////////////
///////////////////////////////////////////////////////

void TranslationHelper::DefineLanguage(LanguageEnum vLanguage, bool vForce) {
    if (vLanguage != TranslationHelper::s_HelpLanguage || vForce) {
        TranslationHelper::s_HelpLanguage = vLanguage;

        switch (vLanguage) {
            case LanguageEnum::EN: DefineLanguageEN(); break;
            case LanguageEnum::FR: DefineLanguageFR(); break;
            default: break;
        }
    }
}

///////////////////////////////////////////////////////
//// CHANGE IMGUI MENU ////////////////////////////////
///////////////////////////////////////////////////////

float TranslationHelper::DrawMenu() {
    float last_cur_pos = ImGui::GetCursorPosX();

    if (ImGui::MenuItem("EN", nullptr, TranslationHelper::s_HelpLanguage == LanguageEnum::EN)) {
        DefineLanguage(LanguageEnum::EN);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Change the translation to the English\nBut you need to restart the app\nAnd dont forgot reset the layout to the default after the restart\nIf you have a "
            "shity layout");
    }

    if (ImGui::MenuItem("FR", nullptr, TranslationHelper::s_HelpLanguage == LanguageEnum::FR)) {
        DefineLanguage(LanguageEnum::FR);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Change la traduction pour le Francais\nMais vous devez redemarrer l'application\n et ne pas oublier de reinitialier la disposition par defaut apres le "
            "redemarrage\nSi vous avez une disposition a la con");
    }

    return ImGui::GetCursorPosX() - last_cur_pos + ImGui::GetStyle().FramePadding.x;
}

///////////////////////////////////////////////////////
//// CHANGE LANGUAGE : PRIVATE ////////////////////////
///////////////////////////////////////////////////////

void TranslationHelper::DefineLanguageEN() {
    TranslationHelper::layout_menu_name = " Layouts";
    TranslationHelper::layout_menu_help = "Default Layout";

    TranslationHelper::mainframe_menubar_project = "Project";
    TranslationHelper::mainframe_menubar_project_open = " Open";
    TranslationHelper::mainframe_menubar_project_reload = " Reload";
    TranslationHelper::mainframe_menubar_project_close = " Close";
    TranslationHelper::mainframe_menubar_settings = " Settings";
}

void TranslationHelper::DefineLanguageFR() {
    TranslationHelper::layout_menu_name = " Dispositions";
    TranslationHelper::layout_menu_help = "Disposition par defaut";

    TranslationHelper::mainframe_menubar_project = "Project";
    TranslationHelper::mainframe_menubar_project_open = " Ouvrir";
    TranslationHelper::mainframe_menubar_project_reload = " Recharger";
    TranslationHelper::mainframe_menubar_project_close = " Fermer";
    TranslationHelper::mainframe_menubar_settings = " Reglages";
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string TranslationHelper::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    // the rest
    str += vOffset + "<help_lang>" + ct::toStr((int)TranslationHelper::s_HelpLanguage) + "</help_lang>\n";

    return str;
}

bool TranslationHelper::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strName == "help_lang")
        DefineLanguage((LanguageEnum)ct::ivariant(strValue).GetI());

    return true;  // continue for explore childs. need to return false if we want explore child ourselves
}