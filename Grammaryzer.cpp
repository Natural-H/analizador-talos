#include "Grammaryzer.h"
#include "Utils.h"

Grammaryzer::Grammaryzer() {
    tokenizer = new Tokenizer();
    asserter = new Asserter();
}

Grammaryzer::~Grammaryzer() {
    delete tokenizer;
    delete asserter;
}

GrammarResults Grammaryzer::checkGrammar() {
    std::stack<int> stack;
    stack.emplace(499); // Push EOF token
    stack.emplace(1); // Push initial state
    Token currentToken = tokenizer->findNextToken();

    cleanLogs();

    asserter->errors.clear();
    asserter->variables.clear();
    asserter->typesStack.clear();
    asserter->operatorsStack.clear();

    do {
        if (currentToken.isError()) {
            return {
                faultyTokenError(currentToken).build(),
                asserter->errors
            };
        }

        if (currentToken.state == Tokenizer::States::comentarioDeLinea ||
            currentToken.state == Tokenizer::States::comentarioDeBloque) {
            currentToken = tokenizer->findNextToken();
            continue;
        }

        while (stack.top() >= 2000) {
            std::vector<ProductionAction> filtered;
            std::copy_if(afterStateActions.begin(), afterStateActions.end(), std::back_inserter(filtered),
                         [&](const auto &action) {
                             return std::find(action.triggers.cbegin(), action.triggers.cend(), stack.top()) != action
                                    .triggers.cend();
                         });

            for_each(filtered.begin(), filtered.end(), [&](const auto &action) {
                action.action(currentToken);
                logsStream << std::endl;
            });

            if (!filtered.empty())
                stack.pop();
            else
                break;
        }

        if (stack.top() > 43) {
            if (currentToken.state == 499 && stack.top() == 499) // EOF
                return {grammarOk().build(), asserter->errors};

            if (stack.top() != currentToken.state) {
                return {
                    unexpectedTokenError(currentToken, stack.top()).build(),
                    asserter->errors
                };
            }

            stack.pop();

            while (stack.top() >= 2000) {
                std::vector<ProductionAction> filtered;
                std::copy_if(onTopActions.begin(), onTopActions.end(), std::back_inserter(filtered),
                             [&](const auto &action) {
                                 return std::find(action.triggers.cbegin(), action.triggers.cend(), stack.top()) !=
                                        action.triggers.cend();
                             });

                for_each(filtered.begin(), filtered.end(), [&](const auto &action) {
                    action.action(currentToken);
                    logsStream << std::endl;
                });

                if (!filtered.empty())
                    stack.pop();
                else
                    break;
            }

            currentToken = tokenizer->findNextToken();
            continue;
        }

        auto row = stateToRow.find(currentToken.state);
        if (row == stateToRow.end()) {
            // was unused token
            return {unknownTokenError(currentToken).build(), asserter->errors};
        }
        const int production = matrizPredictiva[stack.top() - 1][row->second];

        if (600 <= production && production <= 999) {
            return {wrongProductionError(currentToken, production).build(), asserter->errors};
        }

        auto array = matrizProducciones[production - 1];
        stack.pop();
        if (array[0] == -100) // Empty production
            continue;

        for_each(array.rbegin(), array.rend(), [&](const int prod) {
            stack.emplace(prod);
        });
    } while (currentToken.state != 499 || !stack.empty());

    // this shouldn't be reachable, and I would be really scared if it is
    return {"Welp, something broke really bad", asserter->errors};
}

void Grammaryzer::cleanLogs() {
    logsStream.str("");
    logsStream.clear();
}

void Grammaryzer::printTypesStack() {
    logsStream << "TypeStack: [";
    std::for_each(asserter->typesStack.crbegin(), asserter->typesStack.crend() - 1, [&](const auto &type) {
        logsStream << asserter->typeToString[type] << ", ";
    });

    logsStream << asserter->typeToString[asserter->typesStack[0]] << "]" << std::endl;
}

void Grammaryzer::printOperatorsStack() {
    logsStream << "OperatorStack: [";
    std::for_each(asserter->operatorsStack.crbegin(), asserter->operatorsStack.crend() - 1, [&](const auto &oper) {
        logsStream << asserter->operatorToString[oper] << ", ";
    });

    logsStream << asserter->operatorToString[asserter->operatorsStack[0]] << "]" << std::endl;
}

void Grammaryzer::printTypesTable() {
    logsStream << "Types table: " << std::endl;
    std::for_each(asserter->variables.cbegin(), asserter->variables.cend(),
                  [&](const Asserter::Variable &item) {
                      logsStream << "[" << item.name << " | " << asserter->typeToString[item.type] << "]"
                              << std::endl;
                  });
}
