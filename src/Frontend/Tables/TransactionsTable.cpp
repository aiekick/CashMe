#include <Frontend/Tables/TransactionsTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>
#include <Frontend/MainFrontend.h>

TransactionsTable::TransactionsTable() : ADataBarsTable("TransactionsTable", 12) {}

void TransactionsTable::refreshDatas() {
    m_updateBanks();
    m_updateAccounts();
    m_updateEntities();
    m_updateCategories();
    m_updateOperations();
    m_updateTransactions();
}

bool TransactionsTable::m_drawMenu() {
    bool ret = false;
    ret |= m_drawSelectMenu();
    ret |= m_drawGroupingMenu();
    ret |= m_drawDebugMenu();
    ret |= ImGui::MenuItem("Multiline comments", nullptr, &m_enableMultilineComment);
    if (ImGui::MenuItem("Refresh")) {
        refreshDatas();
        ret = true;
    }
    return ret;
}

size_t TransactionsTable::m_getItemsCount() const {
    return m_Datas.transactions_filtered.size();
}

RowID TransactionsTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Datas.transactions_filtered.size()) {
        return m_Datas.transactions_filtered.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double TransactionsTable::m_getItemBarAmount(const size_t& vIdx) const {
    return m_Datas.transactions_filtered.at(vIdx).amounts.amount;
}

void TransactionsTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 2);

    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Comments", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);

    m_drawSearchRow();

    ImGui::TableHeadersRow();
}

void TransactionsTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_Datas.transactions_filtered.at(vIdx);
    if (m_drawColumnCheckbox(t.datas.confirmed, m_isGroupingModeTransactions())) {
        DataBase::ref().ConfirmTransaction(t.id, t.datas.confirmed);
    }
    m_drawColumnText(t.datas.date);
    m_drawColumnSelectable(vIdx, t.id, t.datas.description);
    ImGui::TableNextColumn();
    {
        if (m_enableMultilineComment || t.comment_first_line_end_pos == 0) {
            ImGui::Text(t.datas.comment.c_str());
        } else {
            ImGui::TextUnformatted(t.datas.comment.c_str(), t.datas.comment.data() + t.comment_first_line_end_pos);
        }
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::ref().isHiddenMode(), "%s", t.datas.comment.c_str());
    }
    m_drawColumnText(t.datas.entity.name);
    m_drawColumnText(t.datas.category.name);
    m_drawColumnText(t.datas.operation.name);
    m_drawColumnText(t.datas.income.name);
    m_drawColumnDebit(t.amounts.debit);
    m_drawColumnCredit(t.amounts.credit);
    m_drawColumnAmount(t.amounts.amount);
    m_drawColumnBars(t.amounts.amount, vMaxAmount, 100.0f);
}

void TransactionsTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update selection")) {
            std::vector<TransactionOutput> transactions_to_update;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_isRowSelected(trans.id)) {
                    transactions_to_update.push_back(trans);
                }
            }
            if (transactions_to_update.size() > 1U) {
                MainFrontend::ref().getTransactionDialogRef().setTransactionsToUpdate(transactions_to_update);
                MainFrontend::ref().getTransactionDialogRef().show(DataDialogMode::MODE_UPDATE_ALL);
            } else if (transactions_to_update.size() == 1U) {
                MainFrontend::ref().getTransactionDialogRef().setTransaction(transactions_to_update.front());
                MainFrontend::ref().getTransactionDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
            }
        }
        if (ImGui::MenuItem("Delete selection")) {
            std::vector<TransactionOutput> transactions_to_delete;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_isRowSelected(trans.id)) {
                    transactions_to_delete.push_back(trans);
                }
            }
            MainFrontend::ref().getTransactionDialogRef().setTransactionsToDelete(transactions_to_delete);
            MainFrontend::ref().getTransactionDialogRef().show(DataDialogMode::MODE_DELETE_ALL);
        }
        if (ImGui::MenuItem("Add as income")) {
            std::vector<TransactionOutput> transactions_to_add_as_incomes;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_isRowSelected(trans.id)) {
                    transactions_to_add_as_incomes.push_back(trans);
                }
            }
            MainFrontend::ref().getIncomeDialogRef().setTransactions(transactions_to_add_as_incomes);
            MainFrontend::ref().getIncomeDialogRef().show(DataDialogMode::MODE_CREATION);
        }
    }
}

void TransactionsTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    MainFrontend::ref().getTransactionDialogRef().setTransaction(m_Datas.transactions_filtered.at(vIdx));
    MainFrontend::ref().getTransactionDialogRef().show(DataDialogMode::MODE_UPDATE_ONCE);
}

bool TransactionsTable::m_isGroupingModeTransactions() {
    return (m_groupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS);
}

void TransactionsTable::m_drawSearchRow() {
    bool change = false;
    bool reset = false;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (m_isGroupingModeTransactions()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::ContrastedButton("R", nullptr, nullptr, ImGui::GetColumnWidth(0))) {
            reset = true;
        }
        ImGui::PopStyleVar();
    }
    for (size_t idx = 0; idx < m_getColumnCount(); ++idx) {
        ImGui::TableNextColumn();
        if (idx < SearchColumns::SEARCH_COLUMN_Count) {
            if (m_isGroupingModeTransactions()) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (m_SearchInputTexts.at(idx).DisplayInputText(ImGui::GetColumnWidth(idx), "", "")) {
                    m_SearchTokens[idx] = ez::str::toLower(m_SearchInputTexts.at(idx).GetText());
                    m_filteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
                    change = true;
                }
                ImGui::PopStyleVar();
            }
        } else if (idx == 7) {
            m_drawAmount(m_TotalDebit);
        } else if (idx == 8) {
            m_drawAmount(m_TotalCredit);
        } else if (idx == 9) {
            // m_drawAmount(m_CurrentBaseSolde);
        } else if (idx == 10) {
            ImGui::Text(  //
                "[%u/%u]",
                (uint32_t)m_Datas.transactions_filtered.size(),
                (uint32_t)m_Datas.transactions.size());
        }
    }
    if (reset) {
        resetFiltering();
    }
    if (change) {
        refreshFiltering();
    }
}

bool TransactionsTable::m_drawSelectMenu() {
    if (ImGui::BeginMenu("Select")) {
        if (ImGui::BeginMenu("Rows")) {
            if (ImGui::MenuItem("Displayed")) {
                m_selectRows(0, m_getItemsCount());
            }
            if (ImGui::MenuItem("UnConfirmed")) {
                m_selectUnConfirmedTransactions();
            }
            if (ImGui::MenuItem("Duplicate (Date + Amount)")) {
                m_selectPossibleDuplicateEntryOnPricesAndDates();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Empty")) {
            if (ImGui::MenuItem("Comments")) {
                m_selectEmptyColumn(SearchColumns::SEARCH_COLUMN_COMMENT);
            }
            if (ImGui::MenuItem("Entities")) {
                m_selectEmptyColumn(SearchColumns::SEARCH_COLUMN_ENTITY);
            }
            if (ImGui::MenuItem("Categories")) {
                m_selectEmptyColumn(SearchColumns::SEARCH_COLUMN_CATEGORY);
            }
            if (ImGui::MenuItem("Operations")) {
                m_selectEmptyColumn(SearchColumns::SEARCH_COLUMN_OPERATION);
            }
            ImGui::EndMenu();
        }
        if (!m_getSelectedRows().empty()) {
            if (ImGui::BeginMenu("Selection Actions")) {
                if (ImGui::MenuItem("Do filter")) {
                    m_filterSelection();
                }
                if (ImGui::MenuItem("Do reset")) {
                    m_ResetSelection();
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenu();
    }
    return false;
}

bool TransactionsTable::m_drawGroupingMenu() {
    if (ImGui::MenuItem("T", nullptr, m_groupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS)) {
        m_groupTransactions(GroupingMode::GROUPING_MODE_TRANSACTIONS);
    }
    if (ImGui::MenuItem("D", nullptr, m_groupingMode == GroupingMode::GROUPING_MODE_DAYS)) {
        m_groupTransactions(GroupingMode::GROUPING_MODE_DAYS);
    }
    if (ImGui::MenuItem("M", nullptr, m_groupingMode == GroupingMode::GROUPING_MODE_MONTHS)) {
        m_groupTransactions(GroupingMode::GROUPING_MODE_MONTHS);
    }
    if (ImGui::MenuItem("Y", nullptr, m_groupingMode == GroupingMode::GROUPING_MODE_YEARS)) {
        m_groupTransactions(GroupingMode::GROUPING_MODE_YEARS);
    }
    return false;
}

void TransactionsTable::clear() {
    m_Datas.clear();
}

void TransactionsTable::resetFiltering() {
    m_Datas.transactions_filtered = m_Datas.transactions;
    m_Datas.transactions_filtered_rowids = {};
    m_SearchInputTexts = {};
    m_SearchTokens = {};
    m_filteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
    m_Datas.filtered_selected_transactions.clear();
    refreshFiltering();
}

void TransactionsTable::refreshFiltering() {
    m_Datas.transactions_filtered.clear();
    m_Datas.transactions_filtered_rowids.clear();
    bool use = false;
    double solde = m_getAccount().datas.base_solde;
    m_TotalDebit = 0.0;
    m_TotalCredit = 0.0;
    for (auto t : m_Datas.transactions) {  // copy volontaire
        use = true;
        if (m_filteringMode == FilteringMode::FILTERING_MODE_BY_SEARCH) {
            for (size_t idx = 0; idx < SearchColumns::SEARCH_COLUMN_Count; ++idx) {
                const auto& tk = m_SearchTokens.at(idx);
                if (!tk.empty()) {
                    use &= (t.optimized.at(idx).find(tk) != std::string::npos);
                }
            }
        } else if (m_filteringMode == FilteringMode::FILTERING_MODE_BY_SELECTED_ROW_IDS) {
            use = (m_Datas.filtered_selected_transactions.find(t.id) != m_Datas.filtered_selected_transactions.end());
        }
        if (use) {
            t.amounts.amount = solde += t.amounts.debit + t.amounts.credit;
            const auto first_comment_end_line_pos = t.datas.comment.find('\n');
            if (first_comment_end_line_pos != std::string::npos) {
                t.comment_first_line_end_pos = first_comment_end_line_pos;
            }
            m_Datas.transactions_filtered.push_back(t);
            m_Datas.transactions_filtered_rowids.emplace(t.id);
            m_TotalDebit += t.amounts.debit;
            m_TotalCredit += t.amounts.credit;
        }
    }
    // reverse datas
    std::reverse(m_Datas.transactions_filtered.begin(), m_Datas.transactions_filtered.end());
}

void TransactionsTable::m_filterSelection() {
    m_filteringMode = FilteringMode::FILTERING_MODE_BY_SELECTED_ROW_IDS;
    m_Datas.filtered_selected_transactions = m_getSelectedRows();
    refreshFiltering();
}

void TransactionsTable::m_selectPossibleDuplicateEntryOnPricesAndDates() {
    m_ResetSelection();
    DataBase::ref().GetDuplicateTransactionsOnDatesAndAmount(  //
        m_getAccountID(),                                      //
        [this](const RowID& vRowID) {
            if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                m_Datas.transactions_filtered_rowids.end()) {
                m_selectRow(vRowID);  // select row id
            }
        });
}

void TransactionsTable::m_selectUnConfirmedTransactions() {
    m_ResetSelection();
    DataBase::ref().GetUnConfirmedTransactions(  //
        m_getAccountID(),                        //
        [this](const RowID& vRowID) {
            if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                m_Datas.transactions_filtered_rowids.end()) {
                m_selectRow(vRowID);  // select row id
            }
        });
}

void TransactionsTable::m_selectEmptyColumn(const SearchColumns& vColumn) {
    m_ResetSelection();
    for (const auto& t : m_Datas.transactions_filtered) {
        if (vColumn == SearchColumns::SEARCH_COLUMN_COMMENT) {
            if (t.datas.comment.empty()) {
                m_selectRow(t.id);
            }
        } else if (vColumn == SearchColumns::SEARCH_COLUMN_ENTITY) {
            if (t.datas.entity.name.empty()) {
                m_selectRow(t.id);
            }
        } else if (vColumn == SearchColumns::SEARCH_COLUMN_CATEGORY) {
            if (t.datas.category.name.empty()) {
                m_selectRow(t.id);
            }
        } else if (vColumn == SearchColumns::SEARCH_COLUMN_OPERATION) {
            if (t.datas.operation.name.empty()) {
                m_selectRow(t.id);
            }
        }
    }
}

void TransactionsTable::m_groupTransactions(const GroupingMode& vGroupingMode) {
    m_groupingMode = vGroupingMode;
    m_updateTransactions();
}

void TransactionsTable::m_updateBanks() {
    m_Datas.bankNames.clear();
    DataBase::ref().GetBanks(                    //
        [this](const BankOutput& vBankOutput) {  //
            m_Datas.bankNames.push_back(vBankOutput.datas.name);
        });
}

void TransactionsTable::m_updateEntities() {
    m_Datas.entityNames.clear();
    DataBase::ref().GetEntities(                     //
        [this](const EntityOutput& vEntityOutput) {  //
            m_Datas.entityNames.push_back(vEntityOutput.datas.name);
        });
}

void TransactionsTable::m_updateCategories() {
    m_Datas.categoryNames.clear();
    DataBase::ref().GetCategories(                       //
        [this](const CategoryOutput& vCategoryOutput) {  //
            m_Datas.categoryNames.push_back(vCategoryOutput.datas.name);
        });
}

void TransactionsTable::m_updateOperations() {
    m_Datas.operationNames.clear();
    DataBase::ref().GetOperations(                         //
        [this](const OperationOutput& vOperationOutput) {  //
            m_Datas.operationNames.push_back(vOperationOutput.datas.name);
        });
}

void TransactionsTable::m_updateTransactions() {
    m_Datas.transactions.clear();
    double solde = m_getAccount().datas.base_solde;
    const auto& account_number = m_getAccount().datas.number;
    if (m_isGroupingModeTransactions()) {
        DataBase::ref().GetTransactions(     //
            m_getAccountID(),                //
            [this, &solde, account_number](  //
                const TransactionOutput& vTransactionOutput) {  //
                auto to = vTransactionOutput;
                to.amounts.amount += solde;
                to.optimized[0] = ez::str::toLower(to.datas.date);
                to.optimized[1] = ez::str::toLower(to.datas.description );
                to.optimized[2] = ez::str::toLower(to.datas.comment );
                to.optimized[3] = ez::str::toLower(to.datas.entity.name );
                to.optimized[4] = ez::str::toLower(to.datas.category.name);
                to.optimized[5] = ez::str::toLower(to.datas.operation.name);
                to.amounts.debit = to.datas.amount < 0.0 ? to.datas.amount : 0.0;
                to.amounts.credit = to.datas.amount > 0.0 ? to.datas.amount : 0.0;
                to.amounts.amount = to.datas.amount;
                m_Datas.transactions.push_back(to);
            });
    } else {
        DataBase::ref().GetGroupedTransactions(  //
            m_getAccountID(),
            GroupBy::DATES,
            (DateFormat)(m_groupingMode - 1),
            [this](  //
                const TransactionOutput& vTransactionOutput) {
                auto to = vTransactionOutput;
                to.datas.description = "-- grouped --";
                to.datas.entity.name = "-- grouped --";
                to.datas.category.name = "-- grouped --";
                to.datas.operation.name = "-- grouped --";
                m_Datas.transactions.push_back(to);
            });
    }

    // filtering
    refreshFiltering();
}

