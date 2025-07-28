#include "PluginManager.h"

#include <imguipack.h>

#include <ezlibs/ezFile.hpp>

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////////
////// PluginInstance ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PluginInstance::PluginInstance() {
}

PluginInstance::~PluginInstance() {
    Unit();
}

PluginReturnMsg PluginInstance::Init(const std::string& vName, const std::string& vFilePathName) {
    m_Name = vName;
    m_Loader = ez::plugin::Loader<Cash::PluginInterface>(vFilePathName);
    m_Loader.dlOpenLib();
    m_PluginInstance = m_Loader.dlGetInstance();
    if (m_Loader.isAPlugin()) {
        if (m_Loader.isValid()) {
            if (m_PluginInstance) {
                if (!m_PluginInstance->Init()) {
                    m_PluginInstance.reset();
                } else {
                    return PluginReturnMsg::LOADING_SUCCEED;
                }
            }
        }
        return PluginReturnMsg::LOADING_FAILED;
    }
    return PluginReturnMsg::NOT_A_PLUGIN;
}

void PluginInstance::Unit() {
    m_PluginInstance->Unit();
    m_PluginInstance.reset();
    m_Loader.dlCloseLib();
}

PluginInterfaceWeak PluginInstance::Get() const {
    return m_PluginInstance;
}

//////////////////////////////////////////////////////////////////////////////
////// PluginLoader //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static inline std::string GetDLLExtention() {
#ifdef WIN32
    return "dll";
#elif defined(__linux__)
    return "so";
#elif defined(__APPLE__)
    return "dylib";
#elif
    return "";
#endif
}

void PluginManager::Clear() {
    m_Plugins.clear();
}

void PluginManager::LoadPlugins(const ez::App& vApp) {
    printf("-----------\n");
    LogVarLightInfo("Availables Plugins :\n");
    auto plugin_directory = std::filesystem::path(vApp.getAppPath()).append("plugins");
    if (std::filesystem::exists(plugin_directory)) {
        const auto dir_iter = std::filesystem::directory_iterator(plugin_directory);
        for (const auto& file : dir_iter) {
            m_LoadPlugin(file);
        }
        m_DisplayLoadedPlugins();
    } else {
        LogVarLightInfo("Plugin directory %s not found !", plugin_directory.string().c_str());
    }
    printf("-----------\n");
}

std::vector<Cash::PluginModuleInfos> PluginManager::GetPluginModulesInfos() const {
    std::vector<Cash::PluginModuleInfos> res;

    for (auto plugin : m_Plugins) {
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr != nullptr) {
                auto lib_entrys = pluginInstancePtr->GetModulesInfos();
                if (!lib_entrys.empty()) {
                    res.insert(res.end(), lib_entrys.begin(), lib_entrys.end());
                }
            }
        }
    }

    return res;
}

Cash::PluginModulePtr PluginManager::CreatePluginModule(const std::string& vPluginNodeName) {
    if (!vPluginNodeName.empty()) {
        for (auto plugin : m_Plugins) {
            if (plugin.second) {
                auto pluginInstancePtr = plugin.second->Get().lock();
                if (pluginInstancePtr != nullptr) {
                    auto ptr = pluginInstancePtr->CreateModule(vPluginNodeName, this);
                    if (ptr != nullptr) {
                        return ptr;
                    }
                }
            }
        }
    }
    return nullptr;
}

std::vector<Cash::PluginPaneConfig> PluginManager::GetPluginPanes() const {
    std::vector<Cash::PluginPaneConfig> pluginsPanes;
    for (auto plugin : m_Plugins) {
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr) {
                auto _pluginPanes = pluginInstancePtr->GetPanes();
                if (!_pluginPanes.empty()) {
                    pluginsPanes.insert(pluginsPanes.end(), _pluginPanes.begin(), _pluginPanes.end());
                }
            }
        }
    }
    return pluginsPanes;
}

std::vector<Cash::PluginSettingsConfig> PluginManager::GetPluginSettings() const {
    std::vector<Cash::PluginSettingsConfig> pluginSettings;
    for (auto plugin : m_Plugins) {
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr) {
                auto _pluginSettings = pluginInstancePtr->GetSettings();
                if (!_pluginSettings.empty()) {
                    pluginSettings.insert(pluginSettings.end(), _pluginSettings.begin(), _pluginSettings.end());
                }
            }
        }
    }
    return pluginSettings;
}

//////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void PluginManager::m_LoadPlugin(const fs::directory_entry& vEntry) {
    if (vEntry.is_directory()) {
        const auto dir_iter = std::filesystem::directory_iterator(vEntry);
        for (const auto& file : dir_iter) {
            m_LoadPlugin(file);
        }
    } else if (vEntry.is_regular_file()) {
        auto file_name = vEntry.path().filename().string();
        if (file_name.find(PLUGIN_NAME_PREFIX) == 0U) {
            if (file_name.find(PLUGIN_RUNTIME_CONFIG) != std::string::npos) {
                auto file_path_name = vEntry.path().string();
                if (file_path_name.find(GetDLLExtention()) != std::string::npos) {
                    auto ps = ez::file::parsePathFileName(file_path_name);
                    if (ps.isOk) {
                        auto resPtr = std::make_shared<PluginInstance>();
                        auto ret = resPtr->Init(ps.name, ps.GetFPNE());
                        if (ret != PluginReturnMsg::LOADING_SUCCEED) {
                            resPtr.reset();
                            if (ret == PluginReturnMsg::LOADING_FAILED) {
                                LogVarDebugError("Plugin %s fail to load", ps.name.c_str());
                            }
                        } else {
                            auto pluginInstancePtr = resPtr->Get().lock();
                            if (pluginInstancePtr) {
                                char spaceBuffer[40 + 1] = "";
                                spaceBuffer[0] = '\0';

                                std::string name = pluginInstancePtr->GetName();
                                if (name.size() < 15U) {
                                    size_t of = 15U - name.size();
                                    memset(spaceBuffer, 32, of);  // 32 is space code in ASCII table
                                    spaceBuffer[of] = '\0';
                                    name += spaceBuffer;
                                } else {
                                    name = name.substr(0, 15U);
                                }

                                std::string version = pluginInstancePtr->GetVersion();
                                if (version.size() < 10U) {
                                    size_t of = 10U - version.size();
                                    memset(spaceBuffer, 32, of);  // 32 is space code in ASCII table
                                    spaceBuffer[of] = '\0';
                                    version += spaceBuffer;
                                } else {
                                    version = version.substr(0, 10U);
                                }

                                std::string desc = pluginInstancePtr->GetDescription();
                            }

                            m_Plugins[ps.name] = resPtr;
                        }
                    }
                }
            }
        }
    }
}

void PluginManager::m_DisplayLoadedPlugins() {
    if (!m_Plugins.empty()) {
        size_t max_name_size = 0U;
        size_t max_vers_size = 0U;
        const size_t& minimal_space = 2U;
        for (auto plugin : m_Plugins) {
            if (plugin.second != nullptr) {
                auto plugin_instance_ptr = plugin.second->Get().lock();
                if (plugin_instance_ptr != nullptr) {
                    max_name_size = ez::maxi(max_name_size, plugin_instance_ptr->GetName().size() + minimal_space);
                    max_vers_size = ez::maxi(max_vers_size, plugin_instance_ptr->GetVersion().size() + minimal_space);
                }
            }
        }
        for (auto plugin : m_Plugins) {
            if (plugin.second != nullptr) {
                auto plugin_instance_ptr = plugin.second->Get().lock();
                if (plugin_instance_ptr != nullptr) {
                    const auto& name = plugin_instance_ptr->GetName();
                    const auto& name_space = std::string(max_name_size - name.size(), ' ');  // 32 is a space in ASCII
                    const auto& vers = plugin_instance_ptr->GetVersion();
                    const auto& vers_space = std::string(max_vers_size - vers.size(), ' ');  // 32 is a space in ASCII
                    const auto& desc = plugin_instance_ptr->GetDescription();
                    LogVarLightInfo("Plugin loaded : %s%sv%s%s(%s)",  //
                        name.c_str(), name_space.c_str(),             //
                        vers.c_str(), vers_space.c_str(),             //
                        desc.c_str());
                }
            }
        }
    }
}