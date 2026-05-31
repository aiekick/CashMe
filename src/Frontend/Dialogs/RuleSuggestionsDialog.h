#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <imguipack.h>
#include <Headers/DatasDef.h>

// modal review of categorization suggestions :
// the user edits the suggested category/operation per row, checks the rows to apply, OK applies them
class RuleSuggestionsDialog {
private:
    bool m_show = false;
    std::vector<CategorizationSuggestion> m_suggestions;
    std::vector<std::string> m_categoryDisplay;   // "(none)" + categories, prebuilt in open()
    std::vector<std::string> m_operationDisplay;  // "(none)" + operations, prebuilt in open()
    std::vector<int> m_categoryIndices;           // current category combo index per suggestion, prebuilt in open()
    std::vector<int> m_operationIndices;          // current operation combo index per suggestion, prebuilt in open()

public:
    void open(  //
        const std::vector<CategorizationSuggestion>& vSuggestions,
        const std::vector<std::string>& vCategoryNames,
        const std::vector<std::string>& vOperationNames);
    // returns true the frame the suggestions were applied (so the caller can refresh)
    bool draw(const ImVec2& vCenter);

private:
    int32_t m_displayIndex(const std::vector<std::string>& vDisplay, const std::string& vValue) const;
};
