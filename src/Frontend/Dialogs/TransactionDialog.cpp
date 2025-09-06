#include "TransactionDialog.h"
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezSha.hpp>

#define MULTIPLE_VALUES "Many values"

TransactionDialog::TransactionDialog() : ADataDialog("TransactionModalPopup") {
}

bool TransactionDialog::init() {
    return true;
}

void TransactionDialog::unit() {

}

void TransactionDialog::setTransaction(const TransactionOutput& vTransaction) {
    m_TransactionToUpdate = vTransaction;
}

void TransactionDialog::setTransactionsToUpdate(const std::vector<TransactionOutput>& vTransactions) {
    m_TransactionsToUpdate = vTransactions;
}

void TransactionDialog::setTransactionsToDelete(const std::vector<TransactionOutput>& vTransactions) {
    m_TransactionsToDelete = vTransactions;
}

void TransactionDialog::m_drawContent(const ImVec2& vPos) {
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

void TransactionDialog::m_drawContentCreation(const ImVec2& vPos) {
    const auto align = 125.0f;
    const auto width = 400.0f;
    m_AccountsCombo.displayWithColumn(width, "Account", align);
    m_EntitiesCombo.displayWithColumn(width, "Entity", align);
    m_CategoriesCombo.displayWithColumn(width, "Category", align);
    m_OperationsCombo.displayWithColumn(width, "Operation", align);
    //m_IncomesCombo.displayWithColumn(width, "Income", align);
    m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
    m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
    m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", true, align);
    ImGui::DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble); });
    ImGui::DisplayAlignedWidget(width, "Confirmed", align, [this]() { ImGui::CheckBoxBoolDefault("##Confirmed", &m_TransactionConfirmed, false); });
}

void TransactionDialog::m_drawContentUpdate(const ImVec2& vPos) {
    const auto align = 125.0f;
    const auto width = 400.0f;
    m_AccountsCombo.displayWithColumn(width, "Account", align);
    m_EntitiesCombo.displayWithColumn(width, "Entity", align);
    m_CategoriesCombo.displayWithColumn(width, "Category", align);
    m_OperationsCombo.displayWithColumn(width, "Operation", align);
    //m_IncomesCombo.displayWithColumn(width, "Income", align);
    m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
    m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
    m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", true, align);
    // the update all if for descriptive items buit not for amounrt
    if (getCurrentMode() != DataDialogMode::MODE_UPDATE_ALL) {
        ImGui::DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_TransactionAmountInputDouble); });
    }
    ImGui::DisplayAlignedWidget(width, "Confirmed", align, [this]() {
        if (!m_TransactionConfirmedManyValues) {
            ImGui::CheckBoxBoolDefault("##Confirmed", &m_TransactionConfirmed, false);
        } else {
            ImGui::Text("%s", "Many Values. not editable");
        }
    });
}

void TransactionDialog::m_drawContentDeletion(const ImVec2& vPos) {
    const auto& displaySize = ImGui::GetIO().DisplaySize * 0.5f;
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##TransactionsToDelete", 5, flags, displaySize)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();
        int32_t idx = 0;
        int32_t idx_to_delete = -1;
        const float& item_h = ImGui::GetTextLineHeightWithSpacing();
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        m_TransactionsDeletionListClipper.Begin((int)m_TransactionsToDelete.size(), item_h);
        while (m_TransactionsDeletionListClipper.Step()) {
            for (idx = m_TransactionsDeletionListClipper.DisplayStart; idx < m_TransactionsDeletionListClipper.DisplayEnd; ++idx) {
                if (idx < 0) {
                    continue;
                }

                const auto& t = m_TransactionsToDelete.at(idx);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                {
                    if (ImGui::SmallContrastedButton("-")) {
                        idx_to_delete = idx;
                    }
                }

                ImGui::TableNextColumn();
                { ImGui::Text("%s", t.datas.date.c_str()); }

                ImGui::TableNextColumn();
                { ImGui::Text("%s", t.datas.description.c_str()); }

                ImGui::TableNextColumn();
                {
                    if (t.amounts.debit < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.amounts.debit);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.amounts.credit > 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.amounts.credit);
                        ImGui::PopStyleColor();
                    }
                }
            }
        }
        m_TransactionsDeletionListClipper.End();
        ImGui::EndTable();

        if (idx_to_delete > -1) {
            m_TransactionsToDelete.erase(m_TransactionsToDelete.begin() + idx_to_delete);
        }
    }
}

void TransactionDialog::m_prepare() {
    m_updateAccounts();
    m_updateEntities();
    m_updateOperations();
    m_updateCategories();
    m_updateIncomes();
    m_TransactionConfirmedManyValues = false;
    if (getCurrentMode() == DataDialogMode::MODE_UPDATE_ALL) {
        if (!m_TransactionsToUpdate.empty()) {
            m_TransactionToUpdate = m_TransactionsToUpdate.at(0);
            for (const auto& t : m_TransactionsToUpdate) {
                if (m_TransactionToUpdate.datas.entity.name != t.datas.entity.name) {
                    m_TransactionToUpdate.datas.entity.name = "Many values";
                }
                if (m_TransactionToUpdate.datas.category.name != t.datas.category.name) {
                    m_TransactionToUpdate.datas.category.name = "Many values";
                }
                if (m_TransactionToUpdate.datas.operation.name != t.datas.operation.name) {
                    m_TransactionToUpdate.datas.operation.name = "Many values";
                }
                if (m_TransactionToUpdate.datas.income.name != t.datas.income.name) {
                    m_TransactionToUpdate.datas.income.name = "Many values";
                }
                if (m_TransactionToUpdate.datas.date != t.datas.date) {
                    m_TransactionToUpdate.datas.date = "Many values";
                }
                if (m_TransactionToUpdate.datas.description != t.datas.description) {
                    m_TransactionToUpdate.datas.description = "Many values";
                }
                if (m_TransactionToUpdate.datas.comment != t.datas.comment) {
                    m_TransactionToUpdate.datas.comment = "Many values";
                }
                if (m_TransactionToUpdate.datas.confirmed != t.datas.confirmed) {
                    m_TransactionConfirmedManyValues = true;
                }
            }
        }
    }
    m_SourceName = m_TransactionToUpdate.datas.source.name;
    m_AccountsCombo.select(m_TransactionToUpdate.accountNumber);
    m_EntitiesCombo.setText(m_TransactionToUpdate.datas.entity.name);
    m_CategoriesCombo.setText(m_TransactionToUpdate.datas.category.name);
    m_OperationsCombo.setText(m_TransactionToUpdate.datas.operation.name);
    //m_IncomesCombo.setText(m_TransactionToUpdate.datas.income.name);
    m_TransactionDateInputText.SetText(m_TransactionToUpdate.datas.date);
    m_TransactionDescriptionInputText.SetText(m_TransactionToUpdate.datas.description);
    m_TransactionCommentInputText.SetText(m_TransactionToUpdate.datas.comment);
    m_TransactionAmountInputDouble = m_TransactionToUpdate.datas.amount;
    m_TransactionConfirmed = m_TransactionToUpdate.datas.confirmed;
}

const char* TransactionDialog::m_getTitle() const {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: {
            return "Transaction Creation";
        } break;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: {
            return "Transaction Deletion";
        } break;
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: {
            return "Transaction Update";
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return "";
}

bool TransactionDialog::m_canConfirm() {
    const auto& mode = getCurrentMode();
    assert(mode != DataDialogMode::MODE_MERGE_ALL);  // not supported, make no sense
    switch (mode) {
        case DataDialogMode::MODE_CREATION: return true;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: return !m_TransactionsToDelete.empty();
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: return true;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }
    return false;
}

void TransactionDialog::m_confirmDialog() {
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
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_confirmDialogUpdateAll();
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }    
}

void TransactionDialog::m_cancelDialog() {
}

void TransactionDialog::m_confirmDialogCreation() {
    RowID account_id = 0U;
    if (DataBase::ref().GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::ref().OpenDBFile()) {
            TransactionInput ti;
            ti.entity.name = m_EntitiesCombo.getText();
            ti.category.name = m_CategoriesCombo.getText();
            ti.operation.name = m_OperationsCombo.getText();
            //ti.income.name = m_IncomesCombo.getText();
            ti.incomeConfirmed = true;
            ti.source.name = m_SourceName;
            ti.source.type = m_SourceType;
            ti.source.sha = m_SourceSha;
            ti.date = m_TransactionDateInputText.GetText();
            ti.description = m_TransactionDescriptionInputText.GetText();
            ti.comment = m_TransactionCommentInputText.GetText();
            ti.amount = m_TransactionAmountInputDouble;
            ti.confirmed = m_TransactionConfirmed;
            DataBase::ref().AddTransactionAutoSha(account_id, ti);
            DataBase::ref().CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogUpdateOnce() {
    RowID account_id = 0U;
    if (DataBase::ref().GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::ref().OpenDBFile()) {
            EntityInput ei;
            ei.name = m_EntitiesCombo.getText();
            DataBase::ref().AddEntity(ei);
            CategoryInput ci;
            ci.name = m_CategoriesCombo.getText();
            DataBase::ref().AddCategory(ci);
            OperationInput oi;
            oi.name = m_OperationsCombo.getText();
            DataBase::ref().AddOperation(oi);
            /*IncomeInput ii;
            ii.name = m_IncomesCombo.getText();
            DataBase::ref().AddIncome(account_id, ii);*/
            TransactionInput ti;
            ti.entity = ei;
            ti.category = ci;
            ti.operation = oi;
            //ti.income = ii;
            ti.incomeConfirmed = true;
            ti.source.name = m_SourceName;
            ti.date = m_TransactionDateInputText.GetText();
            ti.description = m_TransactionDescriptionInputText.GetText();
            ti.comment = m_TransactionCommentInputText.GetText();
            ti.amount = m_TransactionAmountInputDouble;
            ti.confirmed = m_TransactionConfirmed;
            DataBase::ref().UpdateTransaction(m_TransactionToUpdate.id, ti);
            DataBase::ref().CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogUpdateAll() {
    RowID account_id = 0U;
    if (DataBase::ref().GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::ref().OpenDBFile()) {
            if (DataBase::ref().BeginDBTransaction()) {
                for (auto t : m_TransactionsToUpdate) {
                    TransactionInput ti = t.datas;
                    if (m_EntitiesCombo.getText() != MULTIPLE_VALUES) {
                        ti.entity.name = m_EntitiesCombo.getText();
                        DataBase::ref().AddEntity(ti.entity);
                    }
                    if (m_CategoriesCombo.getText() != MULTIPLE_VALUES) {
                        ti.category.name = m_CategoriesCombo.getText();
                        DataBase::ref().AddCategory(ti.category);
                    }
                    if (m_OperationsCombo.getText() != MULTIPLE_VALUES) {
                        ti.operation.name = m_OperationsCombo.getText();
                        DataBase::ref().AddOperation(ti.operation);
                    }
                    if (m_TransactionDateInputText.GetText() != MULTIPLE_VALUES) {
                        ti.date = m_TransactionDateInputText.GetText();
                    }
                    if (m_TransactionDescriptionInputText.GetText() != MULTIPLE_VALUES) {
                        ti.description = m_TransactionDescriptionInputText.GetText();
                    }
                    if (m_TransactionCommentInputText.GetText() != MULTIPLE_VALUES) {
                        ti.comment = m_TransactionCommentInputText.GetText();
                    }
                    if (!m_TransactionConfirmedManyValues) {
                        ti.confirmed = m_TransactionConfirmed;
                    }
                    ti.sha =        //
                        ez::sha1()  //
                            .add(ti.date)
                            // un fichier ofc ne peut pas avoir des description de longueur > a 30
                            // alors on limite le sha a utiliser un description de 30
                            // comme cela un ofc ne rentrera pas un collision avec un autre type de fcihier comme les pdf par ex
                            .add(ti.description.substr(0, 30))
                            // must be unique per oepration
                            .addValue(ti.amount)
                            .finalize()
                            .getHex();
                    DataBase::ref().UpdateTransaction(t.id, ti);
                }
                DataBase::ref().CommitDBTransaction();
            }
            DataBase::ref().CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogDeletion() {
    std::set<RowID> m_rows;
    for (const auto& t : m_TransactionsToDelete) {
        m_rows.emplace(t.id);
    }
    if (!m_rows.empty()) {
        DataBase::ref().DeleteTransactions(m_rows);
    }
}

void TransactionDialog::m_updateAccounts() {
    m_AccountsCombo.clear();
    DataBase::ref().GetAccounts(                 //
        [this](const AccountOutput& vAccountOutput) {  //
            m_AccountsCombo.getArrayRef().push_back(vAccountOutput.datas.number);
        });
    m_AccountsCombo.getIndexRef() = 0;
}

void TransactionDialog::m_updateEntities() {
    m_EntitiesCombo.clear();
    DataBase::ref().GetEntities(         //
        [this](const EntityOutput& vEntityOutput) {  //
            m_EntitiesCombo.getArrayRef().push_back(vEntityOutput.datas.name);
        });
    m_EntitiesCombo.getIndexRef() = 0;
    m_EntitiesCombo.finalize();
}

void TransactionDialog::m_updateOperations() {
    m_OperationsCombo.clear();
    DataBase::ref().GetOperations(               //
        [this](const OperationOutput& vOperationOutput) {  //
            m_OperationsCombo.getArrayRef().push_back(vOperationOutput.datas.name);
        });
    m_OperationsCombo.getIndexRef() = 0;
    m_OperationsCombo.finalize();
}

void TransactionDialog::m_updateCategories() {
    m_CategoriesCombo.clear();
    DataBase::ref().GetCategories(             //
        [this](const CategoryOutput& vCategoryOutput) {  //
            m_CategoriesCombo.getArrayRef().push_back(vCategoryOutput.datas.name);
        });
    m_CategoriesCombo.getIndexRef() = 0;
    m_CategoriesCombo.finalize();
}

void TransactionDialog::m_updateIncomes() {
    /*RowID account_id = 0U;
    if (DataBase::ref().GetAccount(m_AccountsCombo.getText(), account_id)) {
        m_IncomesCombo.clear();
        DataBase::ref().GetIncomes(  //
            account_id,
            [this](const IncomeOutput& vIncomeOutput) {  //
                m_IncomesCombo.getArrayRef().push_back(vIncomeOutput.datas.name);
            });
        m_IncomesCombo.getIndexRef() = 0;
        m_IncomesCombo.finalize();
    }*/
}
