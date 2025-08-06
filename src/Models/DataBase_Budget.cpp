#include <Models/DataBase.h>
#include <sqlite3.hpp>
#include <ezlibs/ezSqlite.hpp>
#include <ezlibs/ezLog.hpp>

// https://chatgpt.com/c/688f6160-bc18-8320-b547-96e36ad5b705

bool DataBase::ComputeBudget(  //
    const RowID& vAccountID,
    const BudgetProjectedDays& vProjectedDays,
    std::function<void(const BudgetOutput&)> vCallback) {
    bool ret = false;
    if (vAccountID == 0) {
        return ret;
    }
    // no interest to call that without a callback for retrieve datas
    assert(vCallback);
    const auto select_query = ez::str::toStr(R"(WITH RECURSIVE params AS (SELECT %u AS account_id, %u AS projection_days),)", vAccountID, vProjectedDays) +
        R"(
jours(jour, n) AS (
  SELECT date('now'), 0
  UNION ALL
  SELECT date(jour, '+1 day'), n + 1
  FROM jours, params
  WHERE n < projection_days - 1
),
mois_proj AS (
  SELECT DISTINCT strftime('%Y-%m-01', jour) AS premier_jour
  FROM jours
),
incomes_base AS (
  SELECT *
  FROM incomes
  WHERE account_id = (SELECT account_id FROM params)
),
transactions_base AS (
  SELECT *
  FROM transactions
  WHERE account_id = (SELECT account_id FROM params)
),
-- ----------- COURBE MIN -----------
income_min_dates AS (
    -- Dépenses (min_amount < 0) à min_day
    SELECT
        i.id AS income_id,
        i.account_id,
        i.min_amount AS amount,
        CASE
          WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
          ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
        END AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.min_amount < 0
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND ((i.end_date IS NULL OR i.end_date = '') OR
            (CASE
              WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
              ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
            END) <= i.end_date)
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) >= i.start_date

    UNION ALL

    -- Revenus (max_amount > 0) à max_day
    SELECT
        i.id AS income_id,
        i.account_id,
        i.max_amount AS amount,
        CASE
          WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
          ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
        END AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.max_amount > 0
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND ((i.end_date IS NULL OR i.end_date = '') OR
            (CASE
              WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
              ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
            END) <= i.end_date)
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) >= i.start_date

    -- Rattrapage sur le 1er jour de projection (pour min)
    UNION ALL
    -- Dépenses : min_amount à min_day déjà passé mais max_day pas passé
    SELECT
        i.id AS income_id,
        i.account_id,
        i.min_amount AS amount,
        (SELECT MIN(jour) FROM jours) AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.min_amount < 0
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) < (SELECT MIN(jour) FROM jours)
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND (SELECT MIN(jour) FROM jours) >= i.start_date
      AND ((i.end_date IS NULL OR i.end_date = '') OR (SELECT MIN(jour) FROM jours) <= i.end_date)
      AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = i.id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', (SELECT MIN(jour) FROM jours))
      )

    UNION ALL
    -- Revenus : max_amount à max_day déjà passé mais min_day pas passé
    SELECT
        i.id AS income_id,
        i.account_id,
        i.max_amount AS amount,
        (SELECT MIN(jour) FROM jours) AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.max_amount > 0
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) < (SELECT MIN(jour) FROM jours)
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND (SELECT MIN(jour) FROM jours) >= i.start_date
      AND ((i.end_date IS NULL OR i.end_date = '') OR (SELECT MIN(jour) FROM jours) <= i.end_date)
      AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = i.id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', (SELECT MIN(jour) FROM jours))
      )
),

-- ----------- COURBE MAX -----------

income_max_dates AS (
    -- Revenus (max_amount > 0) à min_day
    SELECT
        i.id AS income_id,
        i.account_id,
        i.max_amount AS amount,
        CASE
          WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
          ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
        END AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.max_amount > 0
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND ((i.end_date IS NULL OR i.end_date = '') OR
            (CASE
              WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
              ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
            END) <= i.end_date)
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) >= i.start_date

    UNION ALL

    -- Dépenses (max_amount < 0) à max_day
    SELECT
        i.id AS income_id,
        i.account_id,
        i.max_amount AS amount,
        CASE
          WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
          ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
        END AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.min_amount < 0
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND ((i.end_date IS NULL OR i.end_date = '') OR
            (CASE
              WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
              ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
            END) <= i.end_date)
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) >= i.start_date

    -- Rattrapage sur le 1er jour de projection (pour max)
    UNION ALL
    -- Revenus : max_amount à min_day déjà passé mais max_day pas passé
    SELECT
        i.id AS income_id,
        i.account_id,
        i.max_amount AS amount,
        (SELECT MIN(jour) FROM jours) AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.max_amount > 0
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) < (SELECT MIN(jour) FROM jours)
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND (SELECT MIN(jour) FROM jours) >= i.start_date
      AND ((i.end_date IS NULL OR i.end_date = '') OR (SELECT MIN(jour) FROM jours) <= i.end_date)
      AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = i.id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', (SELECT MIN(jour) FROM jours))
      )

    UNION ALL
    -- Dépenses : min_amount à max_day déjà passé mais min_day pas passé
    SELECT
        i.id AS income_id,
        i.account_id,
        i.min_amount AS amount,
        (SELECT MIN(jour) FROM jours) AS date_apparition
    FROM incomes i
    CROSS JOIN mois_proj m
    WHERE i.min_amount < 0
      AND (CASE
            WHEN i.max_day >= 1 THEN date(m.premier_jour, '+'||(i.max_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.max_day)||' days')
          END) < (SELECT MIN(jour) FROM jours)
      AND (CASE
            WHEN i.min_day >= 1 THEN date(m.premier_jour, '+'||(i.min_day-1)||' days')
            ELSE date(m.premier_jour, '-1 day', (i.min_day)||' days')
          END) >= (SELECT MIN(jour) FROM jours)
      AND (SELECT MIN(jour) FROM jours) >= i.start_date
      AND ((i.end_date IS NULL OR i.end_date = '') OR (SELECT MIN(jour) FROM jours) <= i.end_date)
      AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = i.id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', (SELECT MIN(jour) FROM jours))
      )
)

SELECT
    ROW_NUMBER() OVER (ORDER BY j.jour) AS id,
    j.jour,
    unixepoch(j.jour) AS epoch,

    -- Delta min du jour
    IFNULL((
      SELECT SUM(imd.amount)
      FROM income_min_dates imd
      WHERE imd.account_id = a.id
        AND imd.date_apparition = j.jour
        AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = imd.income_id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
        )
    ), 0) AS delta_min,

    -- Delta max du jour
    IFNULL((
      SELECT SUM(imd.amount)
      FROM income_max_dates imd
      WHERE imd.account_id = a.id
        AND imd.date_apparition = j.jour
        AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = imd.income_id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
        )
    ), 0) AS delta_max,

    -- Solde min cumulatif
    a.base_solde
      + IFNULL((SELECT SUM(t.amount)
          FROM transactions t
          WHERE t.account_id = a.id
            AND t.date <= j.jour), 0)
      + SUM(
        IFNULL((
          SELECT SUM(imd.amount)
          FROM income_min_dates imd
          WHERE imd.account_id = a.id
            AND imd.date_apparition = j.jour
            AND NOT EXISTS (
                SELECT 1 FROM transactions t2
                WHERE t2.income_id = imd.income_id
                  AND t2.income_confirmed = 1
                  AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
            )
        ), 0)
      ) OVER (ORDER BY j.jour) AS solde_min,

    -- Solde max cumulatif
    a.base_solde
      + IFNULL((SELECT SUM(t.amount)
          FROM transactions t
          WHERE t.account_id = a.id
            AND t.date <= j.jour), 0)
      + SUM(
        IFNULL((
          SELECT SUM(imd.amount)
          FROM income_max_dates imd
          WHERE imd.account_id = a.id
            AND imd.date_apparition = j.jour
            AND NOT EXISTS (
                SELECT 1 FROM transactions t2
                WHERE t2.income_id = imd.income_id
                  AND t2.income_confirmed = 1
                  AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
            )
        ), 0)
      ) OVER (ORDER BY j.jour) AS solde_max,

    -- Détail min
    IFNULL((
      SELECT GROUP_CONCAT(imd.income_id)
      FROM income_min_dates imd
      WHERE imd.account_id = a.id
        AND imd.date_apparition = j.jour
        AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = imd.income_id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
        )
    ), '') AS incomes_min_ids,
    IFNULL((
      SELECT GROUP_CONCAT(imd.amount)
      FROM income_min_dates imd
      WHERE imd.account_id = a.id
        AND imd.date_apparition = j.jour
        AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = imd.income_id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
        )
    ), '') AS incomes_min_amounts,

    -- Détail max
    IFNULL((
      SELECT GROUP_CONCAT(imd.income_id)
      FROM income_max_dates imd
      WHERE imd.account_id = a.id
        AND imd.date_apparition = j.jour
        AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = imd.income_id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
        )
    ), '') AS incomes_max_ids,
    IFNULL((
      SELECT GROUP_CONCAT(imd.amount)
      FROM income_max_dates imd
      WHERE imd.account_id = a.id
        AND imd.date_apparition = j.jour
        AND NOT EXISTS (
            SELECT 1 FROM transactions t2
            WHERE t2.income_id = imd.income_id
              AND t2.income_confirmed = 1
              AND strftime('%Y-%m', t2.date) = strftime('%Y-%m', imd.date_apparition)
        )
    ), '') AS incomes_max_amounts

FROM jours j
CROSS JOIN accounts a
JOIN params ON 1=1
WHERE a.id = params.account_id
-- AND (delta_min != 0 OR delta_max !=0) -- seulement els ligne utiles
ORDER BY j.jour
;

)";
    if (m_OpenDB()) {
        sqlite3_stmt* stmt = nullptr;
        int res = m_debug_sqlite3_prepare_v2(__FUNCTION__, m_SqliteDB, select_query.c_str(), (int)select_query.size(), &stmt, nullptr);
        if (res != SQLITE_OK) {
            LogVarError("%s %s", "Fail get budget min with reason", sqlite3_errmsg(m_SqliteDB));
        } else {
            while (res == SQLITE_OK || res == SQLITE_ROW) {
                res = sqlite3_step(stmt);
                if (res == SQLITE_OK || res == SQLITE_ROW) {
                    BudgetOutput bo;
                    bo.id = sqlite3_column_int(stmt, 0);
                    bo.date = ez::sqlite::readStringColumn(stmt, 1);
                    bo.dateEpoch = sqlite3_column_int64(stmt, 2);
                    bo.delta.min = sqlite3_column_double(stmt, 3);
                    bo.delta.max = sqlite3_column_double(stmt, 4);
                    bo.solde.min = sqlite3_column_double(stmt, 5);
                    bo.solde.max = sqlite3_column_double(stmt, 6);
                    bo.incomesMin = ez::sqlite::readStringColumn(stmt, 7);
                    bo.incomesMinAmount = ez::sqlite::readStringColumn(stmt, 8);
                    bo.incomesMax = ez::sqlite::readStringColumn(stmt, 9);
                    bo.incomesMaxAmount = ez::sqlite::readStringColumn(stmt, 10);
                    vCallback(bo);
                    ret = true;
                }
            }
        }
        sqlite3_finalize(stmt);
        m_CloseDB();
    }
    return ret;
}
