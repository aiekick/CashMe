#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <Headers/DatasDef.h>
#include <Frontend/Dialogs/abstract/ADataDialog.h>

class RuleDialog : public ADataDialog {
private:
    CategorizationRule m_rule;
    ImWidgets::InputText m_nameInput;
    ImWidgets::InputText m_descriptionInput;
    ImWidgets::InputText m_descPatternInput;
    ImWidgets::InputText m_commentPatternInput;
    ImWidgets::InputText m_entityPatternInput;
    ImWidgets::QuickStringEditCombo m_categoryCombo;
    ImWidgets::QuickStringEditCombo m_operationCombo;

public:
    RuleDialog();
    bool init() override;
    void unit() override;
    void setRule(const CategorizationRule& vRule);

protected:
    void m_drawContent(const ImVec2& vPos) override;
    void m_prepare() override;
    const char* m_getTitle() const override;
    bool m_canConfirm() override;
    void m_confirmDialog() override;
    void m_cancelDialog() override;
};
