// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "LCLBroker.h"
#include <Headers/LCLBrokerBuild.h>

#include <Modules/OfcAccountStatementModule.h>
#include <Modules/PdfAccountStatementModule.h>

// needed for plugin creating / destroying
extern "C"  // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
    #define PLUGIN_PREFIX __declspec(dllexport)
#else
    #define PLUGIN_PREFIX
#endif

PLUGIN_PREFIX LCLBroker* allocator() {
    return new LCLBroker();
}

PLUGIN_PREFIX void deleter(LCLBroker* ptr) {
    delete ptr;
}
}

LCLBroker::LCLBroker() {
#ifdef _MSC_VER
    // active memory leak detector
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

bool LCLBroker::Init() {
    return true;
}

void LCLBroker::Unit() {
}

uint32_t LCLBroker::GetMinimalCashMeVersionSupported() const {
    return 0U;
}

uint32_t LCLBroker::GetVersionMajor() const {
    return LCLBroker_MinorNumber;
}

uint32_t LCLBroker::GetVersionMinor() const {
    return LCLBroker_MajorNumber;
}

uint32_t LCLBroker::GetVersionBuild() const {
    return LCLBroker_BuildNumber;
}

std::string LCLBroker::GetName() const {
    return "LCLBroker";
}

std::string LCLBroker::GetAuthor() const {
    return "Stephane Cuillerdier";
}

std::string LCLBroker::GetVersion() const {
    return LCLBroker_BuildId;
}

std::string LCLBroker::GetContact() const {
    return "cashme@funparadigm.com";
}

std::string LCLBroker::GetDescription() const {
    return "LCL Bank Statement Importer";
}

std::vector<Cash::PluginModuleInfos> LCLBroker::GetModulesInfos() const {
    std::vector<Cash::PluginModuleInfos> res;
    res.push_back(Cash::PluginModuleInfos("LCL", "OFC", Cash::PluginModuleType::DATA_BROKER));
    res.push_back(Cash::PluginModuleInfos("LCL", "PDF", Cash::PluginModuleType::DATA_BROKER));
    return res;
}

Cash::PluginModulePtr LCLBroker::CreateModule(const std::string& vPluginModuleName, Cash::PluginBridge* /*vBridgePtr*/) {
    if (vPluginModuleName == "OFC") {
        return OfcAccountStatementModule::create();
    } else if (vPluginModuleName == "PDF") {
        return PdfAccountStatementModule::create();
    }
    return nullptr;
}

std::vector<Cash::PluginPaneConfig> LCLBroker::GetPanes() const {
    return {};
}

std::vector<Cash::PluginSettingsConfig> LCLBroker::GetSettings() const {
    return {};
}
