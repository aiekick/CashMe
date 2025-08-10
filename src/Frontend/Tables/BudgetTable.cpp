#include <Frontend/Tables/BudgetTable.h>
#include <Systems/SettingsDialog.h>
#include <Frontend/MainFrontend.h>
#include <Models/DataBase.h>
#include <ezlibs/ezDate.hpp>
#include <ezlibs/ezTime.hpp>
#include <vector>

BudgetTable::BudgetTable() : ADataTable("BudgetTable", 9) {}

void BudgetTable::clear() {
    m_budgets.clear();
}

void BudgetTable::refreshDatas() {
    m_budgets.clear();
    m_updateAccounts();
    m_computeBudget();
    /*DataBase::ref().ComputeBudget(  //
        m_getAccountID(),           //
        m_projectedDays,
        m_useOptional,
        m_hideEmptyRows,
        [this](const BudgetOutput& vBudgetOutput) {  //
            m_budgets.push_back(vBudgetOutput);
        });*/
    m_budgetGraph.prepare(m_budgets);
}

bool BudgetTable::m_drawMenu() {
    bool ret = false;
    ret |= ImGui::MenuItem("Refresh");
    if (ImGui::BeginMenu("Settings")) {
        ImGui::MenuItem("Show/Hide Table", nullptr, &m_showTable);
        ImGui::MenuItem("Show/Hide Graph", nullptr, &m_showGraph);
        ret |= ImGui::MenuItem("Use/Unuse Optionals", nullptr, &m_useOptional);
        ret |= ImGui::MenuItem("Show/Hide Empty rows", nullptr, &m_hideEmptyRows);
        ret |= ImGui::SliderUIntDefaultCompact(150.0f, "Projected days", &m_projectedDays, 1, 500, 190);
        ImGui::EndMenu();
    }

    if (ret) {
        refreshDatas();
    }

    return ret;
}

void BudgetTable::m_draw(const ImVec2& vSize) {
    if (m_showGraph) {
        auto size = vSize;
        if (m_showTable) {
            size = ImVec2(-1, 0);
        }
        m_budgetGraph.draw(size);
    }
    if (m_showTable) {
        ADataTable::m_draw(ImVec2(-1, 0));
    }
}

size_t BudgetTable::m_getItemsCount() const {
    return m_budgets.size();
}

RowID BudgetTable::m_getItemRowID(const size_t& vIdx) const {
    if (vIdx < m_budgets.size()) {
        return m_budgets.at(vIdx).id;
    }
    return 0;  // the db row id cant be 0
}

void BudgetTable::m_drawTableContent(const size_t& vIdx, const double& vMaxAmount) {
    auto& t = m_budgets.at(vIdx);
    m_drawColumnSelectable(vIdx, t.id, t.date);
    m_drawColumnAmount(t.delta.min);
    m_drawColumnAmount(t.delta.max);
    m_drawColumnAmount(t.solde.min);
    m_drawColumnAmount(t.solde.max);
    m_drawColumnText(t.incomesMin);
    m_drawColumnText(t.incomesMinAmount);
    m_drawColumnText(t.incomesMax);
    m_drawColumnText(t.incomesMaxAmount);
}

void BudgetTable::m_setupColumns() {
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Delta max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Solde max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Max", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Income Min Amounts", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();
}

bool BudgetTable::m_computeBudget() {
    /* on va computer ca a la main
     * on a deux delta min/max
     * deux courbe de balance min/max
     *
     * Un income doit :
     * apparaître une seule fois par mois (même s'il est valide sur plusieurs jours du mois),
     * tomber à la date effective estimée, qui est :
     * - pour le min : le jour le plus tardif possible (max_day)
     * - pour le max : le jour le plus tôt possible (min_day)
     *
     * le min serait : le moins d'argnet dispo a date
     * - si un montant positif ne vient pas a date
     * - si un montant negatif mini vient a date
     * le max serait : le plus d'argent dispo a date
     * - si un montant negatif ne vient pas a date
     * - si un montant positif maxi vient a date
     *
     * Une transaction deja tombé a un income lié et confirmé
     * et dont le income devrait normalement etre prit en compe
     */

    // m_getAccountID(),           //
    // m_projectedDays,
    // m_useOptional

    // 0) on clear le container output
    m_budgets.clear();

    // 1) on recupere le solde de la dernieres transactions du mois precedent
    double lastMonthBalance = 0.0;
    if (!DataBase::ref().getLastMonthEndBalance(m_getAccountID(), lastMonthBalance)) {
        return false;
    }

    // 2) on recupere les dernieres transactions depuis le debut du mois en cours
    std::string first_day_month;
    std::vector<TransactionOutput> transactions;
    if (!DataBase::ref().getMonthTransactions(                              //
            m_getAccountID(),                                               //
            [&transactions](const TransactionOutput& vTransactionOutput) {  //
                transactions.push_back(vTransactionOutput);
            },
            first_day_month)) {
        return false;
    }

    // 3) on recupere les incomes
    std::vector<IncomeOutput> incomes;
    if (!DataBase::ref().GetIncomes(                         //
            m_getAccountID(),                                //
            [&incomes](const IncomeOutput& vIncomeOutput) {  //
                incomes.push_back(vIncomeOutput);
            })) {
        return false;
    }

    // 4) on genere une liste de datas futur depuis le 1er jour du mois de la derniere transaction
    std::vector<std::string> dates;
    dates.reserve(m_projectedDays);
    for (int32_t i = 0; i < m_projectedDays; ++i) {
        dates.push_back(ez::date::addDays(first_day_month, i));
    }

    /*
    struct BudgetOutput {
        RowID id = 0;
        std::string date;
        DateEpoch dateEpoch = 0;
        BudgetMinMax delta;
        BudgetMinMax solde;
        std::string incomesMin;
        std::string incomesMinAmount;
        std::string incomesMax;
        std::string incomesMaxAmount;
    };
    */

    // on test un 1er remplissage de date
    m_budgets.reserve(dates.size());
    double priceMin = 0.0;
    double priceMax = 0.0;
    for (const auto& date : dates) {
        BudgetOutput bo;
        bo.date = date;
        ez::time::iso8601ToEpoch(bo.date, "%Y-%m-%d", bo.dateEpoch);
        bo.solde.min = priceMin += 0.1;
        bo.solde.max = priceMax += 0.2;
        m_budgets.push_back(bo);
    }
}
