#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include <ezlibs/ezApp.hpp>
#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezXmlConfig.hpp>
#include <ezlibs/ezPlugin.hpp>
#include <ezlibs/ezSingleton.hpp>

#include <imguipack.h>

#include <apis/CashMePluginApi.h>

class PluginInstance;
typedef std::weak_ptr<PluginInstance> PluginInstanceWeak;
typedef std::shared_ptr<PluginInstance> PluginInstancePtr;

enum class PluginReturnMsg { LOADING_SUCCEED = 1, LOADING_FAILED = 0, NOT_A_PLUGIN = -1 };

struct PluginInterface;
typedef std::shared_ptr<Cash::PluginInterface> PluginInterfacePtr;
typedef std::weak_ptr<Cash::PluginInterface> PluginInterfaceWeak;

class PluginInstance {
private:
    ez::plugin::Loader<Cash::PluginInterface> m_Loader;
    PluginInterfacePtr m_PluginInstance = nullptr;
    std::string m_Name;

public:
    PluginInstance();
    virtual ~PluginInstance();

    PluginReturnMsg Init(const std::string& vName, const std::string& vFilePathName);
    void Unit();

    PluginInterfaceWeak Get() const;
};

class PluginManager : public Cash::PluginBridge {
    IMPLEMENT_SINGLETON(PluginManager)
private:
    std::map<std::string, PluginInstancePtr> m_Plugins;

public:
    void LoadPlugins(const ez::App& vApp);
    std::vector<Cash::PluginModuleInfos> GetPluginModulesInfos() const;
    Cash::PluginModulePtr CreatePluginModule(const std::string& vPluginNodeName);
    std::vector<Cash::PluginPaneConfig> GetPluginPanes() const;
    std::vector<Cash::PluginSettingsConfig> GetPluginSettings() const;
    void Clear();

private:
    void m_LoadPlugin(const std::filesystem::directory_entry& vEntry);
    void m_DisplayLoadedPlugins();

public:
    PluginManager() = default;                      // Prevent construction
    PluginManager(const PluginManager&) = delete;  // Prevent construction by copying
    PluginManager& operator=(const PluginManager&) {
        return *this;
    };                           // Prevent assignment
    virtual ~PluginManager() = default;  // Prevent unwanted destruction
};