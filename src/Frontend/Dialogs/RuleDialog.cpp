#include <Frontend/Dialogs/RuleDialog.h>
#include <Models/DataBase.h>
#include <Panes/RulesPane.h>
#include <Headers/CustomImWidgetsConfig.h>

RuleDialog::RuleDialog() : ADataDialog("RuleModalPopup") {
}

bool RuleDialog::init() {
    return true;
}

void RuleDialog::unit() {
}

void RuleDialog::setRule(const CategorizationRule& vRule) {
    m_rule = vRule;
}

void RuleDialog::m_prepare() {
    m_categoryCombo.clear();
    DataBase::ref().GetCategories([this](const CategoryOutput& vCategory) {  //
        m_categoryCombo.getArrayRef().push_back(vCategory.datas.name);
    });
    m_categoryCombo.getIndexRef() = 0;
    m_categoryCombo.finalize();
    m_operationCombo.clear();
    DataBase::ref().GetOperations([this](const OperationOutput& vOperation) {  //
        m_operationCombo.getArrayRef().push_back(vOperation.datas.name);
    });
    m_operationCombo.getIndexRef() = 0;
    m_operationCombo.finalize();
    m_nameInput.SetText(m_rule.name);
    m_descriptionInput.SetText(m_rule.description);
    m_descPatternInput.SetText(m_rule.descriptionPattern);
    m_commentPatternInput.SetText(m_rule.commentPattern);
    m_entityPatternInput.SetText(m_rule.entityPattern);
    m_categoryCombo.setText(m_rule.targetCategory);
    m_operationCombo.setText(m_rule.targetOperation);
}

const char* RuleDialog::m_getTitle() const {
    switch (getCurrentMode()) {
        case DataDialogMode::MODE_CREATION: return "New rule";
        case DataDialogMode::MODE_UPDATE_ONCE: return "Edit rule";
        default: break;
    }
    return "Rule";
}

bool RuleDialog::m_canConfirm() {
    return !m_nameInput.empty();
}

void RuleDialog::m_drawContent(const ImVec2& /*vPos*/) {
    const float width = 340.0f;
    const float align = 140.0f;

    m_nameInput.DisplayInputText(width, "Name", "", false, align);
    ImGui::SameLine();
    if (ImGui::ContrastedButton(BUTTON_LABEL_RESET, "Reset")) {
        m_nameInput.Clear();
    }
    m_descriptionInput.DisplayInputText(width, "Description", "", false, align);
    ImGui::SameLine();
    if (ImGui::ContrastedButton(BUTTON_LABEL_RESET, "Reset")) {
        m_descriptionInput.Clear();
    }
    ImGui::CheckBoxBoolDefault("Enabled", &m_rule.enabled, true);

    ImGui::Separator();
    ImGui::TextUnformatted("Conditions (an empty field is ignored)");
    m_descPatternInput.DisplayInputText(width, "Description pattern", "", false, align);
    ImGui::SameLine();
    if (ImGui::ContrastedButton(BUTTON_LABEL_RESET, "Reset")) {
        m_descPatternInput.Clear();
    }
    m_commentPatternInput.DisplayInputText(width, "Comment pattern", "", false, align);
    ImGui::SameLine();
    if (ImGui::ContrastedButton(BUTTON_LABEL_RESET, "Reset")) {
        m_commentPatternInput.Clear();
    }
    m_entityPatternInput.DisplayInputText(width, "Entity pattern", "", false, align);
    ImGui::SameLine();
    if (ImGui::ContrastedButton(BUTTON_LABEL_RESET, "Reset")) {
        m_entityPatternInput.Clear();
    }
    ImGui::CheckBoxBoolDefault("Amount range (signed : <0 debit, >0 credit)", &m_rule.useAmountRange, false);
    if (m_rule.useAmountRange) {
        ImGui::InputDoubleDefault(width, "Amount min", &m_rule.amountMin, 0.0);
        ImGui::InputDoubleDefault(width, "Amount max", &m_rule.amountMax, 0.0);
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Assign (an empty field is left unchanged)");
    m_categoryCombo.displayWithColumn(width, "Resulting Category", align);
    m_operationCombo.displayWithColumn(width, "Resulting Operation", align);
}

void RuleDialog::m_confirmDialog() {
    m_rule.name = m_nameInput.GetText();
    m_rule.description = m_descriptionInput.GetText();
    m_rule.descriptionPattern = m_descPatternInput.GetText();
    m_rule.commentPattern = m_commentPatternInput.GetText();
    m_rule.entityPattern = m_entityPatternInput.GetText();
    m_rule.targetCategory = m_categoryCombo.getText();
    m_rule.targetOperation = m_operationCombo.getText();
    // create the typed-in category/operation if it is new (like the income dialog does)
    if (DataBase::ref().OpenDBFile()) {
        if (!m_rule.targetCategory.empty()) {
            CategoryInput categoryInput;
            categoryInput.name = m_rule.targetCategory;
            DataBase::ref().AddCategory(categoryInput);
        }
        if (!m_rule.targetOperation.empty()) {
            OperationInput operationInput;
            operationInput.name = m_rule.targetOperation;
            DataBase::ref().AddOperation(operationInput);
        }
        DataBase::ref().CloseDBFile();
    }
    switch (getCurrentMode()) {
        case DataDialogMode::MODE_CREATION: {
            RulesPane::ref()->getRulesTableRef().addRule(m_rule);
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE: {
            RulesPane::ref()->getRulesTableRef().updateRule(m_rule);
        } break;
        default: break;
    }
}

void RuleDialog::m_cancelDialog() {
}
