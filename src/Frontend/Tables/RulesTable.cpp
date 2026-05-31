#include <Frontend/Tables/RulesTable.h>
#include <Frontend/MainFrontend.h>
#include <Systems/CategorizationEngine.h>
#include <Models/DataBase.h>
#include <ezlibs/ezStr.hpp>

RulesTable::RulesTable() : ADataTable("RulesTable", 9) {}

void RulesTable::clear() {
    m_rules.clear();
}

void RulesTable::refreshDatas() {
    m_updateAccounts();  // account list used by the Preview target
    m_rules.clear();
    DataBase::ref().GetRules([this](const CategorizationRule& vRule) {  //
        m_rules.push_back(vRule);
    });
}

void RulesTable::addRule(const CategorizationRule& vRule) {
    if (DataBase::ref().OpenDBFile()) {
        DataBase::ref().AddRule(vRule);
        DataBase::ref().CloseDBFile();
    }
    m_needRefresh = true;  // reload happens after the current draw
}

void RulesTable::updateRule(const CategorizationRule& vRule) {
    if (DataBase::ref().OpenDBFile()) {
        DataBase::ref().UpdateRule(vRule.id, vRule);
        DataBase::ref().CloseDBFile();
    }
    m_needRefresh = true;
}

void RulesTable::removeRule(const RowID& vId) {
    if (DataBase::ref().OpenDBFile()) {
        DataBase::ref().DeleteRule(vId);
        DataBase::ref().CloseDBFile();
    }
    m_needRefresh = true;
}

const std::vector<CategorizationRule>& RulesTable::getRules() const {
    return m_rules;
}

std::string RulesTable::m_amountRangeText(const CategorizationRule& vRule) const {
    if (!vRule.useAmountRange) {
        return "";
    }
    return ez::str::toStr("[%.2f ; %.2f]", vRule.amountMin, vRule.amountMax);
}

size_t RulesTable::m_getItemsCount() const {
    return m_rules.size();
}

RowID RulesTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_rules.size()) {
        return m_rules.at(vIdx).id;
    }
    return 0;
}

void RulesTable::m_drawTableContent(const size_t& vIdx, const double& /*vMaxAmount*/) {
    auto& rule = m_rules.at(vIdx);
    if (m_drawColumnCheckbox(rule.enabled)) {
        // persist the enable / disable (the cache bool is already toggled in place)
        if (DataBase::ref().OpenDBFile()) {
            DataBase::ref().UpdateRule(rule.id, rule);
            DataBase::ref().CloseDBFile();
        }
    }
    m_drawColumnSelectable(vIdx, rule.id, rule.name);
    m_drawColumnText(rule.description);
    m_drawColumnText(rule.descriptionPattern);
    m_drawColumnText(rule.commentPattern);
    m_drawColumnText(rule.entityPattern);
    m_drawColumnText(m_amountRangeText(rule));
    m_drawColumnText(rule.targetCategory);
    m_drawColumnText(rule.targetOperation);
}

void RulesTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("On", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Rule Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Rule Description", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Desc pattern", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Comment pattern", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Entity pattern", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Resulting Category", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Resulting Operation", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
}

void RulesTable::m_draw(const ImVec2& vSize) {
    ADataTable::m_draw(vSize);
    if (m_needRefresh) {  // deferred reload, safe now that the table draw is done
        m_needRefresh = false;
        refreshDatas();
    }
}

void RulesTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update")) {
            for (const auto& rule : m_rules) {
                if (m_isRowSelected(rule.id)) {
                    MainFrontend::ref().getRuleDialogRef().setRule(rule);
                    MainFrontend::ref().getRuleDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
                    break;
                }
            }
        }
        if (ImGui::MenuItem("Delete")) {
            std::vector<RowID> idsToDelete;
            for (const auto& rule : m_rules) {
                if (m_isRowSelected(rule.id)) {
                    idsToDelete.push_back(rule.id);
                }
            }
            for (const auto& id : idsToDelete) {
                removeRule(id);
            }
            m_ResetSelection();
        }
    }
}

void RulesTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& /*vRowID*/) {
    if (vIdx < m_rules.size()) {
        MainFrontend::ref().getRuleDialogRef().setRule(m_rules.at(vIdx));
        MainFrontend::ref().getRuleDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
    }
}

bool RulesTable::m_drawMenu() {
    if (ImGui::MenuItem("Add Rule")) {
        CategorizationRule rule;
        MainFrontend::ref().getRuleDialogRef().setRule(rule);
        MainFrontend::ref().getRuleDialogRef().show(DataDialogMode::MODE_CREATION);
    }
    if (ImGui::MenuItem("Apply all rules")) {
        m_runPreview();
    }
    return false;
}

void RulesTable::m_runPreview() {
    std::vector<std::string> categoryNames;
    std::vector<std::string> operationNames;
    DataBase::ref().GetCategories([&categoryNames](const CategoryOutput& vCategory) {  //
        categoryNames.push_back(vCategory.datas.name);
    });
    DataBase::ref().GetOperations([&operationNames](const OperationOutput& vOperation) {  //
        operationNames.push_back(vOperation.datas.name);
    });
    std::vector<CategorizationSuggestion> suggestions;
    DataBase::ref().GetTransactions(m_getAccountID(), [this, &suggestions](const TransactionOutput& vTransaction) {
        std::string category;
        std::string operation;
        if (CategorizationEngine::evaluate(m_rules, vTransaction, category, operation)) {
            // only propose a row that actually changes something vs the current values
            const bool categoryChanges = (!category.empty() && category != vTransaction.datas.category.name);
            const bool operationChanges = (!operation.empty() && operation != vTransaction.datas.operation.name);
            if (categoryChanges || operationChanges) {
                CategorizationSuggestion suggestion;
                suggestion.transactionId = vTransaction.id;
                suggestion.date = vTransaction.datas.date;
                suggestion.description = vTransaction.datas.description;
                suggestion.amount = vTransaction.datas.amount;
                suggestion.suggestedCategory = category;
                suggestion.suggestedOperation = operation;
                suggestion.apply = true;
                suggestions.push_back(suggestion);
            }
        }
    });
    MainFrontend::ref().getRuleSuggestionsDialogRef().open(suggestions, categoryNames, operationNames);
}

// rules are persisted in the sqlite database (no xml config)
