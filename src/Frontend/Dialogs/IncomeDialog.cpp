#include "IncomeDialog.h"
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>

#define MULTIPLE_VALUES "Many values"

IncomeDialog::IncomeDialog() : ADataDialog("IncomeModalPopup") {
}

bool IncomeDialog::init() {
    return true;
}

void IncomeDialog::unit() {

}

void IncomeDialog::setTransactions(const std::vector<TransactionOutput>& vTransactions) {
    m_TransactionToAddAsIncomes = vTransactions;
}

void IncomeDialog::setIncome(const IncomeOutput& vIncome) {
    m_IncomeToUpdate = vIncome;
}

void IncomeDialog::setIncomesToUpdate(const std::vector<IncomeOutput>& vIncomes) {
    m_IncomesToUpdate = vIncomes;
}

void IncomeDialog::setIncomesToDelete(const std::vector<IncomeOutput>& vIncomes) {
    m_IncomesToDelete = vIncomes;
}

void IncomeDialog::m_drawContent(const ImVec2& vPos) {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_drawContentCreation(vPos);
        } break;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: {
            m_drawContentDeletion(vPos);
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_drawContentUpdate(vPos);
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
}

void IncomeDialog::m_prepare() {
    m_updateAccounts();
    m_updateEntities();
    m_updateOperations();
    m_updateCategories();
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            if (!m_TransactionToAddAsIncomes.empty()) {
                const auto& trans = m_TransactionToAddAsIncomes.at(0);
                m_IncomeToUpdate.accountNumber = trans.accountNumber;
                m_IncomeToUpdate.datas.category.name = trans.datas.category.name;
                m_IncomeToUpdate.datas.entity.name = trans.datas.entity.name;
                m_IncomeToUpdate.datas.operation.name = trans.datas.operation.name;
                m_IncomeToUpdate.datas.description = trans.datas.description;
                m_IncomeToUpdate.datas.minAmount = trans.datas.amount;
                m_IncomeToUpdate.datas.maxAmount = trans.datas.amount;
                m_IncomeToUpdate.datas.startDate = trans.datas.date;
                m_IncomeToUpdate.startEpoch = trans.dateEpoch;
                m_IncomeToUpdate.datas.minDay = ez::time::decomposeEpoch(trans.dateEpoch).tm_mday;
                m_IncomeToUpdate.datas.maxDay = m_IncomeToUpdate.datas.minDay;
                for (const auto& t : m_TransactionToAddAsIncomes) {
                    if (m_IncomeToUpdate.datas.entity.name != t.datas.entity.name) {
                        m_IncomeToUpdate.datas.entity.name = "Many values";
                    }
                    if (m_IncomeToUpdate.datas.category.name != t.datas.category.name) {
                        m_IncomeToUpdate.datas.category.name = "Many values";
                    }
                    if (m_IncomeToUpdate.datas.operation.name != t.datas.operation.name) {
                        m_IncomeToUpdate.datas.operation.name = "Many values";
                    }
                    if (m_IncomeToUpdate.datas.description != t.datas.description) {
                        m_IncomeToUpdate.datas.description = "Many values";
                    }
                    if (t.datas.amount < m_IncomeToUpdate.datas.minAmount) {
                        m_IncomeToUpdate.datas.minAmount = t.datas.amount;
                    }
                    if (t.datas.amount > m_IncomeToUpdate.datas.maxAmount) {
                        m_IncomeToUpdate.datas.maxAmount = t.datas.amount;
                    }
                    if (t.dateEpoch < m_IncomeToUpdate.startEpoch) {
                        m_IncomeToUpdate.startEpoch = t.dateEpoch;
                        m_IncomeToUpdate.datas.startDate = t.datas.date;
                    }
                    auto day = ez::time::decomposeEpoch(t.dateEpoch).tm_mday;
                    if (day < m_IncomeToUpdate.datas.minDay) {
                        m_IncomeToUpdate.datas.minDay = day;
                    }
                    if (day > m_IncomeToUpdate.datas.maxDay) {
                        m_IncomeToUpdate.datas.maxDay = day;
                    }
                }
            }
        } break;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: {
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    m_IncomeNameInputText.SetText(m_IncomeToUpdate.datas.name);
    m_AccountsCombo.select(m_IncomeToUpdate.accountNumber);
    m_EntitiesCombo.setText(m_IncomeToUpdate.datas.entity.name);
    m_CategoriesCombo.setText(m_IncomeToUpdate.datas.category.name);
    m_OperationsCombo.setText(m_IncomeToUpdate.datas.operation.name);
    m_IncomeStartDateInputText.SetText(m_IncomeToUpdate.datas.startDate);
    m_IncomeEndDateInputText.SetText(m_IncomeToUpdate.datas.endDate);
    m_IncomeMinDayInputInt32 = m_IncomeToUpdate.datas.minDay;
    m_IncomeMaxDayInputInt32 = m_IncomeToUpdate.datas.maxDay;
    m_IncomeMinAmountInputDouble = m_IncomeToUpdate.datas.minAmount;
    m_IncomeMaxAmountInputDouble = m_IncomeToUpdate.datas.maxAmount;
    m_IncomeDescriptionInputText.SetText(m_IncomeToUpdate.datas.description);
}

void IncomeDialog::m_drawContentCreation(const ImVec2& vPos) {
    const auto align = 125.0f;
    const auto width = 400.0f;
    m_IncomeNameInputText.DisplayInputText(width, "Name", "", false, align);
    m_AccountsCombo.displayWithColumn(width, "Account", align);
    m_EntitiesCombo.displayWithColumn(width, "Entity", align);
    m_CategoriesCombo.displayWithColumn(width, "Category", align);
    m_OperationsCombo.displayWithColumn(width, "Operation", align);
    m_IncomeStartDateInputText.DisplayInputText(width, "Start date", "", false, align);
    m_IncomeEndDateInputText.DisplayInputText(width, "End date", "", false, align);
    m_IncomeMinAmountInputDouble.displayInputValue(width, "Min amount", 0.0, 0.1, 1.0, "%.6f", align, [this](double vDefaultValue, double vValue) {
        return (m_IncomeMaxAmountInputDouble.get() >= vValue);
    });
    m_IncomeMaxAmountInputDouble.displayInputValue(width, "Max amount", 0.0, 0.1, 1.0, "%.6f", align, [this](double vDefaultValue, double vValue) {
        return (m_IncomeMinAmountInputDouble.get() <= vValue);
    });
    m_IncomeMinDayInputInt32.displayInputValue(width, "Min day", 0, 1, 1, "%i", align, [this](int32_t& vDefaultValue, int32_t& vValue) {
        if (vValue < -31) {
            vValue = -31;
        }
        if (vValue > 31) {
            vValue = 31;
        }
        return (m_IncomeMaxDayInputInt32.get() >= vValue);
    });
    m_IncomeMaxDayInputInt32.displayInputValue(width, "Max day", 0, 1, 1, "%i", align, [this](int32_t& vDefaultValue, int32_t& vValue) {
        if (vValue < 0) {
            vValue = 0;
        }
        if (vValue > 31) {
            vValue = 31;
        }
        return (m_IncomeMinDayInputInt32.get() <= vValue);
    });
    m_IncomeDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
}

void IncomeDialog::m_drawContentUpdate(const ImVec2& vPos) {
    m_drawContentCreation(vPos);
}

void IncomeDialog::m_drawContentDeletion(const ImVec2& vPos) {
    const auto& displaySize = ImGui::GetIO().DisplaySize * 0.5f;
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##IncomesToDelete", 9, flags, displaySize)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("StartDate", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("EndDate", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("MinDay", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("MaxDay", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("MinAmount", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("MaxAmount", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        int32_t idx = 0;
        int32_t idx_to_delete = -1;
        const float& item_h = ImGui::GetTextLineHeightWithSpacing();
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        m_IncomesDeletionListClipper.Begin((int)m_IncomesToDelete.size(), item_h);
        while (m_IncomesDeletionListClipper.Step()) {
            for (idx = m_IncomesDeletionListClipper.DisplayStart; idx < m_IncomesDeletionListClipper.DisplayEnd; ++idx) {
                if (idx < 0) {
                    continue;
                }

                const auto& t = m_IncomesToDelete.at(idx);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                if (ImGui::SmallContrastedButton("-")) {
                    idx_to_delete = idx;
                }

                ImGui::TableNextColumn();
                ImGui::Text("%s", t.datas.name.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", t.datas.startDate.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", t.datas.endDate.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%i", t.datas.minDay);

                ImGui::TableNextColumn();
                ImGui::Text("%u", t.datas.maxDay);

                ImGui::TableNextColumn();
                {
                    ImU32 color = ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text));
                    if (t.datas.minAmount < 0.0) {
                        color = bad_color;
                    } else if (t.datas.minAmount > 0.0) {
                        color = good_color;
                    }

                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("%.2f", t.datas.minAmount);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                {
                    ImU32 color = ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text));
                    if (t.datas.maxAmount < 0.0) {
                        color = bad_color;
                    } else if (t.datas.maxAmount > 0.0) {
                        color = good_color;
                    }

                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("%.2f", t.datas.maxAmount);
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                ImGui::Text("%s", t.datas.description.c_str());
            }
        }
        m_IncomesDeletionListClipper.End();
        ImGui::EndTable();

        if (idx_to_delete > -1) {
            m_IncomesToDelete.erase(m_IncomesToDelete.begin() + idx_to_delete);
        }
    }
}

const char* IncomeDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            return "Income Creation";
        } break;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: {
            return "Income Deletion";
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            return "Income Update";
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return "";
}

bool IncomeDialog::m_canConfirm() {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: return !m_IncomesToDelete.empty();
        case DataDialogMode::MODE_CREATION:
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            if ((m_IncomeMaxAmountInputDouble.get() < m_IncomeMinAmountInputDouble.get()) ||  //
                (m_IncomeMinDayInputInt32.get() > m_IncomeMaxDayInputInt32.get())) {
                return false;
            }
        }
            return true;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return false;
}

void IncomeDialog::m_confirmDialog() {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            m_confirmDialogCreation();
        } break;
        case DataDialogMode::MODE_DELETE_ONCE: 
        case DataDialogMode::MODE_DELETE_ALL: {
            m_confirmDialogDeletion();
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE: {
            m_confirmDialogUpdateOnce();
        } break;
        case DataDialogMode::MODE_UPDATE_ALL:
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }    
}

void IncomeDialog::m_cancelDialog() {
}

void IncomeDialog::m_confirmDialogCreation() {
    RowID account_id = 0U;
    if (DataBase::ref().GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::ref().OpenDBFile()) {
            IncomeInput ii;
            ii.account_id = account_id;
            ii.name = m_IncomeNameInputText.GetText();
            ii.category.name = m_CategoriesCombo.getText();
            ii.entity.name = m_EntitiesCombo.getText();
            ii.operation.name = m_OperationsCombo.getText();
            ii.description = m_IncomeDescriptionInputText.GetText();
            ii.minAmount = m_IncomeMinAmountInputDouble.get();
            ii.maxAmount = m_IncomeMaxAmountInputDouble.get();
            ii.startDate = m_IncomeStartDateInputText.GetText();
            ii.endDate = m_IncomeEndDateInputText.GetText();
            ii.minDay = m_IncomeMinDayInputInt32.get();
            ii.maxDay = m_IncomeMaxDayInputInt32.get();
            DataBase::ref().AddIncome(account_id, ii);
            DataBase::ref().CloseDBFile();
        }
    }
}

void IncomeDialog::m_confirmDialogUpdateOnce() {
    RowID account_id = 0U;
    if (DataBase::ref().GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::ref().OpenDBFile()) {
            IncomeInput ii;
            ii.account_id = account_id;
            ii.name = m_IncomeNameInputText.GetText();
            ii.category.name = m_CategoriesCombo.getText();
            ii.entity.name = m_EntitiesCombo.getText();
            ii.operation.name = m_OperationsCombo.getText();
            ii.description = m_IncomeDescriptionInputText.GetText();
            ii.minAmount = m_IncomeMinAmountInputDouble.get();
            ii.maxAmount = m_IncomeMaxAmountInputDouble.get();
            ii.startDate = m_IncomeStartDateInputText.GetText();
            ii.endDate = m_IncomeEndDateInputText.GetText();
            ii.minDay = m_IncomeMinDayInputInt32.get();
            ii.maxDay = m_IncomeMaxDayInputInt32.get();
            DataBase::ref().UpdateIncome(m_IncomeToUpdate.id, ii);
            DataBase::ref().CloseDBFile();
        }
    }
}

void IncomeDialog::m_confirmDialogDeletion() {
    std::set<RowID> m_rows;
    for (const auto& t : m_IncomesToDelete) {
        m_rows.emplace(t.id);
    }
    if (!m_rows.empty()) {
        DataBase::ref().DeleteIncomes(m_rows);
    }
}

void IncomeDialog::m_updateAccounts() {
    m_AccountsCombo.clear();
    DataBase::ref().GetAccounts(  //
        [this](const AccountOutput& vAccountOutput) {  //
            m_AccountsCombo.getArrayRef().push_back(vAccountOutput.datas.number);
        });
    m_AccountsCombo.getIndexRef() = 0;
}

void IncomeDialog::m_updateEntities() {
    m_EntitiesCombo.clear();
    DataBase::ref().GetEntities(           //
        [this](const EntityOutput& vEntityOutput) {  //
            m_EntitiesCombo.getArrayRef().push_back(vEntityOutput.datas.name);
        });
    m_EntitiesCombo.getIndexRef() = 0;
    m_EntitiesCombo.finalize();
}

void IncomeDialog::m_updateOperations() {
    m_OperationsCombo.clear();
    DataBase::ref().GetOperations(               //
        [this](const OperationOutput& vOperationOutput) {  //
            m_OperationsCombo.getArrayRef().push_back(vOperationOutput.datas.name);
        });
    m_OperationsCombo.getIndexRef() = 0;
    m_OperationsCombo.finalize();
}

void IncomeDialog::m_updateCategories() {
    m_CategoriesCombo.clear();
    DataBase::ref().GetCategories(             //
        [this](const CategoryOutput& vOperationOutput) {  //
            m_CategoriesCombo.getArrayRef().push_back(vOperationOutput.datas.name);
        });
    m_CategoriesCombo.getIndexRef() = 0;
    m_CategoriesCombo.finalize();
}
