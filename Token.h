//
// Created by Marco on 5/7/2025.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <string>

struct Token
{
    std::string content;
    std::string type;
    int state;
    int index;
    int length;
    int line;
    int indexLine;

    [[nodiscard]] bool isError() const
    {
        return (500 <= state && state <= 999) || state < 100;
    }
};

#endif //TOKEN_H
