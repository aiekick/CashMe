#include <Frontend/Dialogs/RuleSuggestionsDialog.h>
#include <Models/DataBase.h>

void RuleSuggestionsDialog::open(  //
    const std::vector<CategorizationSuggestion>& vSuggestions,
    const std::vector<std::string>& vCategoryNames,
    const std::vector<std::string>& vOperationNames) {
    m_suggestions = vSuggestions;
    // prebuild the display lists once : a "(none)" first entry lets the user clear a value
    m_categoryDisplay.clear();
    m_categoryDisplay.reserve(vCategoryNames.size() + 1U);
    m_categoryDisplay.push_back("(none)");
    m_categoryDisplay.insert(m_categoryDisplay.end(), vCategoryNames.begin(), vCategoryNames.end());
    m_operationDisplay.clear();
    m_operationDisplay.reserve(vOperationNames.size() + 1U);
    m_operationDisplay.push_back("(none)");
    m_operationDisplay.insert(m_operationDisplay.end(), vOperationNames.begin(), vOperationNames.end());
    // prebuild the current combo index of each suggestion
    m_categoryIndices.resize(m_suggestions.size());
    m_operationIndices.resize(m_suggestions.size());
    for (size_t row = 0; row < m_suggestions.size(); ++row) {
        m_categoryIndices[row] = m_displayIndex(m_categoryDisplay, m_suggestions[row].suggestedCategory);
        m_operationIndices[row] = m_displayIndex(m_operationDisplay, m_suggestions[row].suggestedOperation);
    }
    m_show = true;
}

int32_t RuleSuggestionsDialog::m_displayIndex(const std::vector<std::string>& vDisplay, const std::string& vValue) const {
    if (vValue.empty()) {
        return 0;  // index 0 is the "(none)" entry
    }
    for (size_t idx = 1; idx < vDisplay.size(); ++idx) {
        if (vDisplay[idx] == vValue) {
            return static_cast<int32_t>(idx);
        }
    }
    return 0;
}

bool RuleSuggestionsDialog::draw(const ImVec2& vCenter) {
    if (!m_show) {
        return false;
    }
    bool applied = false;
    const char* title = "Categorization suggestions";
    ImGui::OpenPopup(title);
    ImGui::SetNextWindowPos(vCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(900.0f, 500.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(title, &m_show, ImGuiWindowFlags_NoDocking)) {
        if (!m_suggestions.empty()) {
            if (ImGui::ContrastedButton("Check all")) {
                for (auto& suggestion : m_suggestions) {
                    suggestion.apply = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::ContrastedButton("Check none")) {
                for (auto& suggestion : m_suggestions) {
                    suggestion.apply = false;
                }
            }
        }

        const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
        const ImVec2 tableSize(0.0f, -ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y);
        if (ImGui::BeginTable("##suggestions", 6, flags, tableSize)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Apply", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            for (size_t row = 0; row < m_suggestions.size(); ++row) {
                auto& suggestion = m_suggestions[row];
                ImGui::TableNextRow();
                ImGui::PushID(static_cast<int>(row));
                ImGui::TableNextColumn();
                ImGui::CheckBoxBoolDefault("##apply", &suggestion.apply, true);
                ImGui::TableNextColumn();
                ImGui::Text("%s", suggestion.date.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", suggestion.description.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", suggestion.amount);
                ImGui::TableNextColumn();
                if (ImGui::ContrastedComboVectorDefault(150.0f, "##cat", &m_categoryIndices[row], m_categoryDisplay, 0)) {
                    suggestion.suggestedCategory = (m_categoryIndices[row] <= 0) ? std::string() : m_categoryDisplay[m_categoryIndices[row]];
                }
                ImGui::TableNextColumn();
                if (ImGui::ContrastedComboVectorDefault(150.0f, "##op", &m_operationIndices[row], m_operationDisplay, 0)) {
                    suggestion.suggestedOperation = (m_operationIndices[row] <= 0) ? std::string() : m_operationDisplay[m_operationIndices[row]];
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (ImGui::ContrastedButton("OK")) {
            if (DataBase::ref().BeginDBTransaction()) {
                for (const auto& suggestion : m_suggestions) {
                    if (suggestion.apply && (!suggestion.suggestedCategory.empty() || !suggestion.suggestedOperation.empty())) {
                        DataBase::ref().SetTransactionCategoryOperation(suggestion.transactionId, suggestion.suggestedCategory, suggestion.suggestedOperation);
                    }
                }
                DataBase::ref().CommitDBTransaction();
            }
            applied = true;
            m_show = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::ContrastedButton("Cancel")) {
            m_show = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return applied;
}
