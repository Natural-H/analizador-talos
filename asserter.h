#ifndef ASSERTER_H
#define ASSERTER_H

#include <string>
#include <map>
#include <QStack>
#include <utility>

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
        SI,
        SF,
        SV
    };

    struct Variable {
        std::string name;
        Type type;
    };

    struct Quadruple {
        Operator op;

        virtual ~Quadruple() = default;

        Quadruple() = default;

        explicit Quadruple(const Operator operator_) : op(operator_) {
        };
    };

    struct OperationQuadruple : Quadruple {
        Variable operand1;
        Variable operand2;
        std::string result;

        OperationQuadruple() = default;

        OperationQuadruple(const Operator op, Variable operand1, Variable operand2,
                           std::string result) : Quadruple(op), operand1(std::move(operand1)),
                                                 operand2(std::move(operand2)), result(std::move(result)) {
        }
    };

    struct AssignQuadruple : Quadruple {
        Variable toAssign;
        std::string result;

        AssignQuadruple() = default;

        AssignQuadruple(const Operator operator_, Variable variable, std::string name) : Quadruple(operator_),
            toAssign(std::move(variable)), result(std::move(name)) {
        };
    };

    struct SIQuadruple : Quadruple {
        long long destiny = 0;

        SIQuadruple() = default;

        explicit SIQuadruple(const long long destiny) : Quadruple(SI), destiny(destiny) {
        }
    };

    struct SFQuadruple : Quadruple {
        long long destiny = 0;
        Variable condition;

        SFQuadruple() = default;

        SFQuadruple(Variable condition, const long long destiny) : Quadruple(SF), destiny(destiny),
            condition(std::move(condition)) {
        }
    };

    struct SVQuadruple : Quadruple {
        long long destiny = 0;
        Variable condition;

        SVQuadruple() = default;

        SVQuadruple(Variable condition, const long long destiny) : Quadruple(SV), destiny(destiny),
            condition(std::move(condition)) {
        }
    };

    Variable emptyVar = {"", Unassigned};

    std::vector<Quadruple *> quadruples;

    // std::map<std::string, Type> variablesTypes;
    std::vector<Variable> variables;

    QStack<Variable> varStack;
    QStack<Operator> operatorsStack;
    QStack<long long> jumpStack;
    std::vector<std::string> errors;

    [[nodiscard]] Type findType(const std::string &name) const;

    [[nodiscard]] Variable findVar(const std::string &name) const;

    [[nodiscard]] bool varExists(const std::string &name) const;

    [[nodiscard]] bool hasErrors() const;

    static std::map<Type, std::string> typeToString;
    static std::map<Operator, std::string> operatorToString;

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
