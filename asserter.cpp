#include "asserter.h"

Asserter::Asserter() {

}

Asserter::Type Asserter::applyOperator(Type first, Type second, Operator op) {
    if (first >= Void || second >= Void)
        return Error;

    if (op == Equal)
        return first == second ? first : Error;

    return operatorsTable[first][op][second];
}


