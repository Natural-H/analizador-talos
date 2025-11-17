#ifndef ASSERTER_H
#define ASSERTER_H

#include <string>
#include <map>
#include <QStack>

class Asserter {
public:
    Asserter();

    enum Type {
        Int = 0,
        Float,
        Char,
        String,
        Bool,
        Void,
        Error,
        Unassigned,
    };

    enum Operator {
        Add = 0,
        Dif,
        Mul,
        Div,
        Equal,
        Mff,
    };

    struct Variable {
        std::string name;
        Type type;
    };

    // std::map<std::string, Type> variablesTypes;
    std::vector<Variable> variables;

    QStack<Type> typesStack;
    QStack<Operator> operatorsStack;
    std::vector<std::string> errors;

    [[nodiscard]] Type findType(const std::string &name) const;

    [[nodiscard]] Variable *findVar(const std::string &name) const;

    [[nodiscard]] bool varExists(const std::string &name) const;

    std::map<Type, std::string> typeToString = {
        {Int, "Int"},
        {Float, "Float"},
        {Char, "Char"},
        {String, "String"},
        {Bool, "Bool"},
        {Void, "Void"},
        {Error, "Error"},
        {Unassigned, "Unassigned"},
    };

    std::map<Operator, std::string> operatorToString = {
        {Add, "+"},
        {Dif, "-"},
        {Mul, "*"},
        {Div, "/"},
        {Equal, "="},
        {Mff, "Mff"},
    };

    [[nodiscard]] Type applyOperator(Type, Type, Operator) const;

    Type operatorsTable[5][4][5] = {
        {
            // Int allowed operators
            //  int  float  char   string bool
            {Int, Float, Error, Error, Error}, // add
            {Int, Float, Error, Error, Error}, // diff
            {Int, Float, Error, Error, Error}, // mult
            {Int, Float, Error, Error, Error}, // div
        },
        {
            // Float allowed operators
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error}
        },
        {
            // Char allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error}
        },
        {
            // String allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error}
        },
        {
            // Bool allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error}
        },
    };
};

#endif // ASSERTER_H
