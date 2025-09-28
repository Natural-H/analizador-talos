#ifndef ASSERTER_H
#define ASSERTER_H

#include <string>
#include <map>
#include <stack>

class Asserter
{
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
        Unasigned
    };

    enum Operator {
        Add = 0,
        Dif,
        Mul,
        Div
    };

    std::map<std::string, Type> variablesTypes;

    std::stack<Type> typesStack;
    std::stack<Operator> operatorsStack;

    Type applyOperator(Type, Type, Operator);

    Type operatorsTable[5][4][5] = {
        {   // Int allowed operators
            //int float  char   string bool
            {Int, Float, Error, Error, Error}, // add
            {Int, Float, Error, Error, Error}, // diff
            {Int, Float, Error, Error, Error}, // mult
            {Int, Float, Error, Error, Error}  // div
        },
        {   // Float allowed operators
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error},
            {Float, Float, Error, Error, Error}
        },
        {   // Char allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error}
        },
        {   // String allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error}
        },
        {   // Bool allowed operators
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error},
            {Error, Error, Error, Error, Error}
        },
    };
};

#endif // ASSERTER_H
