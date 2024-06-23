/*#include <Frontend/Tables/BudgetTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>

BudgetTable::BudgetTable() : ADataTable("BudgetTable", 11) {
}

BudgetTable::~BudgetTable() {
}

bool BudgetTable::Init() {
    return m_BudgetDialog.init();
}

void BudgetTable::Unit() {
    m_BudgetDialog.unit();
    clear();
}

bool BudgetTable::load() {
    if (ADataTable::load()) {
        refreshDatas();
        return true;
    }
    return false;
}

void BudgetTable::unload() {
    ADataTable::unload();
    clear();
}

bool BudgetTable::drawMenu() {
    return false;
}

BudgetDialog& BudgetTable::getBudgetDialogRef() {
    return m_BudgetDialog;
}

size_t BudgetTable::m_getItemsCount() const {
    return m_Datas.transactions_filtered.size();
}

RowID BudgetTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Datas.transactions_filtered.size()) {
        return m_Datas.transactions_filtered.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

double BudgetTable::m_getItemBarAmount(const size_t& vIdx) const {
    return m_Datas.transactions_filtered.at(vIdx).solde;
}

void BudgetTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_Datas.transactions_filtered.at(vIdx);

    ImGui::TableNextColumn();
    {
        if (m_IsGroupingModeBudget()) {
            ImGui::PushID(t.id);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            if (ImGui::Checkbox("##check", &t.confirmed)) {
                DataBase::Instance()->ConfirmBudget(t.id, t.confirmed);
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
        ImGui::Text("%s", t.comment.c_str());
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

void BudgetTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 2);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Dates", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Descriptions", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Comments", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Debit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Credit", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Bars", ImGuiTableColumnFlags_WidthFixed);
    m_drawSearchRow();
    ImGui::TableHeadersRow();
}

void BudgetTable::m_drawContextMenuContent() {
    if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update selection")) {
            std::vector<Budget> transactions_to_update;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_IsRowSelected(trans.id)) {
                    transactions_to_update.push_back(trans);
                }
            }
            if (transactions_to_update.size() > 1U) {
                m_BudgetDialog.setBudgetToUpdate(transactions_to_update);
                m_BudgetDialog.show(DataDialogMode::MODE_UPDATE_ALL);
            } else if (transactions_to_update.size() == 1U) {
                m_BudgetDialog.setBudget(transactions_to_update.front());
                m_BudgetDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
            }
        }
        if (ImGui::MenuItem("Delete selection")) {
            std::vector<Budget> transactions_to_delete;
            for (const auto& trans : m_Datas.transactions_filtered) {
                if (m_IsRowSelected(trans.id)) {
                    transactions_to_delete.push_back(trans);
                }
            }
            m_BudgetDialog.setBudgetToDelete(transactions_to_delete);
            m_BudgetDialog.show(DataDialogMode::MODE_DELETE_ALL);
        }
    }
}

void BudgetTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    m_BudgetDialog.setBudget(m_Datas.transactions_filtered.at(vIdx));
    m_BudgetDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
}

void BudgetTable::refreshDatas() {
    m_UpdateBanks();
    m_UpdateEntities();
    m_UpdateCategories();
    m_UpdateOperations();
    m_UpdateAccounts();
}

bool BudgetTable::m_isGroupingModeBudget() {
    return (m_GroupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS);
}

void BudgetTable::m_drawSearchRow() {
    bool change = false;
    bool reset = false;
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (m_IsGroupingModeBudget()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::ContrastedButton("R", nullptr, nullptr, ImGui::GetColumnWidth(0))) {
            reset = true;
        }
        ImGui::PopStyleVar();
    }
    for (size_t idx = 0; idx < 10; ++idx) {
        ImGui::TableNextColumn();
        if (idx < SearchColumns::SEARCH_COLUMN_Count) {
            if (m_IsGroupingModeBudget()) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                if (m_SearchInputTexts.at(idx).DisplayInputText(ImGui::GetColumnWidth(idx), "", "")) {
                    m_SearchTokens[idx] = ct::toLower(m_SearchInputTexts.at(idx).GetText());
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

void BudgetTable::drawSelectMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::BeginMenu("Select")) {
        if (ImGui::BeginMenu("Rows")) {
            if (ImGui::MenuItem("Displayed")) {
                m_selectRows(0, m_getItemsCount());
            }
            if (ImGui::MenuItem("UnConfirmed")) {
                m_SelectUnConfirmedBudget();
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

void BudgetTable::drawDebugMenu(FrameActionSystem& vFrameActionSystem) {
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
            if (ImGui::MenuItem("Budget")) {
                DataBase::Instance()->DeleteBudget();
                refreshDatas();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
#endif
}

void BudgetTable::drawGroupingMenu(FrameActionSystem& vFrameActionSystem) {
    if (ImGui::MenuItem("T", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS)) {
        m_GroupBudget(GroupingMode::GROUPING_MODE_TRANSACTIONS);
    }
    if (ImGui::MenuItem("D", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_DAYS)) {
        m_GroupBudget(GroupingMode::GROUPING_MODE_DAYS);
    }
    if (ImGui::MenuItem("M", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_MONTHS)) {
        m_GroupBudget(GroupingMode::GROUPING_MODE_MONTHS);
    }
    if (ImGui::MenuItem("Y", nullptr, m_GroupingMode == GroupingMode::GROUPING_MODE_YEARS)) {
        m_GroupBudget(GroupingMode::GROUPING_MODE_YEARS);
    }
}

void BudgetTable::drawAccountsMenu(FrameActionSystem& vFrameActionSystem) {
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
                                        if (ImGui::Selectable(a.number.c_str(), m_getAccountComboRef().getIndex() == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                                            m_ResetSelection();
                                            m_UpdateBudget(a.id);
                                            m_getAccountComboRef().getIndexRef() = idx;
                                        }
                                    }
                                    ImGui::PopID();

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%s", a.name.c_str());

                                    ImGui::TableNextColumn();
                                    ImGui::Text("%s", a.type.c_str());

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

void BudgetTable::clear() {
    m_Datas.clear();
}

void BudgetTable::resetFiltering() {
    m_Datas.transactions_filtered = m_Datas.transactions;
    m_Datas.transactions_filtered_rowids = {};
    m_SearchInputTexts = {};
    m_SearchTokens = {};
    m_FilteringMode = FilteringMode::FILTERING_MODE_BY_SEARCH;
    m_Datas.filtered_selected_transactions.clear();
    refreshFiltering();
}

void BudgetTable::refreshFiltering() {
    m_Datas.transactions_filtered.clear();
    m_Datas.transactions_filtered_rowids.clear();
    bool use = false;
    double solde = m_CurrentBaseSolde;
    m_TotalDebit = 0.0;
    m_TotalCredit = 0.0;
    for (auto t : m_Datas.transactions) {
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
            m_Datas.transactions_filtered.push_back(t);
            m_Datas.transactions_filtered_rowids.emplace(t.id);
            m_TotalDebit += t.debit;
            m_TotalCredit += t.credit;
        }
    }
}

void BudgetTable::m_FilterSelection() {
    m_FilteringMode = FilteringMode::FILTERING_MODE_BY_SELECTED_ROW_IDS;
    m_Datas.filtered_selected_transactions = m_getSelectedRows();
    refreshFiltering();
}

void BudgetTable::m_SelectPossibleDuplicateEntryOnPricesAndDates() {
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        RowID account_id = m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id;
        m_ResetSelection();
        DataBase::Instance()->GetDuplicateBudgetOnDatesAndAmount(  //
            account_id,                                                  //
            [this](const RowID& vRowID) {
                if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                    m_Datas.transactions_filtered_rowids.end()) {
                    m_SelectRow(vRowID);  // select row id
                }
            });
    }
}

void BudgetTable::m_SelectUnConfirmedBudget() {
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        RowID account_id = m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id;
        m_ResetSelection();
        DataBase::Instance()->GetUnConfirmedBudget(  //
            account_id,                                    //
            [this](const RowID& vRowID) {
                if (m_Datas.transactions_filtered_rowids.find(vRowID) !=  //
                    m_Datas.transactions_filtered_rowids.end()) {
                    m_SelectRow(vRowID);  // select row id
                }
            });
    }
}

void BudgetTable::m_SelectEmptyColumn(const SearchColumns& vColumn) {
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

void BudgetTable::m_GroupBudget(const GroupingMode& vGroupingMode) {
    m_GroupingMode = vGroupingMode;
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        m_UpdateBudget(m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id);
    }
}

void BudgetTable::m_UpdateBanks() {
    m_Datas.bankNames.clear();
    DataBase::Instance()->GetBanks(                                       //
        [this](const BankName& vUserName, const std::string& /*vUrl) {  //
    m_Datas.bankNames.push_back(vUserName);
});
}

void BudgetTable::m_UpdateEntities() {
    m_Datas.entityNames.clear();
    DataBase::Instance()->GetEntities(           //
        [this](const EntityName& vEntityName) {  //
            m_Datas.entityNames.push_back(vEntityName);
        });
}

void BudgetTable::m_UpdateCategories() {
    m_Datas.categoryNames.clear();
    DataBase::Instance()->GetCategories(             //
        [this](const CategoryName& vCategoryName) {  //
            m_Datas.categoryNames.push_back(vCategoryName);
        });
}

void BudgetTable::m_UpdateOperations() {
    m_Datas.operationNames.clear();
    DataBase::Instance()->GetOperations(               //
        [this](const OperationName& vOperationName) {  //
            m_Datas.operationNames.push_back(vOperationName);
        });
}

void BudgetTable::m_UpdateAccounts() {
    m_Accounts.clear();
    m_Datas.accounts.clear();
    m_Datas.accountNumbers.clear();
    DataBase::Instance()->GetAccounts(  //
        [this](const RowID& vRowID,
               const BankName& vBankName,
               const BankAgency& vBankAgency,
               const AccountType& vAccountType,
               const AccountName& vAccountName,
               const AccountNumber& vAccountNumber,
               const AccounBaseSolde& vBaseSolde,
               const BudgetCount& vCount) {  //
            Account a;
            a.id = vRowID;
            a.bank = vBankName;
            a.agency = vBankAgency;
            a.type = vAccountType;
            a.name = vAccountName;
            a.number = vAccountNumber;
            a.base_solde = vBaseSolde;
            a.count = vCount;
            m_Datas.accounts.push_back(a);
            m_Datas.accountNumbers.push_back(vAccountNumber);
            m_Accounts[vBankName + "##BankName"][vBankAgency + "##BankAgency"][vAccountNumber] = a;
        });
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        m_UpdateBudget(m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id);
    }
}

void BudgetTable::m_UpdateBudget(const RowID& vAccountID) {
    m_Datas.transactions.clear();
    const auto& zero_based_account_id = vAccountID - 1;
    if (zero_based_account_id < m_Datas.accounts.size()) {
        double solde = m_CurrentBaseSolde = m_Datas.accounts.at(zero_based_account_id).base_solde;
        const auto& account_number = m_Datas.accounts.at(zero_based_account_id).number;
        if (m_IsGroupingModeBudget()) {
            DataBase::Instance()->GetBudget(     //
                vAccountID,                      //
                [this, &solde, account_number](  //
                    const RowID& vBudgetID,
                    const EntityName& vEntityName,
                    const CategoryName& vCategoryName,
                    const OperationName& vOperationName,
                    const SourceName& vSourceName,
                    const BudgetDate& vDate,
                    const BudgetDescription& vDescription,
                    const BudgetComment& vComment,
                    const BudgetAmount& vAmount,
                    const BudgetConfirmed& vConfirmed,
                    const BudgetHash& vHash) {  //
                    solde += vAmount;
                    Budget t;
                    t.id = vBudgetID;
                    t.account = account_number;
                    t.optimized[0] = ct::toLower(t.date = vDate);
                    t.optimized[1] = ct::toLower(t.description = vDescription);
                    t.optimized[2] = ct::toLower(t.comment = vComment);
                    t.optimized[3] = ct::toLower(t.entity = vEntityName);
                    t.optimized[4] = ct::toLower(t.category = vCategoryName);
                    t.optimized[5] = ct::toLower(t.operation = vOperationName);
                    t.hash = vHash;
                    t.source = vSourceName;
                    t.debit = vAmount < 0.0 ? vAmount : 0.0;
                    t.credit = vAmount > 0.0 ? vAmount : 0.0;
                    t.amount = vAmount;
                    t.confirmed = vConfirmed;
                    t.solde = solde;
                    m_Datas.transactions.push_back(t);
                });
        } else {
            DataBase::Instance()->GetGroupedBudget(  //
                vAccountID,
                GroupBy::DATES,
                (DateFormat)(m_GroupingMode - 1),
                [this](  //
                    const RowID& vRowID,
                    const BudgetDate& vBudgetDate,
                    const BudgetDescription& vBudgetDescription,
                    const EntityName& vEntityName,
                    const CategoryName& vCategoryName,
                    const OperationName& vOperationName,
                    const BudgetDebit& vBudgetDebit,
                    const BudgetCredit& vBudgetCredit) {
                    Budget t;
                    t.id = vRowID;
                    t.date = vBudgetDate;
                    t.description = "-- grouped --";
                    t.entity = "-- grouped --";
                    t.category = "-- grouped --";
                    t.operation = "-- grouped --";
                    t.debit = vBudgetDebit;
                    t.credit = vBudgetCredit;
                    t.amount = vBudgetDebit + vBudgetCredit;
                    m_Datas.transactions.push_back(t);
                });
        }
    }
    refreshFiltering();
}

bool BudgetTable::m_IsGroupingModeBudget() {
    return (m_GroupingMode == GroupingMode::GROUPING_MODE_TRANSACTIONS);
}

void BudgetTable::m_drawAmount(const double& vAmount) {
    if (vAmount < 0.0) {
        const auto& bad_color = ImGui::GetColorU32(ImVec4(1, 0, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, bad_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else if (vAmount > 0.0) {
        const auto& good_color = ImGui::GetColorU32(ImVec4(0, 1, 0, 1));
        ImGui::PushStyleColor(ImGuiCol_Text, good_color);
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
        ImGui::PopStyleColor();
    } else {
        ImGui::Text("%.2f", vAmount);
        ImGui::HideByFilledRectForHiddenMode(SettingsDialog::Instance()->isHiddenMode(), "%.2f", vAmount);
    }
}
*/