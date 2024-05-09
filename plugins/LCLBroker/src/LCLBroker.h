#pragma once

#include <apis/CashMePluginApi.h>

/*
kind of charting  type to take into account :
* Curve               => OK
* Bars                => OK
* Candlesticks        => OK
* Renko               => OK
* Heiking Ashi        => OK
* Hollow Candlesticks => TO DO
* Columns             => TO DO
* Area                => TO DO
* Baseline            => TO DO
* High-Low            => TO DO
* Line Break          => TO DO
* Kagi                => TO DO
* Point & Figure      => TO DO
* Range               => TO DO
*/

class LCLBroker : public Cash::PluginInterface {
public:
	LCLBroker();
    virtual ~LCLBroker() = default;
	bool Init() override;
    void Unit() override;
    uint32_t GetMinimalCashMeVersionSupported() const override;
	uint32_t GetVersionMajor() const override;
	uint32_t GetVersionMinor() const override;
	uint32_t GetVersionBuild() const override;
    std::string GetName() const override;
    std::string GetAuthor() const override;
    std::string GetVersion() const override;
    std::string GetContact() const override;
	std::string GetDescription() const override;
    std::vector<Cash::PluginModuleInfos> GetModulesInfos() const override;
    Cash::PluginModulePtr CreateModule(const std::string& vPluginModuleName, Cash::PluginBridge* vBridgePtr) override;
    std::vector<Cash::PluginPaneConfig> GetPanes() const override;
    std::vector<Cash::PluginSettingsConfig> GetSettings() const override;
};