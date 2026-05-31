#pragma once

#include <string>
#include <vector>
#include <Headers/DatasDef.h>

// evaluates user-defined categorization rules against transactions to suggest a category/operation
namespace CategorizationEngine {

// evaluates the rules (in order) against a transaction.
// fills vOutCategory / vOutOperation with the first matching rule that proposes each (empty if none).
// returns true if at least one of category/operation was proposed.
bool evaluate(  //
    const std::vector<CategorizationRule>& vRules,
    const TransactionOutput& vTransaction,
    std::string& vOutCategory,
    std::string& vOutOperation);

}  // namespace CategorizationEngine
