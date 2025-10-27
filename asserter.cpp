#include "asserter.h"

Asserter::Asserter() = default;

Asserter::Type Asserter::applyOperator(const Type first, const Type second, const Operator op) const {
    if (first >= Void || second >= Void)
        return Error;

    if (op == Equal)
        return first == second ? first : Error;

    return operatorsTable[first][op][second];
}
