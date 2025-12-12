#include "asserter.h"

Asserter::Asserter() = default;

Asserter::Type Asserter::findType(const std::string &name) const {
    const auto var = std::find_if(variables.cbegin(), variables.cend(),
                                  [&](const Variable &variable) { return name == variable.name; });

    return var != variables.cend() ? var->type : Error;
}

Asserter::Variable Asserter::findVar(const std::string &name) const {
    return *std::find_if(variables.cbegin(), variables.cend(),
                         [&](const Variable &variable) { return name == variable.name; });
}

bool Asserter::varExists(const std::string &name) const {
    return std::find_if(variables.cbegin(), variables.cend(),
                        [&](const Variable &variable) { return name == variable.name; }) != variables.cend();
}

bool Asserter::hasErrors() const {
    return !errors.empty();
}

Asserter::Type Asserter::applyOperator(const Type first, const Type second, const Operator op) const {
    if (first >= Void || second >= Void)
        return Error;

    if (op == Assign)
        return first == second ? first : Error;

    return operatorsTable[first][op][second];
}

std::map<Asserter::Type, std::string> Asserter::typeToString = {
    {Int, "Int"},
    {Float, "Float"},
    {Char, "Char"},
    {String, "String"},
    {Bool, "Bool"},
    {Void, "Void"},
    {Error, "Error"},
    {Unassigned, "Unassigned"},
};

std::map<Asserter::Operator, std::string> Asserter::operatorToString = {
    {Add, "+"},
    {Dif, "-"},
    {Mul, "*"},
    {Div, "/"},
    {Assign, "="},
    {Equals, "=="},
    {Lesser, "<"},
    {LesserEq, "<="},
    {Greater, ">"},
    {GreaterEq, ">="},
    {NotEquals, "!="},
    {Not, "!"},
    {And, "&&"},
    {Or, "||"},
    {Mff, "Mff"},
    {SI, "SI"},
    {SF, "SF"},
    {SV, "SV"},
    {Write, "Write"},
    {Read, "Read"}
};
