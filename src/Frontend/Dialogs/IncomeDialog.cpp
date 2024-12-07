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

void IncomeDialog::setTransactions(const std::vector<Transaction>& vTransactions) {
    m_TransactionToAddAsIncomes = vTransactions;
}

void IncomeDialog::setIncome(const Income& vIncome) {
    m_IncomeToUpdate = vIncome;
}

void IncomeDialog::setIncomesToUpdate(const std::vector<Income>& vIncomes) {
    m_IncomesToUpdate = vIncomes;
}

void IncomeDialog::setIncomesToDelete(const std::vector<Income>& vIncomes) {
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
    ImGui::DisplayAlignedWidget(width, "Min amount", align, [this]() { ImGui::InputDouble("##MinAmount", &m_IncomeMinAmountInputDouble); });
    ImGui::DisplayAlignedWidget(width, "Max amount", align, [this]() { ImGui::InputDouble("##MaxAmount", &m_IncomeMaxAmountInputDouble); });
    ImGui::DisplayAlignedWidget(width, "Min day", align, [this]() { ImGui::InputInt("##MinDay", &m_IncomeMinDayInputInt32); });
    ImGui::DisplayAlignedWidget(width, "Max day", align, [this]() { ImGui::InputInt("##MaxDay", &m_IncomeMaxDayInputInt32); });
    m_IncomeDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
}

void IncomeDialog::m_drawContentUpdate(const ImVec2& vPos) {
    /*const auto align = 125.0f;
    const auto width = 400.0f;
    m_AccountsCombo.displayWithColumn(width, "Account", align);
    m_EntitiesCombo.displayWithColumn(width, "Entity", align);
    m_CategoriesCombo.displayWithColumn(width, "Category", align);
    m_OperationsCombo.displayWithColumn(width, "Operation", align);
    m_IncomeDateInputText.DisplayInputText(width, "Date", "", false, align);
    m_IncomeDescriptionInputText.DisplayInputText(width, "Description", "", false, align);
    m_IncomeCommentInputText.DisplayInputText(width, "Comment", "", false, align);
    // the update all if for descriptive items buit not for amounrt
    if (getCurrentMode() != DataDialogMode::MODE_UPDATE_ALL) {
        ImGui::DisplayAlignedWidget(width, "Amount", align, [this]() { ImGui::InputDouble("##Amount", &m_IncomeAmountInputDouble); });
    }*/
}

void IncomeDialog::m_drawContentDeletion(const ImVec2& vPos) {
    /*const auto& displaySize = ImGui::GetIO().DisplaySize * 0.5f;
    static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("##IncomesToDelete", 5, flags, displaySize)) {
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
        m_IncomesDeletionListClipper.Begin((int)m_IncomesToDelete.size(), item_h);
        while (m_IncomesDeletionListClipper.Step()) {
            for (idx = m_IncomesDeletionListClipper.DisplayStart; idx < m_IncomesDeletionListClipper.DisplayEnd; ++idx) {
                if (idx < 0) {
                    continue;
                }

                const auto& t = m_IncomesToDelete.at(idx);

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                {
                    if (ImGui::SmallContrastedButton("-")) {
                        idx_to_delete = idx;
                    }
                }

                ImGui::TableNextColumn();
                { ImGui::Text(t.date.c_str()); }

                ImGui::TableNextColumn();
                { ImGui::Text(t.description.c_str()); }

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
        m_IncomesDeletionListClipper.End();
        ImGui::EndTable();

        if (idx_to_delete > -1) {
            m_IncomesToDelete.erase(m_IncomesToDelete.begin() + idx_to_delete);
        }
    }*/
}

void IncomeDialog::m_prepare() {
    m_UpdateAccounts();
    m_UpdateEntities();
    m_UpdateOperations();
    m_UpdateCategories();
    if (getCurrentMode() == DataDialogMode::MODE_CREATION) {
        if (!m_TransactionToAddAsIncomes.empty()) {
            const auto& trans = m_TransactionToAddAsIncomes.at(0);
            m_IncomeToUpdate.account = trans.account;
            m_IncomeToUpdate.category = trans.category;
            m_IncomeToUpdate.entity = trans.entity;
            m_IncomeToUpdate.operation = trans.operation;
            m_IncomeToUpdate.description = trans.description;
            m_IncomeToUpdate.maxAmount = trans.amount;
            m_IncomeToUpdate.minAmount = trans.amount;
            m_IncomeToUpdate.startDate = trans.date;
            m_IncomeToUpdate.startDateEpoch = trans.epoch;
            m_IncomeToUpdate.minDay = ez::time::decomposeEpoch(trans.epoch).tm_mday;
            m_IncomeToUpdate.maxDay = m_IncomeToUpdate.minDay;  
            for (const auto& t : m_TransactionToAddAsIncomes) {
                if (m_IncomeToUpdate.entity != t.entity) {
                    m_IncomeToUpdate.entity = "Many values";
                }
                if (m_IncomeToUpdate.category != t.category) {
                    m_IncomeToUpdate.category = "Many values";
                }
                if (m_IncomeToUpdate.operation != t.operation) {
                    m_IncomeToUpdate.operation = "Many values";
                }
                if (m_IncomeToUpdate.description != t.description) {
                    m_IncomeToUpdate.description = "Many values";
                }
                if (t.amount < m_IncomeToUpdate.minAmount) {
                    m_IncomeToUpdate.minAmount = t.amount;
                }
                if (t.amount > m_IncomeToUpdate.maxAmount) {
                    m_IncomeToUpdate.maxAmount = t.amount;
                }
                if (t.epoch < m_IncomeToUpdate.startDateEpoch) {
                    m_IncomeToUpdate.startDateEpoch = t.epoch;
                    m_IncomeToUpdate.startDate = t.date;
                }
                auto day = ez::time::decomposeEpoch(t.epoch).tm_mday;
                if (day < m_IncomeToUpdate.minDay) {
                    m_IncomeToUpdate.minDay = day;
                }
                if (day > m_IncomeToUpdate.maxDay) {
                    m_IncomeToUpdate.maxDay = day;
                }
            }
        }
    }
    m_AccountsCombo.select(m_IncomeToUpdate.account);
    m_EntitiesCombo.setText(m_IncomeToUpdate.entity);
    m_CategoriesCombo.setText(m_IncomeToUpdate.category);
    m_OperationsCombo.setText(m_IncomeToUpdate.operation);
    m_IncomeStartDateInputText.SetText(m_IncomeToUpdate.startDate);
    m_IncomeEndDateInputText.SetText(m_IncomeToUpdate.endDate);
    m_IncomeMinDayInputInt32 = m_IncomeToUpdate.minDay;
    m_IncomeMaxDayInputInt32 = m_IncomeToUpdate.maxDay;
    m_IncomeMinAmountInputDouble = m_IncomeToUpdate.minAmount;
    m_IncomeMaxAmountInputDouble = m_IncomeToUpdate.maxAmount;
    m_IncomeDescriptionInputText.SetText(m_IncomeToUpdate.description);
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
        case DataDialogMode::MODE_CREATION: return true;
        case DataDialogMode::MODE_DELETE_ONCE:
        case DataDialogMode::MODE_DELETE_ALL: return !m_IncomesToDelete.empty();
        case DataDialogMode::MODE_UPDATE_ONCE:
        case DataDialogMode::MODE_UPDATE_ALL: return true;
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
        case DataDialogMode::MODE_UPDATE_ALL: {
            m_confirmDialogUpdateAll();
        } break;
        case DataDialogMode::MODE_MERGE_ALL:
        case DataDialogMode::MODE_NONE:
        default: break;
    }    
}

void IncomeDialog::m_cancelDialog() {
}

void IncomeDialog::m_confirmDialogCreation() {
    RowID account_id = 0U;
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            DataBase::Instance()->AddIncome(  //
                account_id,
                m_IncomeNameInputText.GetText(),
                m_EntitiesCombo.getText(),
                m_CategoriesCombo.getText(),
                m_OperationsCombo.getText(),
                m_IncomeStartDateInputText.GetText(),
                m_IncomeEndDateInputText.GetText(),
                m_IncomeMinAmountInputDouble,
                m_IncomeMaxAmountInputDouble,
                m_IncomeMinDayInputInt32,
                m_IncomeMaxDayInputInt32,
                m_IncomeDescriptionInputText.GetText());
            DataBase::Instance()->CloseDBFile();
        }
    }
}

void IncomeDialog::m_confirmDialogUpdateOnce() {
    /*RowID account_id = 0U;
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            const auto hash = ez::str::toStr(  //
                "%s_%s_%f",               //
                m_IncomeDateInputText.GetText().c_str(),
                // un fichier ofc ne peut pas avoir des labels de longueur > a 30
                // alors on limite le hash a utiliser un label de 30
                // comme cela un ofc ne rentrera pas en collision avec un autre type de fichier comme les pdf par ex
                m_IncomeDescriptionInputText.GetText().substr(0, 30).c_str(),
                m_IncomeAmountInputDouble);  // must be unique per operation
            DataBase::Instance()->AddEntity(m_EntitiesCombo.getText());
            DataBase::Instance()->AddCategory(m_CategoriesCombo.getText());
            DataBase::Instance()->AddOperation(m_OperationsCombo.getText());
            DataBase::Instance()->UpdateIncome(          //
                m_IncomeToUpdate.id,                     //
                m_EntitiesCombo.getText(),                    //
                m_CategoriesCombo.getText(),                  //
                m_OperationsCombo.getText(),                  //
                m_SourceName,                                 //
                m_IncomeDateInputText.GetText(),         //
                m_IncomeDescriptionInputText.GetText(),  //
                m_IncomeCommentInputText.GetText(),      //
                m_IncomeAmountInputDouble,               //
                false,                                        //
                hash);
            DataBase::Instance()->CloseDBFile();
        }
    }*/
}

void IncomeDialog::m_confirmDialogUpdateAll() {
    /*RowID account_id = 0U;
    if (DataBase::Instance()->GetAccount(m_AccountsCombo.getText(), account_id)) {
        if (DataBase::Instance()->OpenDBFile()) {
            if (DataBase::Instance()->BeginIncome()) {
                for (auto t : m_IncomesToUpdate) {
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
                    if (m_IncomeDateInputText.GetText() != MULTIPLE_VALUES) {
                        t.date = m_IncomeDateInputText.GetText();
                    }
                    if (m_IncomeDescriptionInputText.GetText() != MULTIPLE_VALUES) {
                        t.description = m_IncomeDescriptionInputText.GetText();
                    }
                    if (m_IncomeCommentInputText.GetText() != MULTIPLE_VALUES) {
                        t.comment = m_IncomeCommentInputText.GetText();
                    }
                    if (!m_IncomeConfirmedManyValues) {
                        t.confirmed = m_IncomeConfirmed;
                    }
                    DataBase::Instance()->UpdateIncome(  //
                        t.id,                                 //
                        t.entity,                             //
                        t.operation,                          //
                        t.category,                           //
                        t.source,                             //
                        t.date,                               //
                        t.description,                        //
                        t.comment,                            //
                        t.amount,                             //
                        t.confirmed,                          //
                        t.hash);
                }
                DataBase::Instance()->CommitIncome();
            }
            DataBase::Instance()->CloseDBFile();
        }
    }*/
}

void IncomeDialog::m_confirmDialogDeletion() {
    /*std::set<RowID> m_rows;
    for (const auto& t : m_IncomesToDelete) {
        m_rows.emplace(t.id);
    }
    if (!m_rows.empty()) {
        DataBase::Instance()->DeleteIncomes(m_rows);
    }*/
}

void IncomeDialog::m_UpdateAccounts() {
    m_AccountsCombo.clear();
    DataBase::Instance()->GetAccounts(  //
        [this](
            const RowID& vRowID,
            const BankName& vBankName,
            const BankAgency& vBankAgency,
            const AccountType& vAccountType,
            const AccountName& vAccountName,
            const AccountNumber& vAccountNumber,
            const AccounBaseSolde& vAccounBaseSolde,
            const TransactionsCount& vTransactionsCount) {  //
            m_AccountsCombo.getArrayRef().push_back(vAccountNumber);
        });
    m_AccountsCombo.getIndexRef() = 0;
}

void IncomeDialog::m_UpdateEntities() {
    m_EntitiesCombo.clear();
    DataBase::Instance()->GetEntities(           //
        [this](const EntityName& vEntityName) {  //
            m_EntitiesCombo.getArrayRef().push_back(vEntityName);
        });
    m_EntitiesCombo.getIndexRef() = 0;
    m_EntitiesCombo.finalize();
}

void IncomeDialog::m_UpdateOperations() {
    m_OperationsCombo.clear();
    DataBase::Instance()->GetOperations(               //
        [this](const OperationName& vOperationName) {  //
            m_OperationsCombo.getArrayRef().push_back(vOperationName);
        });
    m_OperationsCombo.getIndexRef() = 0;
    m_OperationsCombo.finalize();
}

void IncomeDialog::m_UpdateCategories() {
    m_CategoriesCombo.clear();
    DataBase::Instance()->GetCategories(             //
        [this](const CategoryName& vCategoryName) {  //
            m_CategoriesCombo.getArrayRef().push_back(vCategoryName);
        });
    m_CategoriesCombo.getIndexRef() = 0;
    m_CategoriesCombo.finalize();
}
