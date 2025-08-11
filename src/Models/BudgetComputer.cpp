#include <Models/BudgetComputer.h>
#include <Models/DataBase.h>
#include <ezlibs/ezTools.hpp>
#include <ezlibs/ezDate.hpp>
#include <ezlibs/ezTime.hpp>
#include <ezlibs/ezStr.hpp>
#include <vector>
#include <list>
#include <map>

void BudgetComputer::clear() {
    m_budgets.clear();
}

bool BudgetComputer::compute(const RowID vAccountID, const std::pair<int32_t, uint32_t>& vMonthRangeOffset, const bool vUseOptional) {
    /* on va computer ca a la main
     * on a deux delta min/max
     * deux courbe de balance min/max
     */

    // 0) on clear le container output
    clear();

    // 1) on recupere le solde de la dernieres transactions du mois precedent
    m_lastMonthBalance = 0.0;
    if (!DataBase::ref().getLastMonthEndBalance(vAccountID, vMonthRangeOffset.first, m_lastMonthBalance)) {
        return false;
    }

    // 2) on recupere les dernieres transactions depuis le debut du mois en cours
    std::string first_day_month;
    std::string first_transaction_date;
    std::string last_transaction_date;
    std::map<std::string, std::vector<TransactionOutput>> transactions;
    if (!DataBase::ref().getMonthTransactions(  //
            vAccountID,
            vMonthRangeOffset.first,                                                                                         //
            [&transactions, &first_transaction_date, &last_transaction_date](const TransactionOutput& vTransactionOutput) {  //
                transactions[vTransactionOutput.datas.date].push_back(vTransactionOutput);
                if (first_transaction_date.empty()) {
                    first_transaction_date = vTransactionOutput.datas.date;
                }
                last_transaction_date = vTransactionOutput.datas.date;
            },
            first_day_month)) {
        return false;
    }

    m_countMonthTransactionsDates = ez::date::diffDays(last_transaction_date, first_transaction_date) + 1U;  // +1 because its a count

    // 3) on recupere les incomes
    std::vector<IncomeOutput> incomes;
    if (!DataBase::ref().GetIncomes(                         //
            vAccountID,                                      //
            [&incomes](const IncomeOutput& vIncomeOutput) {  //
                incomes.push_back(vIncomeOutput);
            })) {
        return false;
    }

    // 4) on genere une liste de dates futur depuis le 1er jour du mois de la derniere transaction
    // todo : utiliser les fonctions de conversions de dates pour avoir le bon nombres de jour
   
    const auto& projectFinalDate = ez::date::Date(first_day_month).offsetMonth(static_cast<int32_t>(vMonthRangeOffset.second - vMonthRangeOffset.first)).getDate();
    const auto& projectedDays = ez::date::diffDays(projectFinalDate, first_day_month);
    std::vector<std::string> dates;
    dates.reserve(projectedDays);
    for (int32_t i = 0; i < projectedDays; ++i) {
        dates.push_back(ez::date::addDays(first_day_month, i));
    }

    /*
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
     */

    // 5) en fait on veut avoir les soldes min/max depuis le debut de mois jusqu'a x jours projetés dans le futur
    //    et en plus on va superposer la courbe du realisé (le vrai solde bancaire) pour voir ou on se situ
    m_budgets.reserve(dates.size());
    double soldeMin = m_lastMonthBalance;
    double soldeReal = m_lastMonthBalance;
    double soldeMax = m_lastMonthBalance;
    std::string lastStartOfMonth;
    RowID id = 1;
    for (const auto& date : dates) {
        BudgetOutput bo;
        bo.id = id++;
        bo.date = date;
        if (ez::time::iso8601ToEpoch(date, "%Y-%m-%d", bo.dateEpoch)) {  // for implot x axis

            // new month ?
            const auto startOfMonthFromDate = ez::date::startOfMonth(date);
            if (lastStartOfMonth != startOfMonthFromDate) {
                for (auto& income_ref : incomes) {
                    income_ref.budget.usedInMinCurve = false;
                    income_ref.budget.usedInMaxCurve = false;
                }
                lastStartOfMonth = startOfMonthFromDate;
            }

            // on cherche un income qui pourrait convenir avec date entre min_day et max_days pour la courbe min
            for (auto& income_ref : incomes) {
                if (!vUseOptional && income_ref.datas.optional) {
                    continue;  // optional but we dont want so we skip it
                }
                if (ez::date::diffDays(date, income_ref.datas.startDate) < 0 ||  // income not started
                    ez::date::diffDays(date, income_ref.datas.endDate) > 0) {    // income finished
                    continue;                                                    // not active, we skip it
                }
                const auto minDay = ez::date::dayOfMonthToDate(income_ref.datas.minDay, startOfMonthFromDate);
                const auto maxDay = ez::date::dayOfMonthToDate(income_ref.datas.maxDay, startOfMonthFromDate);
                if (ez::date::diffDays(date, minDay) >= 0 && ez::date::diffDays(date, maxDay) <= 0) {
                    // Min curve
                    bool isMinDebit = (income_ref.datas.minAmount < 0);
                    bool isMinCredit = (income_ref.datas.minAmount > 0);
                    if (!income_ref.budget.usedInMinCurve) {
                        if (isMinDebit ||                       // les debits au plus tot
                            (isMinCredit && date == maxDay)) {  // les credits au dernier moment
                            income_ref.budget.usedInMinCurve = true;
                            bo.delta.min += income_ref.datas.minAmount;
                            soldeMin += income_ref.datas.minAmount;  // les plus gros debits ou les plus petits credits
                            bo.incomesMin += income_ref.datas.name + " : " + std::to_string(income_ref.datas.minAmount) + "\n\t";  // for tooltip
                        }
                    }

                    // Max curve
                    bool isMaxDebit = (income_ref.datas.maxAmount < 0);
                    bool isMaxCredit = (income_ref.datas.maxAmount > 0);
                    if (!income_ref.budget.usedInMaxCurve) {
                        if (isMaxCredit ||                     // les credit au au plus tot
                            (isMaxDebit && date == maxDay)) {  // les debit au dernier moment
                            income_ref.budget.usedInMaxCurve = true;
                            bo.delta.max += income_ref.datas.maxAmount;
                            soldeMax += income_ref.datas.maxAmount;  // les plus gros credits ou les plus petits debits
                            bo.incomesMax += income_ref.datas.name + " : " + std::to_string(income_ref.datas.maxAmount) + "\n\t";  // for tooltip
                        }
                    }
                }
            }

            size_t last_min_end_line = bo.incomesMin.find_last_of("\n\t");
            if (last_min_end_line != std::string::npos) {
                bo.incomesMin = bo.incomesMin.substr(0, last_min_end_line);
            }

            size_t last_max_end_line = bo.incomesMax.find_last_of("\n\t");
            if (last_min_end_line != std::string::npos) {
                bo.incomesMax = bo.incomesMax.substr(0, last_max_end_line);
            }

            // not fully used income will be reset or considered used ? for projected days
            if (date == last_transaction_date) {  // projected days zone
                for (auto& income_ref : incomes) {
                    if (income_ref.budget.usedInMinCurve && !income_ref.budget.usedInMaxCurve) {
                        income_ref.budget.usedInMinCurve = true;
                        income_ref.budget.usedInMaxCurve = true;
                    }
                    if (income_ref.budget.usedInMaxCurve && !income_ref.budget.usedInMinCurve) {
                        income_ref.budget.usedInMaxCurve = true;
                        income_ref.budget.usedInMinCurve = true;
                    }
                }
            }

            // transaction found with the date
            bool isExistingTransaction = (transactions.find(date) != transactions.end());

            if (isExistingTransaction) {
                const auto& transGroup = transactions.at(date);  // before last transaction date real balance
                for (const auto& trans : transGroup) {
                    double delta = soldeReal - trans.datas.amount;
                    soldeReal += trans.datas.amount;
                }
            }

            if ((isExistingTransaction) || (ez::date::diffDays(date, last_transaction_date) < 0)) {  // not in transactions but before last transactions date
                soldeMin = soldeReal;
                soldeMax = soldeReal;
                bo.delta.min = 0.0;
                bo.delta.max = 0.0;
                bo.incomesMin.clear();
                bo.incomesMax.clear();
            }

            bo.solde.min = soldeMin;
            bo.solde.max = soldeMax;
            bo.soldeReal = soldeReal;
            m_budgets.push_back(bo);
        }
    }

    return true;
}

const std::vector<BudgetOutput>& BudgetComputer::getBudgetDatas() const {
    return m_budgets;
}

uint32_t BudgetComputer::getCountMonthTransactionsDates() const {
    return m_countMonthTransactionsDates;
}

double BudgetComputer::getLastMonthBalance() const {
    return m_lastMonthBalance;
}

size_t BudgetComputer::getSoldeRealCount() const {
    return m_soldeRealCount;
}