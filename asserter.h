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
        Assign,
        Equals,
        Lesser,
        LesserEq,
        Greater,
        GreaterEq,
        NotEquals,
        Not,
        And,
        Or,
        Mff,
    };

    struct Variable {
        std::string name;
        Type type;
    };

    // std::map<std::string, Type> variablesTypes;
    std::vector<Variable> variables;

    QStack<Variable> varStack;
    QStack<Operator> operatorsStack;
    std::vector<std::string> errors;

    [[nodiscard]] Type findType(const std::string &name) const;

    [[nodiscard]] Variable findVar(const std::string &name) const;

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
        {Mff, "Mff"}
    };

    [[nodiscard]] Type applyOperator(Type, Type, Operator) const;

    Type operatorsTable[5][13][5] = {
        {
            // Int allowed operators
            //  int  float  char   string bool
            {Int, Float, Error, Error, Error}, // add
            {Int, Float, Error, Error, Error}, // diff
            {Int, Float, Error, Error, Error}, // mult
            {Int, Float, Error, Error, Error}, // div
            {Bool, Bool, Error, Error, Error}, // isEqual,
            {Bool, Bool, Error, Error, Error}, // lesser,
            {Bool, Bool, Error, Error, Error}, // lesserEq,
            {Bool, Bool, Error, Error, Error}, // greater,
            {Bool, Bool, Error, Error, Error}, // greaterEq,
            {Bool, Bool, Error, Error, Error}, // notEqual,
            {Error, Error, Error, Error, Error}, // not_,
            {Error, Error, Error, Error, Error}, // and_,
            {Error, Error, Error, Error, Error} // or_,
        },
        {
            // Float allowed operators
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error},
            {Bool, Bool, Error, Error, Error}, // isEqual,
            {Bool, Bool, Error, Error, Error}, // lesser,
            {Bool, Bool, Error, Error, Error}, // lesserEq,
            {Bool, Bool, Error, Error, Error}, // greater,
            {Bool, Bool, Error, Error, Error}, // greaterEq,
            {Bool, Bool, Error, Error, Error}, // notEqual,
            {Error, Error, Error, Error, Error}, // not_,
            {Error, Error, Error, Error, Error}, // and_,
            {Error, Error, Error, Error, Error} // or_,
        },
        {
            // Char allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Bool, Error, Error}, // isEqual,
            {Error, Error, Error, Error, Error}, // lesser,
            {Error, Error, Error, Error, Error}, // lesserEq,
            {Error, Error, Error, Error, Error}, // greater,
            {Error, Error, Error, Error, Error}, // greaterEq,
            {Error, Error, Bool, Error, Error}, // notEqual,
            {Error, Error, Error, Error, Error}, // not_,
            {Error, Error, Error, Error, Error}, // and_,
            {Error, Error, Error, Error, Error} // or_,
        },
        {
            // String allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Bool, Error}, // isEqual,
            {Error, Error, Error, Error, Error}, // lesser,
            {Error, Error, Error, Error, Error}, // lesserEq,
            {Error, Error, Error, Error, Error}, // greater,
            {Error, Error, Error, Error, Error}, // greaterEq,
            {Error, Error, Error, Bool, Error}, // notEqual,
            {Error, Error, Error, Error, Error}, // not_,
            {Error, Error, Error, Error, Error}, // and_,
            {Error, Error, Error, Error, Error} // or_,
        },
        {
            // Bool allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Bool}, // isEqual,
            {Error, Error, Error, Error, Error}, // lesser,
            {Error, Error, Error, Error, Error}, // lesserEq,
            {Error, Error, Error, Error, Error}, // greater,
            {Error, Error, Error, Error, Error}, // greaterEq,
            {Error, Error, Error, Error, Bool}, // notEqual,
            {Error, Error, Error, Error, Bool}, // not_,
            {Error, Error, Error, Error, Bool}, // and_,
            {Error, Error, Error, Error, Bool} // or_,
        },
    };
};

#endif // ASSERTER_H
