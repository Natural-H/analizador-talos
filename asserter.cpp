#include "asserter.h"

Asserter::Asserter() = default;

Asserter::Type Asserter::findType(const std::string &name) const {
    const auto var = std::find_if(variables.cbegin(), variables.cend(),
                                  [&](const Variable &variable) { return name == variable.name; });

    return var != variables.cend() ? var->type : Error;
}

Asserter::Variable* Asserter::findVar(const std::string &name) const {
    return std::find_if(variables.cbegin(), variables.cend(),
                         [&](const Variable &variable) { return name == variable.name; })._Ptr;
}

bool Asserter::varExists(const std::string &name) const {
    return std::find_if(variables.cbegin(), variables.cend(),
                        [&](const Variable &variable) { return name == variable.name; }) != variables.cend();
}

Asserter::Type Asserter::applyOperator(const Type first, const Type second, const Operator op) const {
    if (first >= Void || second >= Void)
        return Error;

    if (op == Equal)
        return first == second ? first : Error;

    return operatorsTable[first][op][second];
}
