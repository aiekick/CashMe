#include "TransactionDialog.h"
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>

#define MULTIPLE_VALUES "Many values"

TransactionDialog::TransactionDialog() : ADataDialog("TransactionModalPopup") {
}

bool TransactionDialog::init() {
    return true;
}

void TransactionDialog::unit() {

}

void TransactionDialog::setTransaction(const Transaction& vTransaction) {
    m_TransactionToUpdate = vTransaction;
}

void TransactionDialog::setTransactionsToUpdate(const std::vector<Transaction>& vTransactions) {
    m_TransactionsToUpdate = vTransactions;
}

void TransactionDialog::setTransactionsToDelete(const std::vector<Transaction>& vTransactions) {
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
    m_TransactionDateInputText.DisplayInputText(width, "Date", "", false, align);
    m_TransactionDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
    m_TransactionCommentInputText.DisplayInputText(width, "Comment", "", false, align);
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
                { ImGui::Text("%s", t.date.c_str()); }

                ImGui::TableNextColumn();
                { ImGui::Text("%s", t.description.c_str()); }

                ImGui::TableNextColumn();
                {
                    if (t.debit < 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
                        ImGui::Text("%.2f", t.debit);
                        ImGui::PopStyleColor();
                    }
                }

                ImGui::TableNextColumn();
                {
                    if (t.credit > 0.0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
                        ImGui::Text("%.2f", t.credit);
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
    m_UpdateAccounts();
    m_UpdateEntities();
    m_UpdateOperations();
    m_UpdateCategories();
    m_TransactionConfirmedManyValues = false;
    if (getCurrentMode() == DataDialogMode::MODE_UPDATE_ALL) {
        if (!m_TransactionsToUpdate.empty()) {
            m_TransactionToUpdate = m_TransactionsToUpdate.at(0);
            for (const auto& t : m_TransactionsToUpdate) {
                if (m_TransactionToUpdate.entity != t.entity) {
                    m_TransactionToUpdate.entity = "Many values";
                }
                if (m_TransactionToUpdate.category != t.category) {
                    m_TransactionToUpdate.category = "Many values";
                }
                if (m_TransactionToUpdate.operation != t.operation) {
                    m_TransactionToUpdate.operation = "Many values";
                }
                if (m_TransactionToUpdate.date != t.date) {
                    m_TransactionToUpdate.date = "Many values";
                }
                if (m_TransactionToUpdate.description != t.description) {
                    m_TransactionToUpdate.description = "Many values";
                }
                if (m_TransactionToUpdate.comment != t.comment) {
                    m_TransactionToUpdate.comment = "Many values";
                }
                if (m_TransactionToUpdate.confirmed != t.confirmed) {
                    m_TransactionConfirmedManyValues = true;
                }
            }
        }
    }
    m_SourceName = m_TransactionToUpdate.source;
    m_AccountsCombo.select(m_TransactionToUpdate.account);
    m_EntitiesCombo.setText(m_TransactionToUpdate.entity);
    m_CategoriesCombo.setText(m_TransactionToUpdate.category);
    m_OperationsCombo.setText(m_TransactionToUpdate.operation);
    m_TransactionDateInputText.SetText(m_TransactionToUpdate.date);
    m_TransactionDescriptionInputText.SetText(m_TransactionToUpdate.description);
    m_TransactionCommentInputText.SetText(m_TransactionToUpdate.comment);
    m_TransactionAmountInputDouble = m_TransactionToUpdate.amount;
    m_TransactionConfirmed = m_TransactionToUpdate.confirmed;
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
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            const auto hash = ez::str::toStr(  //
                "%s_%s_%f",               //
                m_TransactionDateInputText.GetText().c_str(),
                // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                // alors on limite le hash a utiliser un label de 30
                // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                m_TransactionAmountInputDouble);              // must be unique per oepration
            DataBase::Instance()->AddTransaction(             //
                account_id,                                   //
                m_EntitiesCombo.getText(),                    //
                m_CategoriesCombo.getText(),                  //
                m_OperationsCombo.getText(),                  //
                m_SourceName,                                 //
                m_SourceType,                                 //
                m_SourceSha,                                  //
                m_TransactionDateInputText.GetText(),         //
                m_TransactionDescriptionInputText.GetText(),  //
                m_TransactionCommentInputText.GetText(),      //
                m_TransactionAmountInputDouble,               //
                false,                                        //
                hash);
            DataBase::Instance()->CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogUpdateOnce() {
    RowID account_id = 0U;
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            const auto hash = ez::str::toStr(  //
                "%s_%s_%f",               //
                m_TransactionDateInputText.GetText().c_str(),
                // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                // alors on limite le hash a utiliser un label de 30
                // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                m_TransactionDescriptionInputText.GetText().substr(0, 30).c_str(),
                m_TransactionAmountInputDouble);  // must be unique per operation
            DataBase::Instance()->AddEntity(m_EntitiesCombo.getText());
            DataBase::Instance()->AddCategory(m_CategoriesCombo.getText());
            DataBase::Instance()->AddOperation(m_OperationsCombo.getText());
            DataBase::Instance()->UpdateTransaction(          //
                m_TransactionToUpdate.id,                     //
                m_EntitiesCombo.getText(),                    //
                m_CategoriesCombo.getText(),                  //
                m_OperationsCombo.getText(),                  //
                m_SourceName,                                 //
                m_TransactionDateInputText.GetText(),         //
                m_TransactionDescriptionInputText.GetText(),  //
                m_TransactionCommentInputText.GetText(),      //
                m_TransactionAmountInputDouble,               //
                false,                                        //
                hash);
            DataBase::Instance()->CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogUpdateAll() {
    RowID account_id = 0U;
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            if (DataBase::Instance()->BeginTransaction()) {
                for (auto t : m_TransactionsToUpdate) {
                    if (m_EntitiesCombo.getText() != MULTIPLE_VALUES) {
                        t.entity = m_EntitiesCombo.getText();
                        DataBase::Instance()->AddEntity(t.entity);
                    }
                    if (m_CategoriesCombo.getText() != MULTIPLE_VALUES) {
                        t.category = m_CategoriesCombo.getText();
                        DataBase::Instance()->AddCategory(t.category);
                    }
                    if (m_OperationsCombo.getText() != MULTIPLE_VALUES) {
                        t.operation = m_OperationsCombo.getText();
                        DataBase::Instance()->AddOperation(t.operation);
                    }
                    if (m_TransactionDateInputText.GetText() != MULTIPLE_VALUES) {
                        t.date = m_TransactionDateInputText.GetText();
                    }
                    if (m_TransactionDescriptionInputText.GetText() != MULTIPLE_VALUES) {
                        t.description = m_TransactionDescriptionInputText.GetText();
                    }
                    if (m_TransactionCommentInputText.GetText() != MULTIPLE_VALUES) {
                        t.comment = m_TransactionCommentInputText.GetText();
                    }
                    if (!m_TransactionConfirmedManyValues) {
                        t.confirmed = m_TransactionConfirmed;
                    }
                    DataBase::Instance()->UpdateTransaction(  //
                        t.id,                                 //
                        t.entity,                             //
                        t.category,                           //
                        t.operation,                          //
                        t.source,                             //
                        t.date,                               //
                        t.description,                        //
                        t.comment,                            //
                        t.amount,                             //
                        t.confirmed,                          //
                        t.hash);
                }
                DataBase::Instance()->CommitTransaction();
            }
            DataBase::Instance()->CloseDBFile();
        }
    }
}

void TransactionDialog::m_confirmDialogDeletion() {
    std::set<RowID> m_rows;
    for (const auto& t : m_TransactionsToDelete) {
        m_rows.emplace(t.id);
    }
    if (!m_rows.empty()) {
        DataBase::Instance()->DeleteTransactions(m_rows);
    }
}

void TransactionDialog::m_UpdateAccounts() {
    m_AccountsCombo.clear();
    DataBase::Instance()->GetAccounts(  //
        [this](const RowID& vRowID,
               const BankName& vBankName,
               const BankAgency& vBankAgency,
               const AccountType& vAccountType,
               const AccountName& vAccountName,
               const AccountNumber& vAccountNumber,
               const AccountBaseSolde& vAccounBaseSolde,
               const TransactionsCount& vTransactionsCount) {  //
            m_AccountsCombo.getArrayRef().push_back(vAccountNumber);
        });
    m_AccountsCombo.getIndexRef() = 0;
}

void TransactionDialog::m_UpdateEntities() {
    m_EntitiesCombo.clear();
    DataBase::Instance()->GetOperations(         //
        [this](const EntityName& vEntityName) {  //
            m_EntitiesCombo.getArrayRef().push_back(vEntityName);
        });
    m_EntitiesCombo.getIndexRef() = 0;
    m_EntitiesCombo.finalize();
}

void TransactionDialog::m_UpdateOperations() {
    m_OperationsCombo.clear();
    DataBase::Instance()->GetOperations(               //
        [this](const OperationName& vOperationName) {  //
            m_OperationsCombo.getArrayRef().push_back(vOperationName);
        });
    m_OperationsCombo.getIndexRef() = 0;
    m_OperationsCombo.finalize();
}

void TransactionDialog::m_UpdateCategories() {
    m_CategoriesCombo.clear();
    DataBase::Instance()->GetCategories(             //
        [this](const CategoryName& vCategoryName) {  //
            m_CategoriesCombo.getArrayRef().push_back(vCategoryName);
        });
    m_CategoriesCombo.getIndexRef() = 0;
    m_CategoriesCombo.finalize();
}
