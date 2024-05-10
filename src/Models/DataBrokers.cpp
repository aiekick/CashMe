#include <Models/DataBrokers.h>

#include <Plugins/PluginManager.h>

#include <ImGuiPack.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <csv/csv.h>

#include <Project/ProjectFile.h>

#include <chrono>
#include <vector>
#include <ctime>

#define OFFSET_IN_DAYS_FROM_NOW 360

bool DataBrokers::init() {
    m_GetAvailableDataBrokers();
    return true;
}

void DataBrokers::unit() {
    m_Clear();
}

void DataBrokers::load() {
    m_RefreshSymbols();
}

std::time_t convertToEpochTime(const std::string& vIsoDateTime, const char* format) {
    struct std::tm time = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::istringstream ss(vIsoDateTime);
    ss >> std::get_time(&time, format);
    if (ss.fail()) {
        std::cerr << "ERROR: Cannot parse date string (" << vIsoDateTime << "); required format %Y-%m-%d" << std::endl;
        exit(1);
    }
    time.tm_hour = 0;
    time.tm_min = 0;
    time.tm_sec = 0;
#ifdef _WIN32
    return _mkgmtime(&time);
#else
    return timegm(&time);
#endif
}

std::string convertToISO8601(const std::time_t& vEpochTime) {
    auto tp = std::chrono::system_clock::from_time_t(vEpochTime);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    auto* timeinfo = std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%Y-%m-%d");
    return oss.str();
}


bool DataBrokers::draw() {
    bool change = false;
    change |= m_DrawPluginBrockers();
    change |= m_DrawDBSymbols();
    return change;
}

void DataBrokers::drawDialogs() {
    /* if (ImGuiFileDialog::Instance()->Display("ImportPrices")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            auto files = ImGuiFileDialog::Instance()->GetSelection();
            ImportTypeEnum pType = (ImportTypeEnum)(uintptr_t)ImGuiFileDialog::Instance()->GetUserDatas();
            for (const auto& file : files) {
                m_ImportFromFiles(file.second, pType);
            }
            m_RefreshSymbols();
        }
        ImGuiFileDialog::Instance()->Close();
    }*/
}

bool DataBrokers::drawImportMenu() {
    return false;
}

bool DataBrokers::drawCreationMenu() {
    return false;
}

std::string DataBrokers::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);
    std::string res;
    res += vOffset + "<data_brokers>\n";
    // we save all modules not just the one is selected
    for (const auto& mod : m_DataBrokerModules) {
        auto ptr = dynamic_cast<Cash::IXmlSettings*>(mod.second.get());
        if (ptr != nullptr) {
            if (vUserDatas == "app") {
                res += ptr->GetXmlSettings(vOffset + "\t", Cash::ISettingsType::APP);
            } else if (vUserDatas == "project") {
                res += ptr->GetXmlSettings(vOffset + "\t", Cash::ISettingsType::PROJECT);
            } else {
                CTOOL_DEBUG_BREAK;  // ERROR
            }
        }
    }
    res += vOffset + "\t<selected_brocker>" + m_DataBrokerNames[m_BrokerComboIdx] + "</selected_brocker>\n";
    res += vOffset + "</data_brokers>\n";
    return res;
}

bool DataBrokers::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != 0)
        strParentName = vParent->Value();

    // we load all modules not just the one is selected
    for (const auto& mod : m_DataBrokerModules) {
        auto ptr = dynamic_cast<Cash::IXmlSettings*>(mod.second.get());
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

    if (strParentName == "data_brokers") {
        if (strName == "selected_brocker") {
            m_SelectBrocker(strValue);
        }
    }

    return true;
}

bool DataBrokers::m_DrawPluginBrockers() {
    bool change = false;
    /*ImGui::Header("Brokers :");
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::BeginContrastedCombo("##Data brokers", m_DataBrokerNames[m_BrokerComboIdx].c_str())) {
        for (int32_t idx = 0; idx < m_DataBrokerNames.size(); ++idx) {
            const bool is_selected = (m_BrokerComboIdx == idx);
            if (ImGui::Selectable(m_DataBrokerNames[idx].c_str(), is_selected)) {
                m_BrokerComboIdx = idx;
                change = true;
                m_SelectedBroker.reset();
                auto selectedName = m_DataBrokerNames.at(m_BrokerComboIdx);
                if (m_DataBrokerModules.find(selectedName) != m_DataBrokerModules.end()) {
                    m_SelectedBroker = m_DataBrokerModules.at(selectedName);
                }
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
        ImGui::Separator();
    }
    if (!m_SelectedBroker.expired()) {
        auto ptr = m_SelectedBroker.lock();
        if (ptr != nullptr) {
            auto drawer_ptr = std::dynamic_pointer_cast<Cash::IGuiDrawer>(ptr);
            if (drawer_ptr != nullptr) {
                change |= drawer_ptr->DrawWidgets(0, nullptr, nullptr);
            }
            float column_offest = 100.0f;
            ImGui::Separator();
            ImGui::Text("Symbol");
            ImGui::SameLine(column_offest);
            ImGui::PushItemWidth(100.0f);
            ImGui::InputText("##Symmbol", m_Symmbol.data(), m_Symmbol.size());
            ImGui::PopItemWidth();
            ImGui::Text("Start Date");
            ImGui::SameLine(column_offest);
            ImGui::PushItemWidth(100.0f);
            ImGui::InputText("##StartDate", m_StartDate.data(), m_StartDate.size());
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::ContrastedButton("E##startDate")) {
                m_StartDatePickerLevel = 0;  // day level
                m_StartDatePicker = ImPlotTime(convertToEpochTime(m_StartDate.data(), "%Y-%m-%d"));
                m_ShowStartDatePicker = !m_ShowStartDatePicker;
            }
            if (m_ShowStartDatePicker) {
                if (ImPlot::ShowDatePicker("##startDatePicekr", &m_StartDatePickerLevel, &m_StartDatePicker)) {
                    std::tm* tm_date = std::localtime(&m_StartDatePicker.S);
                    strftime(m_StartDate.data(), m_StartDate.size(), "%Y-%m-%d", tm_date);
                    m_ShowStartDatePicker = false;
                }
            }
            ImGui::Text("End Date");
            ImGui::SameLine(column_offest);
            ImGui::PushItemWidth(100.0f);
            ImGui::InputText("##EndDate", m_EndDate.data(), m_EndDate.size());
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::ContrastedButton("E##endDate")) {
                m_EndDatePickerLevel = 0;  // day level
                m_EndDatePicker = ImPlotTime(convertToEpochTime(m_EndDate.data(), "%Y-%m-%d"));
                m_ShowEndDatePicker = !m_ShowEndDatePicker;
            }
            if (m_ShowEndDatePicker) {
                if (ImPlot::ShowDatePicker("##endDatePicekr", &m_EndDatePickerLevel, &m_EndDatePicker)) {
                    std::tm* tm_date = std::localtime(&m_EndDatePicker.S);
                    strftime(m_EndDate.data(), m_EndDate.size(), "%Y-%m-%d", tm_date);
                    m_ShowEndDatePicker = false;
                }
            }
            ImGui::Separator();
            static bool downloadDatas = false;
            ImGui::SameLine();
            ImGui::Text(downloadDatas ? "Download from Internet" : "Read from File");
            ImGui::SameLine();
            ImGui::Checkbox("##downloadDatas", &downloadDatas);
            ImGui::Separator();
            if (ImGui::ContrastedButton("Request")) {
                if (ptr->RequestSymbolPrices(m_Symmbol.data(), m_StartDate.data(), m_EndDate.data(), downloadDatas ? "net" : "file")) {
                    auto symbolPrices = ptr->getLastRequestedSymbolPrices();
                    m_SavePrices(symbolPrices.prices);
                    m_VizualizePrices(symbolPrices.prices);
                }
            }
        }
    }*/
    return change;
}

bool DataBrokers::m_DrawDBSymbols() {
    bool change = false;
    /*ImGui::Header("Symbols :");
    if (ImGui::ContrastedButton("Refresh", nullptr, nullptr, -1.0f)) {
        m_RefreshSymbols();
    }
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::BeginTable("##Symbols", 6)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Up", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Market", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Symbol", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("TimeFrame", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Load", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        bool selected = false;
        for (const auto& market : m_DBSymbols) {
            for (const auto& symbol : market.second) {
                for (const auto& timeframe : symbol.second) {
                    ImGui::TableNextColumn();  // update, fill fill input with symbol name
                    if (ImGui::SmallContrastedButton("U")) {
                        strncpy(m_Symmbol.data(), symbol.first.c_str(), 32);
                    }
                    ImGui::TableNextColumn();  // market name
                    ImGui::Selectable(market.first.c_str(), &selected, ImGuiSelectableFlags_SpanAvailWidth);
                    ImGui::TableNextColumn();  // symbol name
                    ImGui::Text("%s", symbol.first.c_str());
                    ImGui::TableNextColumn();  // timeframe
                    ImGui::Text("%u", timeframe.first);
                    ImGui::TableNextColumn();  // bars count
                    ImGui::Text("%u", timeframe.second);
                    ImGui::TableNextColumn();  // load graph
                    if (ImGui::SmallContrastedButton(">")) {
                        m_VizualizePrices(m_ExtractPricesFromDB(market.first, symbol.first, timeframe.first));
                    }
                }
            }
        }
        ImGui::EndTable();
    }*/
    return change;
}

void DataBrokers::m_Clear() {
    m_DataBrokerNames.clear();
    // msut be cleared even if they are some smart pointers
    // since the pointers come from a plugin loaded memory
    // when the plugin is destoryed, an automatic reset of weak pointer will cause a crash
    // the same for all pointers type who are coming from the plugin.
    // so we must do this manually here
    // pointer on a internal plugin data
    // thoses contain weak ptrs
    m_SelectedBroker.reset();
    // this dico save plugin ptr, we clear it at end
    m_DataBrokerModules.clear();
}

void DataBrokers::m_GetAvailableDataBrokers() {
    m_Clear();
    m_DataBrokerNames.push_back("None");
    auto modules = PluginManager::Instance()->GetPluginModulesInfos();
    for (const auto& mod : modules) {
        if (mod.type == Cash::PluginModuleType::DATA_BROKER) {
            auto ptr = std::dynamic_pointer_cast<Cash::BankStatementImportModule>(PluginManager::Instance()->CreatePluginModule(mod.label));
            if (ptr != nullptr) {
                m_DataBrokerModules[mod.label] = ptr;
                m_DataBrokerNames.push_back(mod.label);
            }
        }
    }
}

/*void DataBrokers::m_SavePrices(const Cash::SymbolPrices& vPrices) {
    if (vPrices.market.empty() || vPrices.symbol.empty() || vPrices.prices.period == 0 || vPrices.prices.times.empty()) {
        LogVarError("Cant save Prices since empty");
        return;
    }
    if (DataBase::Instance()->BeginTransaction()) {
        DataBase::Instance()->AddMarket(vPrices.market);
        DataBase::Instance()->AddSymbol(vPrices.symbol);
        DataBase::Instance()->AddTimeFrame(vPrices.prices.period);
        for (size_t idx = 0; idx < vPrices.prices.times.size(); ++idx) {
            DataBase::Instance()->AddPrice(vPrices.market,
                                           vPrices.symbol,
                                           vPrices.prices.period,
                                           vPrices.prices.times[idx],
                                           vPrices.prices.opens[idx],
                                           vPrices.prices.highs[idx],
                                           vPrices.prices.lows[idx],
                                           vPrices.prices.closes[idx],
                                           vPrices.prices.volumes[idx]);
        }
        DataBase::Instance()->CommitTransaction();
        ProjectFile::Instance()->SetProjectChange();
    }
}*/

void DataBrokers::m_SelectBrocker(const DataBrokerName& vName) {
    /*if (m_DataBrokerModules.find(vName) != m_DataBrokerModules.end()) {
        m_SelectedBroker = m_DataBrokerModules.at(vName);
        m_BrokerComboIdx = 0;
        for (const auto& name : m_DataBrokerNames) {
            if (name != vName) {
                ++m_BrokerComboIdx;
            }
        }
    }*/
}

void DataBrokers::m_RefreshSymbols() {
    /* m_DBSymbols.clear();
    DataBase::Instance()->GetSymbols([this](const Market& vMarket, const Symbol& vSymbol, const TimeFrame& vTimeFrame, const BarsCount& vBarsCount) {
        if (!vMarket.empty() && !vSymbol.empty()) {
            m_DBSymbols[vMarket][vSymbol][vTimeFrame] = vBarsCount;
        }
    });*/
}

/*Cash::SymbolPrices DataBrokers::m_ExtractPricesFromDB(const Market& vMarket, const Symbol& vSymbol, const TimeFrame& vTimeFrame) {
    Cash::SymbolPrices res;
    res.market = vMarket;
    res.symbol = vSymbol;
    res.prices.period = vTimeFrame;
    // todo : faudrait optmiser en recuperant le nombre de ligne pour accelere les push_back
    DataBase::Instance()->GetPrices(
        vMarket,
        vSymbol,
        vTimeFrame,
        [&res](const Date& vDate, const OpenPrice& vOpen, const HighPrice& vHigh, const LowPrice& vLow, const ClosePrice& vClose, const VolumePrice& vVolume) {
            res.prices.times.push_back(vDate);
            res.prices.opens.push_back(vOpen);
            res.prices.highs.push_back(vHigh);
            res.prices.lows.push_back(vLow);
            res.prices.closes.push_back(vClose);
            res.prices.volumes.push_back(vVolume);
        });    
    return res;
}*/

void DataBrokers::m_DrawAccountCreation() {

}
