#include <Frontend/Tables/BudgetTable.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <Systems/SettingsDialog.h>

BudgetTable::BudgetTable() : ADataTable("BudgetTable", 10) {}

bool BudgetTable::init() {
    return true;
}

void BudgetTable::unit() {
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

size_t BudgetTable::m_getItemsCount() const {
    return m_Datas.budgets.size();
}

RowID BudgetTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_Datas.budgets.size()) {
        return m_Datas.budgets.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

void BudgetTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_Datas.budgets.at(vIdx);
    /*m_drawColumnSelectable(vIdx, t.id, t.name);
    m_drawColumnText(t.startDate);
    m_drawColumnText(t.endDate);
    m_drawColumnText(t.entity);
    m_drawColumnText(t.category);
    m_drawColumnText(t.operation);
    m_drawColumnAmount(t.minAmount);
    m_drawColumnAmount(t.maxAmount);
    m_drawColumnInt(t.minDay);
    m_drawColumnInt(t.maxDay);*/
}

void BudgetTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 2);
    /*ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Start Date", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("End Date", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("EntityOutput", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Min Day", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Max Day", ImGuiTableColumnFlags_WidthFixed);*/
    ImGui::TableHeadersRow();
}

void BudgetTable::m_drawContextMenuContent() {
    /*if (!m_getSelectedRows().empty()) {
        if (ImGui::MenuItem("Update")) {
            std::vector<Income> incomes_to_update;
            for (const auto& trans : m_Datas.budgets) {
                if (m_IsRowSelected(trans.id)) {
                    incomes_to_update.push_back(trans);
                }
            }
            if (incomes_to_update.size() > 1U) {
                m_IncomeDialog.setBudgetToUpdate(incomes_to_update);
                m_IncomeDialog.show(DataDialogMode::MODE_UPDATE_ALL);
            } else if (incomes_to_update.size() == 1U) {
                m_IncomeDialog.setIncome(incomes_to_update.front());
                m_IncomeDialog.show(DataDialogMode::MODE_UPDATE_ONCE);
            }
        }
        if (m_getSelectedRows().size() > 1U) {
            if (ImGui::MenuItem("Merge")) {
                std::vector<Income> incomes_to_update;
                for (const auto& trans : m_Datas.budgets) {
                    if (m_IsRowSelected(trans.id)) {
                        incomes_to_update.push_back(trans);
                    }
                }
                m_IncomeDialog.setBudgetToUpdate(incomes_to_update);
                m_IncomeDialog.show(DataDialogMode::MODE_MERGE_ALL);
            }
        }
        if (ImGui::MenuItem("Delete")) {
            std::vector<Income> incomes_to_delete;
            for (const auto& trans : m_Datas.budgets) {
                if (m_IsRowSelected(trans.id)) {
                    incomes_to_delete.push_back(trans);
                }
            }
            m_IncomeDialog.setBudgetToDelete(incomes_to_delete);
            m_IncomeDialog.show(DataDialogMode::MODE_DELETE_ALL);
        }
    }*/
}

void BudgetTable::m_doActionOnDblClick(const size_t& vIdx, const RowID& vRowID) {
    /*m_IncomeDialog.setIncome(m_Datas.budgets.at(vIdx));
    m_IncomeDialog.show(DataDialogMode::MODE_UPDATE_ONCE);*/
}

void BudgetTable::refreshDatas() {
    m_UpdateAccounts();
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
                DataBase::ref().DeleteBanks();
                refreshDatas();
            }
            if (ImGui::MenuItem("Accounts")) {
                DataBase::ref().DeleteAccounts();
                refreshDatas();
            }
            if (ImGui::MenuItem("Entities")) {
                DataBase::ref().DeleteEntities();
                refreshDatas();
            }
            if (ImGui::MenuItem("Categories")) {
                DataBase::ref().DeleteCategories();
                refreshDatas();
            }
            if (ImGui::MenuItem("Operations")) {
                DataBase::ref().DeleteOperations();
                refreshDatas();
            }
            /*if (ImGui::MenuItem("Budget")) {
                DataBase::ref().DeleteBudget();
                refreshDatas();
            }*/
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
#endif
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
                                        if (ImGui::Selectable(a.datas.number.c_str(), m_getAccountComboRef().getIndex() == idx, ImGuiSelectableFlags_SpanAllColumns)) {
                                            m_ResetSelection();
                                            m_UpdateBudget(a.id);
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

void BudgetTable::clear() {
    m_Datas.clear();
}

void BudgetTable::m_UpdateAccounts() {
    m_Accounts.clear();
    m_Datas.accounts.clear();
    m_Datas.accountNumbers.clear();
    DataBase::ref().GetAccounts(  //
        [this](
            const AccountOutput& vAccountOutput) {  //
            m_Datas.accounts.push_back(vAccountOutput);
            m_Datas.accountNumbers.push_back(vAccountOutput.datas.number);
            m_Accounts[vAccountOutput.bankName + "##BankName"][vAccountOutput.datas.bank_agency + "##BankAgency"][vAccountOutput.datas.number] = vAccountOutput;
        });
    if (m_getAccountComboRef().getIndex() < m_Datas.accounts.size()) {
        m_UpdateBudget(m_Datas.accounts.at(m_getAccountComboRef().getIndex()).id);
    }
}

void BudgetTable::m_UpdateBudget(const RowID& vAccountID) {
    m_Datas.budgets.clear();
    const auto& zero_based_account_id = vAccountID - 1;
    if (zero_based_account_id < m_Datas.accounts.size()) {
        const auto& account_number = m_Datas.accounts.at(zero_based_account_id).datas.number;
        /*DataBase::ref().GetBudget(
            vAccountID,
            [this, account_number](
                const RowID& vIncomeID,
                const IncomeName& vIncomeName,
                const EntityName& vEntityName,
                const CategoryName& vCategoryName,
                const OperationName& vOperationName,
                const IncomeDate& vStartDate,
                const IncomeDateEpoch& vStartDateEpoch,
                const IncomeDate& vEndDate,
                const IncomeDateEpoch& vEndDateEpoch,
                const IncomeAmount& vMinAmount,
                const IncomeAmount& vMaxAmount,
                const IncomeDay& vMinDays,
                const IncomeDay& vMaxDays,
                const IncomeDescription& vDescription) {
                Income in;
                in.id = vIncomeID;
                in.account = account_number;
                in.entity = vEntityName;
                in.category = vCategoryName;
                in.operation = vOperationName;
                in.name = vIncomeName;
                in.startDate = vStartDate;
                in.startDateEpoch = vStartDateEpoch;
                in.endDate = vEndDate;
                in.endDateEpoch = vEndDateEpoch;
                in.minAmount = vMinAmount;
                in.maxAmount = vMaxAmount;
                in.minDay = vMinDays;
                in.maxDay = vMaxDays;
                in.description;
                m_Datas.budgets.push_back(in);
            });
            */
    }
}
