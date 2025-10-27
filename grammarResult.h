#ifndef GRAMMARRESULT_H
#define GRAMMARRESULT_H
#include <string>
#include <sstream>

#include "Token.h"
#include "Tokenizer.h"

class grammarResult {
public:
    grammarResult() = default;
    virtual ~grammarResult() = default;

    virtual std::string build() = 0;
};

class grammarOk final : public grammarResult {
public:
    std::string build() override;
};

class faultyTokenError final : public grammarResult {
public:
    const Token &token;

    explicit faultyTokenError(const Token &token) : token(token) {
    }

    std::string build() override;
};

class unexpectedTokenError final : public grammarResult {
public:
    const Token &currentToken;
    const int expected;

    explicit unexpectedTokenError(const Token &token, const int expected)
        : currentToken(token), expected(expected) {
    }

    std::string build() override;
};

class unknownTokenError final : public grammarResult {
public:
    const Token &token;

    explicit unknownTokenError(const Token &token)
        : token(token) {
    }

    std::string build() override;
};

class wrongProductionError final : public grammarResult {
public:
    static std::map<int, std::vector<int> > expectedTokens;
    Token &token;
    int production;

    explicit wrongProductionError(Token &token, const int production) : token(token), production(production) {};
    std::string build() override;
};

#endif //GRAMMARRESULT_H
