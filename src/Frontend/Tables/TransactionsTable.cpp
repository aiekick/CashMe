#include <Frontend/Tables/TransactionsTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>

TransactionsTable::TransactionsTable() : ADataBarsTable("TransactionsTable", 11) {
}

bool TransactionsTable::init() {
    bool ret = true;
    ret &= m_TransactionDialog.init();
    ret &= m_IncomeDialog.init();
    return ret;
}

void TransactionsTable::unit() {
    m_IncomeDialog.unit();
    m_TransactionDialog.unit();
    clear();
}

bool TransactionsTable::load() {
    if (ADataBarsTable::load()) {
        refreshDatas();
        return true;
    }
    return false;
}

void TransactionsTable::unload() {
    ADataBarsTable::unload();
    clear();
}

bool TransactionsTable::drawMenu() {
    bool ret = false;
    ret |= ImGui::MenuItem("Multiline comments", nullptr, &m_enableMultilineComment);
    return ret;
}

TransactionDialog& TransactionsTable::getTransactionDialogRef() {
    return m_TransactionDialog;
}

IncomeDialog& TransactionsTable::getIncomeDialogRef() {
    return m_IncomeDialog;
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
    return m_Datas.transactions_filtered.at(vIdx).solde;
}

void TransactionsTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_Datas.transactions_filtered.at(vIdx);

    ImGui::TableNextColumn();
    {
        if (m_IsGroupingModeTransactions()) {
            ImGui::PushID(t.id);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            if (ImGui::Checkbox("##check", &t.confirmed)) {
                DataBase::Instance()->ConfirmTransaction(t.id, t.confirmed);
            }
            ImGui::PopStyleVar();
            ImGui::PopID();
        }
    }

    ImGui::TableNextColumn();
    { ImGui::Text("%s", t.date.c_str()); }

    m_drawColumnSelectable(vIdx, t.id, t.description);

    ImGui::TableNextColumn();
    {
        if (m_enableMultilineComment || t.comment_first_line_end_pos == 0) {
            ImGui::Text(t.comment.c_str());
        } else {
            ImGui::TextUnformatted(t.comment.c_str(), t.comment.data() + t.comment_first_line_end_pos);
        }
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.comment.c_str());
    }

    ImGui::TableNextColumn();
    {
        ImGui::Text("%s", t.entity.c_str());
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.entity.c_str());
    }

    ImGui::TableNextColumn();
    {
        ImGui::Text("%s", t.category.c_str());
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.category.c_str());
    }

    ImGui::TableNextColumn();
    {
        ImGui::Text("%s", t.operation.c_str());
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%s", t.operation.c_str());
    }

    m_drawColumnDebit(t.debit);

    m_drawColumnCredit(t.credit);

    m_drawColumnAmount(t.solde);

    m_drawColumnBars(t.solde, vMaxAmount, 100.0f);
}

void TransactionsTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 2);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Comments", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("EntityOutput", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    m_drawSearchRow();
    ImGui::TableHeadersRow();
}

void TransactionsTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update selection")) {
            std::vector<Transaction> transactions_to_update;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_IsRowSelected(trans.id)) {
                    transactions_to_update.push_back(trans);
                }
            }
            if (transactions_to_update.size() > 1U) {
                m_TransactionDialog.setTransactionsToUpdate(transactions_to_update);
                m_TransactionDialog.show(DataDialogMode::MODE_UPDATE_ALL);
            } else if (transactions_to_update.size() == 1U) {
                m_TransactionDialog.setTransaction(transactions_to_update.front());
                m_TransactionDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
            }
        }
        if (ImGui::MenuItem("Delete selection")) {
            std::vector<Transaction> transactions_to_delete;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_IsRowSelected(trans.id)) {
                    transactions_to_delete.push_back(trans);
                }
            }
            m_TransactionDialog.setTransactionsToDelete(transactions_to_delete);
            m_TransactionDialog.show(DataDialogMode::MODE_DELETE_ALL);
        }
        if (ImGui::MenuItem("Add as income")) {
            std::vector<Transaction> transactions_to_add_as_incomes;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_IsRowSelected(trans.id)) {
                    transactions_to_add_as_incomes.push_back(trans);
                }
            }
            m_IncomeDialog.setTransactions(transactions_to_add_as_incomes);
            m_IncomeDialog.show(DataDialogMode::MODE_CREATION);
        }
    }
}

void TransactionsTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    m_TransactionDialog.setTransaction(m_Datas.transactions_filtered.at(vIdx));
    m_TransactionDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
}

void TransactionsTable::refreshDatas() {
    m_UpdateBanks();
    m_UpdateEntities();
    m_UpdateCategories();
    m_UpdateOperations();
    m_UpdateAccounts();
}

bool TransactionsTable::m_isGroupingModeTransactions() {
    return (m_GroupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS);
}

void TransactionsTable::m_drawSearchRow() {
    bool change = false;
    bool reset = false;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (m_IsGroupingModeTransactions()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::ContrastedButton("R", nullptr, nullptr, ImGui::GetColumnWidth(0))) {
            reset = true;
        }
        ImGui::PopStyleVar();
    }
    for (size_t idx = 0; idx < 10; ++idx) {
        ImGui::TableNextColumn();
        if (idx < SearchColumns::SEARCH_COLUMN_Count) {
            if (m_IsGroupingModeTransactions()) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (m_SearchInputTexts.at(idx).DisplayInputText(ImGui::GetColumnWidth(idx), "", "")) {
                    m_SearchTokens[idx] = ez::str::toLower(m_SearchInputTexts.at(idx).GetText());
                    m_FilteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
                    change = true;
                }
                ImGui::PopStyleVar();
            }
        } else if (idx == 6) {
            m_drawAmount(m_TotalDebit);
        } else if (idx == 7) {
            m_drawAmount(m_TotalCredit);
        } else if (idx == 8) {
            // m_drawAmount(m_CurrentBaseSolde);
        } else if (idx == 9) {
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

void TransactionsTable::drawSelectMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Select")) {
        if (ImGui::BeginMenu("Rows")) {
            if (ImGui::MenuItem("Displayed")) {
                m_selectRows(0, m_getItemsCount());
            }
            if (ImGui::MenuItem("UnConfirmed")) {
                m_SelectUnConfirmedTransactions();
            }
            if (ImGui::MenuItem("Duplicate (Date + Amount)")) {
                m_SelectPossibleDuplicateEntryOnPricesAndDates();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Empty")) {
            if (ImGui::MenuItem("Comments")) {
                m_SelectEmptyColumn(SearchColumns::SEARCH_COLUMN_COMMENT);
            }
            if (ImGui::MenuItem("Entities")) {
                m_SelectEmptyColumn(SearchColumns::SEARCH_COLUMN_ENTITY);
            }
            if (ImGui::MenuItem("Categories")) {
                m_SelectEmptyColumn(SearchColumns::SEARCH_COLUMN_CATEGORY);
            }
            if (ImGui::MenuItem("Operations")) {
                m_SelectEmptyColumn(SearchColumns::SEARCH_COLUMN_OPERATION);
            }
            ImGui::EndMenu();
        }
        if (!m_getSelectedRows().empty()) {
            if (ImGui::BeginMenu("Selection Actions")) {
                if (ImGui::MenuItem("Do filter")) {
                    m_FilterSelection();
                }
                if (ImGui::MenuItem("Do reset")) {
                    m_ResetSelection();
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenu();
    }
}

void TransactionsTable::drawDebugMenu(FrameActionSystem& vFrameActionSystem) {
#ifdef _DEBUG
    if (ImGui::BeginMenu("Debug")) {
        if (ImGui::MenuItem("Refresh")) {
            refreshDatas();
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Delete Tables")) {
            if (ImGui::MenuItem("Banks")) {
                DataBase::Instance()->DeleteBanks();
                refreshDatas();
            }
            if (ImGui::MenuItem("Accounts")) {
                DataBase::Instance()->DeleteAccounts();
                refreshDatas();
            }
            if (ImGui::MenuItem("Entities")) {
                DataBase::Instance()->DeleteEntities();
                refreshDatas();
            }
            if (ImGui::MenuItem("Categories")) {
                DataBase::Instance()->DeleteCategories();
                refreshDatas();
            }
            if (ImGui::MenuItem("Operations")) {
                DataBase::Instance()->DeleteOperations();
                refreshDatas();
            }
            if (ImGui::MenuItem("Incomes")) {
                DataBase::Instance()->DeleteIncomes();
                refreshDatas();
            }
            if (ImGui::MenuItem("Transactions")) {
                DataBase::Instance()->DeleteTransactions();
                refreshDatas();
            }
            if (ImGui::MenuItem("Sources")) {
                DataBase::Instance()->DeleteSources();
                refreshDatas();
            }
            if (ImGui::MenuItem("All Except Accounts and banks")) {
                DataBase::Instance()->DeleteEntities();
                DataBase::Instance()->DeleteCategories();
                DataBase::Instance()->DeleteOperations();
                DataBase::Instance()->DeleteIncomes();
                DataBase::Instance()->DeleteTransactions();
                DataBase::Instance()->DeleteSources();
                refreshDatas();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
#endif
}

void TransactionsTable::drawGroupingMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::MenuItem("T", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS)) {
        m_GroupTransactions(GroupingMode::GROUPING_MODE_TRANSACTIONS);
    }
    if (ImGui::MenuItem("D", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_DAYS)) {
        m_GroupTransactions(GroupingMode::GROUPING_MODE_DAYS);
    }
    if (ImGui::MenuItem("M", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_MONTHS)) {
        m_GroupTransactions(GroupingMode::GROUPING_MODE_MONTHS);
    }
    if (ImGui::MenuItem("Y", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_YEARS)) {
        m_GroupTransactions(GroupingMode::GROUPING_MODE_YEARS);
    }
}

void TransactionsTable::drawAccountsMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Accounts")) {
        for (const auto& bank : m_Accounts) {
            if (ImGui::BeginMenu(bank.first.c_str())) {  // bank name
                for (const auto& agency : bank.second) {
                    if (ImGui::BeginMenu(agency.first.c_str())) {  // bank agency
                        static auto flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
                        if (ImGui::BeginTable("##MenuAccounts", 4, flags)) {
                            ImGui::TableSetupScrollFreeze(0, 1);
                            ImGui::TableSetupColumn("Number", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed);
                            ImGui::TableHeadersRow();
                            size_t idx = 0U;
                            for (const auto& number : agency.second) {
                                const auto& a = number.second;
                                ImGui::TableNextRow();

                                ImGui::PushID(a.id);
                                {
                                    ImGui::TableNextColumn();
                                    ImGui::PushID(&a);
                                    {
                                        if (ImGui::Selectable(a.datas.number.c_str(), m_getAccountComboRef().getIndex() == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                                            m_ResetSelection();
                                            m_UpdateTransactions(a.id);
                                            m_getAccountComboRef().getIndexRef() = idx;
                                        }
                                    }
                                    ImGui::PopID();

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%s", a.datas.name.c_str());

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%s", a.datas.type.c_str());

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%u", a.count);
                                }
                                ImGui::PopID();

                                ++idx;
                            }
                            ImGui::EndTable();
                        }
                        ImGui::EndMenu();
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenu();
    }
}

void TransactionsTable::clear() {
    m_Datas.clear();
}

void TransactionsTable::resetFiltering() {
    m_Datas.transactions_filtered = m_Datas.transactions;
    m_Datas.transactions_filtered_rowids = {};
    m_SearchInputTexts = {};
    m_SearchTokens = {};
    m_FilteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
    m_Datas.filtered_selected_transactions.clear();
    refreshFiltering();
}

void TransactionsTable::refreshFiltering() {
    m_Datas.transactions_filtered.clear();
    m_Datas.transactions_filtered_rowids.clear();
    bool use = false;
    double solde = m_CurrentBaseSolde;
    m_TotalDebit = 0.0;
    m_TotalCredit = 0.0;
    for (auto t : m_Datas.transactions) { // copy volontaire
        use = true;
        if (m_FilteringMode == FilteringMode::FILTERING_MODE_BY_SEARCH) {
            for (size_t idx = 0; idx < SearchColumns::SEARCH_COLUMN_Count; ++idx) {
                const auto& tk = m_SearchTokens.at(idx);
                if (!tk.empty()) {
                    use &= (t.optimized.at(idx).find(tk) != std::string::npos);
                }
            }
        } else if (m_FilteringMode == FilteringMode::FILTERING_MODE_BY_SELECTED_ROW_IDS) {
            use = (m_Datas.filtered_selected_transactions.find(t.id) != m_Datas.filtered_selected_transactions.end());
        }
        if (use) {
            t.solde = solde += t.debit + t.credit;
            const auto first_comment_end_line_pos = t.comment.find('\n');
            if (first_comment_end_line_pos != std::string::npos) {
                t.comment_first_line_end_pos = first_comment_end_line_pos;
            }
            m_Datas.transactions_filtered.push_back(t);
            m_Datas.transactions_filtered_rowids.emplace(t.id);
            m_TotalDebit += t.debit;
            m_TotalCredit += t.credit;
        }
    }
    // reverse datas
    std::reverse(m_Datas.transactions_filtered.begin(), m_Datas.transactions_filtered.end());
}

void TransactionsTable::m_FilterSelection() {
    m_FilteringMode = FilteringMode::FILTERING_MODE_BY_SELECTED_ROW_IDS;
    m_Datas.filtered_selected_transactions = m_getSelectedRows();
    refreshFiltering();
}

void TransactionsTable::m_SelectPossibleDuplicateEntryOnPricesAndDates() {
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        RowID account_id = m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id;
        m_ResetSelection();
        DataBase::Instance()->GetDuplicateTransactionsOnDatesAndAmount(  //
            account_id,                                                  //
            [this](const RowID& vRowID) {
                if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                    m_Datas.transactions_filtered_rowids.end()) {
                    m_SelectRow(vRowID);  // select row id
                }
            });
    }
}

void TransactionsTable::m_SelectUnConfirmedTransactions() {
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        RowID account_id = m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id;
        m_ResetSelection();
        DataBase::Instance()->GetUnConfirmedTransactions(  //
            account_id,                                    //
            [this](const RowID& vRowID) {
                if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                    m_Datas.transactions_filtered_rowids.end()) {
                    m_SelectRow(vRowID);  // select row id
                }
            });
    }
}

void TransactionsTable::m_SelectEmptyColumn(const SearchColumns& vColumn) {
    m_ResetSelection();
    for (const auto& t : m_Datas.transactions_filtered) {
        if (vColumn == SearchColumns::SEARCH_COLUMN_COMMENT) {
            if (t.comment.empty()) {
                m_SelectRow(t.id);
            }
        } else if (vColumn == SearchColumns::SEARCH_COLUMN_ENTITY) {
            if (t.entity.empty()) {
                m_SelectRow(t.id);
            }
        } else if (vColumn == SearchColumns::SEARCH_COLUMN_CATEGORY) {
            if (t.category.empty()) {
                m_SelectRow(t.id);
            }
        } else if (vColumn == SearchColumns::SEARCH_COLUMN_OPERATION) {
            if (t.operation.empty()) {
                m_SelectRow(t.id);
            }
        }
    }
}

void TransactionsTable::m_GroupTransactions(const GroupingMode& vGroupingMode) {
    m_GroupingMode = vGroupingMode;
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        m_UpdateTransactions(m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id);
    }
}

void TransactionsTable::m_UpdateBanks() {
    m_Datas.bankNames.clear();
    DataBase::Instance()->GetBanks(                                       //
        [this](const BankOutput& vBankOutput) {  //
            m_Datas.bankNames.push_back(vBankOutput.datas.name);
        });
}

void TransactionsTable::m_UpdateEntities() {
    m_Datas.entityNames.clear();
    DataBase::Instance()->GetEntities(           //
        [this](const EntityOutput& vEntityOutput) {  //
            m_Datas.entityNames.push_back(vEntityOutput.datas.name);
        });
}

void TransactionsTable::m_UpdateCategories() {
    m_Datas.categoryNames.clear();
    DataBase::Instance()->GetCategories(             //
        [this](const CategoryName& vCategoryName) {  //
            m_Datas.categoryNames.push_back(vCategoryName);
        });
}

void TransactionsTable::m_UpdateOperations() {
    m_Datas.operationNames.clear();
    DataBase::Instance()->GetOperations(               //
        [this](const OperationName& vOperationName) {  //
            m_Datas.operationNames.push_back(vOperationName);
        });
}

void TransactionsTable::m_UpdateAccounts() {
    m_Accounts.clear();
    m_Datas.accounts.clear();
    m_Datas.accountNumbers.clear();
    DataBase::Instance()->GetAccounts(                 //
        [this](const AccountOutput& vAccountOutput) {  //
            m_Datas.accounts.push_back(vAccountOutput);
            m_Datas.accountNumbers.push_back(vAccountOutput.datas.number);
            m_Accounts[vAccountOutput.bankName + "##BankName"][vAccountOutput.datas.bank_agency + "##BankAgency"][vAccountOutput.datas.number] = vAccountOutput;
        });
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        m_UpdateTransactions(m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id);
    }
}

void TransactionsTable::m_UpdateTransactions(const RowID& vAccountID) {
    m_Datas.transactions.clear();
    const auto& zero_based_account_id = vAccountID - 1;
    if (zero_based_account_id < m_Datas.accounts.size()) {
        double solde = m_CurrentBaseSolde = m_Datas.accounts.at(zero_based_account_id).datas.base_solde;
        const auto& account_number = m_Datas.accounts.at(zero_based_account_id).datas.number;
        if (m_IsGroupingModeTransactions()) {
            DataBase::Instance()->GetTransactions(  //
                vAccountID,                         //
                [this, &solde, account_number](     //
                    const RowID& vTransactionID,
                    const EntityName& vEntityName,
                    const CategoryName& vCategoryName,
                    const OperationName& vOperationName,
                    const SourceName& vSourceName,
                    const TransactionDate& vDate,
                    const TransactionDateEpoch& vDateEpoch,
                    const TransactionDescription& vDescription,
                    const TransactionComment& vComment,
                    const TransactionAmount& vAmount,
                    const TransactionConfirmed& vConfirmed,
                    const TransactionSha& vSha) {  //
                    solde += vAmount;
                    Transaction t;
                    t.id = vTransactionID;
                    t.account = account_number;
                    t.optimized[0] = ez::str::toLower(t.date = vDate);
                    t.optimized[1] = ez::str::toLower(t.description = vDescription);
                    t.optimized[2] = ez::str::toLower(t.comment = vComment);
                    t.optimized[3] = ez::str::toLower(t.entity = vEntityName);
                    t.optimized[4] = ez::str::toLower(t.category = vCategoryName);
                    t.optimized[5] = ez::str::toLower(t.operation = vOperationName);
                    t.epoch = vDateEpoch;
                    t.sha = vSha;
                    t.source = vSourceName;
                    t.debit = vAmount < 0.0 ? vAmount : 0.0;
                    t.credit = vAmount > 0.0 ? vAmount : 0.0;
                    t.amount = vAmount;
                    t.confirmed = vConfirmed;
                    t.solde = solde;
                    m_Datas.transactions.push_back(t);
                });
        } else {
            DataBase::Instance()->GetGroupedTransactions(  //
                vAccountID,
                GroupBy::DATES,
                (DateFormat)(m_GroupingMode - 1),
                [this](  //
                    const RowID& vRowID,
                    const TransactionDate& vTransactionDate,
                    const TransactionDescription& vTransactionDescription,
                    const EntityName& vEntityName,
                    const CategoryName& vCategoryName,
                    const OperationName& vOperationName,
                    const TransactionDebit& vTransactionDebit,
                    const TransactionCredit& vTransactionCredit) {
                    Transaction t;
                    t.id = vRowID;
                    t.date = vTransactionDate;
                    t.description = "-- grouped --";
                    t.entity = "-- grouped --";
                    t.category = "-- grouped --";
                    t.operation = "-- grouped --";
                    t.debit = vTransactionDebit;
                    t.credit = vTransactionCredit;
                    t.amount = vTransactionDebit + vTransactionCredit;
                    m_Datas.transactions.push_back(t);
                });
        }
    }
    // filtering
    refreshFiltering();
}

bool TransactionsTable::m_IsGroupingModeTransactions() {
    return (m_GroupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS);
}

void TransactionsTable::m_drawAmount(const double& vAmount) {
    if (vAmount < 0.0) {
        const auto& bad_color = ImGui::GetColorU32(ImGui::CustomStyle::BadColor);
        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        const auto& good_color = ImGui::GetColorU32(ImGui::CustomStyle::GoodColor);
        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
    }
}