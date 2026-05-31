#include <Systems/CategorizationEngine.h>
#include <ezlibs/ezStr.hpp>

namespace CategorizationEngine {

// case insensitive wildcard ('*') test ; an empty pattern means "column not tested"
static bool patternMatches(const std::string& vPattern, const std::string& vValue) {
    if (vPattern.empty()) {
        return true;
    }
    const std::string lowerValue = ez::str::toLower(vValue);
    const std::string lowerPattern = ez::str::toLower(vPattern);
    return !ez::str::searchForPatternWithWildcards(lowerValue, lowerPattern).empty();
}

// a rule needs at least one condition, and all its specified conditions must hold (AND)
static bool ruleMatches(const CategorizationRule& vRule, const TransactionOutput& vTransaction) {
    const bool hasCondition =                  //
        !vRule.descriptionPattern.empty() ||   //
        !vRule.commentPattern.empty() ||       //
        !vRule.entityPattern.empty() ||        //
        vRule.useAmountRange;
    if (!hasCondition) {
        return false;
    }
    if (!patternMatches(vRule.descriptionPattern, vTransaction.datas.description)) {
        return false;
    }
    if (!patternMatches(vRule.commentPattern, vTransaction.datas.comment)) {
        return false;
    }
    if (!patternMatches(vRule.entityPattern, vTransaction.datas.entity.name)) {
        return false;
    }
    if (vRule.useAmountRange) {
        const double amount = vTransaction.datas.amount;
        if (amount < vRule.amountMin || amount > vRule.amountMax) {
            return false;
        }
    }
    return true;
}

bool evaluate(  //
    const std::vector<CategorizationRule>& vRules,
    const TransactionOutput& vTransaction,
    std::string& vOutCategory,
    std::string& vOutOperation) {
    vOutCategory.clear();
    vOutOperation.clear();
    for (const auto& rule : vRules) {
        if (!rule.enabled) {
            continue;
        }
        if (!ruleMatches(rule, vTransaction)) {
            continue;
        }
        if (vOutCategory.empty() && !rule.targetCategory.empty()) {
            vOutCategory = rule.targetCategory;
        }
        if (vOutOperation.empty() && !rule.targetOperation.empty()) {
            vOutOperation = rule.targetOperation;
        }
        if (!vOutCategory.empty() && !vOutOperation.empty()) {
            break;  // both resolved, first match wins
        }
    }
    return (!vOutCategory.empty() || !vOutOperation.empty());
}

}  // namespace CategorizationEngine
