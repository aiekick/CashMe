#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

// https://chatgpt.com/c/688f6160-bc18-8320-b547-96e36ad5b705

void DataBase::ComputeBudget(  //
    const RowID& vAccountID,
    const BudgetProjectedDays& vProjectedDays,
    std::function<void(const Budget&)> vCallback) {
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);

    
    /*
la date de depart est la la date de la derniere transaction du compte concernés

le solde initial est le solde cumulé de tout les treasnactions (soit le montane cumulé a la dernier)

je veux predire su x jours l'evolution du budget suivant deux courbes :
- solde mini : 
 - un credit pas encore tombé ou un debit tombé
- solde maxi :
 - un credit tombé ou un debit pâs tombé

un income est valide si : (min day et max day sont des offset positif/negatif depuis le jour 0 du mois) mlin_day peut valoir -2
- date d'evaluation >= start_date && date d'evaluation>= min_day
- date d'evaluation <= end_date && date d'evaluation<= max_day

si une transaction passé  est lié a un income valide via transaction.incomi_id et que le champs 
income_confirmed est a 1 alors ca veut dire qu'un income qui aurait pu etre compté dans le budget 
est deja sortie et est deja dans le solde cummulé des trasnactions zet donc n'est pas a reprendre.
 par cotnre il le pourra les prochain coup^. il est considéré utilsié je pour le mois en cours

une ligne = un jour (donc si plusieurs incoems le meme jours, tu fait la somme) et tu me met dans un colonne les diffretn incomde id utilsié ce jour

pas besoin d'afficher les jour ou il n'ya a pas de varaition avec le jour d'avant sur tout les colonne de prix

il me faut des colonnes delta min, delta max, balance min, balance max

un income utilisé ne doit plus l'etre jusqu'au prochain mois

j'utilsie sqlite 3.50.2 via SqliteSpy pour tester la requete

on impose un ou des account_id
    */

    double baseSolde = 0.0; // account.base_solde + sum(trasnactions.amount);


    Budget budget;
    vCallback(budget);
}