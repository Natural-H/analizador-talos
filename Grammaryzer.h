#ifndef GRAMMARYZER_H
#define GRAMMARYZER_H
#include <map>
#include <vector>
#include <stack>
#include <QMessageBox>

#include "Tokenizer.h"
#include "Token.h"
#include "grammarResult.h"

#include <algorithm>
#include <iostream>

#include "asserter.h"
#include <sstream>

#include <functional>

struct ProductionAction {
    std::vector<int> triggers;
    std::function<void(Token &)> action;
};

struct GrammarResults {
    std::string grammarResult;
    std::vector<std::string> semanticErrors;
};

class Grammaryzer : public QObject {
    Q_OBJECT

signals:
    void newLogs(const std::ostringstream &oss);

public:
    Grammaryzer();

    ~Grammaryzer() override;

    GrammarResults checkGrammar();

    Tokenizer *tokenizer;
    Asserter *asserter;

    std::ostringstream logsStream;

    void cleanLogs();

    void printTypesStack();

    void printOperatorsStack();

    void printTypesTable();

    std::vector<ProductionAction> onTopActions = {
        {
            {2000}, [&](Token &t) {
                // for debug purposes only
                printTypesTable();
            }
        },
        {
            {2001}, [&](Token &t) {
                logsStream << "Got declaration for: " << t.content << std::endl;

                if (asserter->varExists(t.content)) {
                    logsStream << "Error: " + t.content + " already in here." << std::endl;
                    asserter->errors.emplace_back(
                        "Error (L: " + std::to_string(t.line + 1) + "): " + t.content + " already in here.");
                    return;
                }

                asserter->variables.push_back({t.content, Asserter::Type::Unassigned});
            }
        },
        {
            {2002}, [&](Token &t) {
                const auto gotType = static_cast<Asserter::Type>(t.state - 1005);
                logsStream << "Got Type: " << asserter->typeToString[gotType] << std::endl;
                for (auto &[name, type]: asserter->variables) {
                    if (type == Asserter::Type::Unassigned)
                        type = gotType;
                }
            }
        },
        {
            {2003}, [&](Token &t) {
                logsStream << "Got operand: " << t.content << std::endl;

                if (t.state != 101) {
                    logsStream << "I think I got a constant value, so, I'm not searching if this exists" << std::endl;
                    Asserter::Type state;

                    // ugly conversion 'cause I wasn't using the original convention
                    switch (t.state) {
                        case Tokenizer::States::entero:
                            state = Asserter::Type::Int;
                            break;
                        case Tokenizer::States::real:
                        case Tokenizer::States::notacionCientifica:
                            state = Asserter::Type::Float;
                            break;
                        case Tokenizer::States::constanteString:
                            state = Asserter::Type::String;
                        case Tokenizer::States::constanteCaracter:
                            state = Asserter::Type::Char;
                        default:
                            state = Asserter::Type::Unassigned;
                    }

                    asserter->typesStack.emplace_back(state);
                    printTypesStack();
                    return;
                }

                const auto variable = std::find_if(asserter->variables.cbegin(),
                                                   asserter->variables.cend(),
                                                   [&](const Asserter::Variable &var) {
                                                       return var.name == t.content;
                                                   });
                if (variable == asserter->variables.end()) {
                    logsStream << "Error: item " << t.content << " not found!" << std::endl;
                    asserter->errors.emplace_back(
                        "Error (L: " + std::to_string(t.line + 1) + "): item " + t.content + " not found!");
                    logsStream << "Time to brutally patch this by adding " << t.content << " as Float" << std::endl;

                    asserter->variables.push_back({t.content, Asserter::Type::Float});
                    asserter->typesStack.emplace_back(Asserter::Type::Float);
                    printTypesTable();
                    return;
                }

                asserter->typesStack.emplace_back(variable->type);
                printTypesStack();
            }
        },
        {
            {2004}, [&](const Token &t) {
                logsStream << "Got equal sign: " << t.content << std::endl;

                asserter->operatorsStack.emplace_back(Asserter::Operator::Equal);
                printOperatorsStack();
            }
        },
        {
            {2007}, [&](Token &t) {
                logsStream << "Got item: " << t.content << " added to operatorsStack" << std::endl;
                asserter->operatorsStack.emplace_back(static_cast<Asserter::Operator>(t.state - 105));
                // logsStream << "opStack top: " << asserter->operatorsStack.top() << std::endl;
                printOperatorsStack();
            }
        },
        {
            {2008}, [&](Token &t) {
                logsStream << "Got item: " << t.content << " added MFF to operatorsStack" << std::endl;
                asserter->operatorsStack.emplace_back(Asserter::Operator::Mff);
                printOperatorsStack();
            }
        },
        {
            {2009}, [&](Token &t) {
                logsStream << "Got item: " << t.content << " removed MFF from operatorsStack" << std::endl;
                asserter->operatorsStack.pop();
                // logsStream << "Current top: " << asserter->operatorsStack.top() << std::endl;
                printOperatorsStack();
            }
        },
        {
            {2010}, [&](Token &t) {
                logsStream << "Got item: " << t.content << " time to check the operation" << std::endl;

                if (asserter->operatorsStack.top() != Asserter::Operator::Equal) {
                    logsStream << "Top of operators Stack wasn't equals sign, it was: " << asserter->operatorToString[
                        asserter->operatorsStack.top()] << std::endl;
                    asserter->errors.emplace_back(
                        "Top of operators Stack wasn't equals sign, it was: " + asserter->operatorToString[
                            asserter->operatorsStack.top()]);
                    return;
                }

                asserter->operatorsStack.pop();

                const auto &type2 = asserter->typesStack.pop();
                const auto &type1 = asserter->typesStack.pop();

                if (type1 == type2)
                    logsStream << "Assigned!" << std::endl;
                else {
                    logsStream << "Error: Types [" << asserter->typeToString[type1] << "] and ["
                            << asserter->typeToString[type2] << "] aren't equal!" << std::endl;
                    asserter->errors.emplace_back(
                        "Error (L: " + std::to_string(t.line + 1) + "): Types [" + asserter->typeToString[type1] +
                        "] and [" + asserter->typeToString[type2] + "] aren't equal!");
                }


                if (!asserter->typesStack.empty()) {
                    logsStream << "FATAL: typesStack is not empty! it has " << asserter->typesStack.size() <<
                            " items" << std::endl;
                    asserter->errors.emplace_back("FATAL: typesStack is not empty! Check Logs");
                    printTypesStack();
                }
                if (!asserter->operatorsStack.empty()) {
                    logsStream << "FATAL: operatorsStack is not empty! it has " << asserter->operatorsStack.
                            size() << " items, top: " << asserter->operatorsStack.top() << std::endl;
                    asserter->errors.emplace_back("FATAL: operatorsStack is not empty! Check Logs");
                    printOperatorsStack();
                }
            }
        }
    };

    std::vector<ProductionAction> afterStateActions = {
        {
            {2005}, [&](Token &t) {
                logsStream << "Trying to apply Mul/Div operator." << std::endl;

                printTypesStack();
                printOperatorsStack();

                const std::vector expectedOperators = {
                    Asserter::Operator::Mul,
                    Asserter::Operator::Div,
                };

                while (std::find(expectedOperators.cbegin(), expectedOperators.cend(), asserter->operatorsStack.top())
                       != expectedOperators.cend()) {
                    const auto &operand2 = asserter->typesStack.pop();
                    const auto &operand1 = asserter->typesStack.pop();

                    const auto &op = asserter->operatorsStack.pop();

                    logsStream << "Applying " << asserter->typeToString[operand1] << ", " << asserter->typeToString[
                        operand2] << " with operator: " << asserter->operatorToString[op] << std::endl;
                    const auto &result = asserter->applyOperator(operand1, operand2, op);
                    if (result == Asserter::Type::Error) {
                        logsStream << "Error: incompatible types, cannot do [" << asserter->typeToString[operand1] <<
                                " " << asserter->operatorToString[op] << " " << asserter->typeToString[operand2] << "]"
                                << std::endl;

                        asserter->errors.emplace_back(
                            "Error (L: " + std::to_string(t.line + 1) + "): incompatible types, cannot do [" + asserter
                            ->typeToString[operand1] + " " + asserter->operatorToString[op] + " " + asserter->
                            typeToString[operand2] + "]");

                        logsStream << "Patching with Float in here..." << std::endl;

                        asserter->typesStack.emplace_back(Asserter::Type::Float);
                        printTypesStack();
                        printOperatorsStack();
                        return;
                    }
                    asserter->typesStack.emplace_back(result);
                    printTypesStack();
                    printOperatorsStack();
                }
            }
        },
        {
            {2006}, [&](Token &t) {
                logsStream << "Trying to apply Add/Diff operator." << std::endl;
                printTypesStack();
                printOperatorsStack();

                const std::vector expectedOperators = {
                    Asserter::Operator::Add,
                    Asserter::Operator::Dif,
                };

                while (std::find(expectedOperators.cbegin(), expectedOperators.cend(), asserter->operatorsStack.top())
                       != expectedOperators.cend()) {
                    const auto &operand2 = asserter->typesStack.pop();
                    const auto &operand1 = asserter->typesStack.pop();

                    const auto &op = asserter->operatorsStack.pop();

                    logsStream << "Applying " << asserter->typeToString[operand1] << ", " << asserter->typeToString[
                        operand2] << " with operator: " << asserter->operatorToString[op] << std::endl;
                    const auto &result = asserter->applyOperator(operand1, operand2, op);
                    if (result == Asserter::Type::Error) {
                        logsStream << "Error: incompatible types, cannot do [" << asserter->typeToString[operand1] <<
                                " " << asserter->operatorToString[op] << " " << asserter->typeToString[operand2] << "]"
                                << std::endl;

                        asserter->errors.emplace_back(
                            "Error (L: " + std::to_string(t.line + 1) + "): incompatible types, cannot do [" + asserter
                            ->typeToString[operand1] + " " + asserter->operatorToString[op] + " " + asserter->
                            typeToString[operand2] + "]");

                        logsStream << "Patching with Float in here..." << std::endl;

                        asserter->typesStack.emplace_back(Asserter::Type::Float);
                        printTypesStack();
                        printOperatorsStack();
                        return;
                    }

                    asserter->typesStack.emplace_back(result);
                    printTypesStack();
                    printOperatorsStack();
                }
            }
        },
    };

private:
    const std::map<int, int> stateToRow = {
        {1000, 0},
        {1001, 1},
        {1002, 2},
        {1003, 3},
        {1004, 4},
        {1005, 5},
        {1006, 6},
        {1007, 7},
        {1008, 8},
        {1009, 9},
        {1010, 10},
        {1011, 11},
        {1012, 12},
        {1013, 13},
        {1014, 14},
        {1015, 15},
        {1016, 16},
        {1017, 17},
        {1018, 18},
        {1019, 19},
        {1020, 20},
        {1021, 21},
        {1022, 22},
        {1023, 23},
        {1024, 24},
        {1025, 25},
        {1026, 26},
        {1027, 27},
        {1028, 28},
        {1029, 29},
        {1030, 30},
        {101, 31},
        {102, 32},
        {103, 33},
        {104, 34},
        {105, 35},
        {106, 36},
        {107, 37},
        {108, 38},
        {109, 39},
        {110, 40},
        {111, 41},
        {112, 42},
        {113, 43},
        {114, 44},
        {115, 45},
        {116, 46},
        {117, 47},
        {118, 48},
        {119, 49},
        {120, 50},
        {123, 51},
        {124, 52},
        {125, 53},
        {126, 54},
        {128, 55},
        {129, 56},
        {130, 57},
        {133, 58},
        {134, 59},
        {499, 60}
    };

    int matrizPredictiva[43][61] = {
        {
            1, 600, 600, 1, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 1, 1,
            600, 600, 600, 600, 1, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,
            600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600
        },
        {
            601, 601, 601, 5, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 2, 3,
            601, 601, 601, 601, 4, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601,
            601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601
        },
        {
            6, 602, 602, 7, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 7, 7,
            602, 602, 602, 602, 7, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602,
            602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602
        },
        {
            603, 603, 603, 9, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 9,
            8, 603, 603, 603, 603, 9, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603,
            603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603
        },
        {
            604, 604, 604, 11, 11, 604, 604, 604, 604, 604, 604, 11, 11, 11, 11, 11, 604, 11, 11, 11, 11, 10, 11, 604,
            11, 11, 11, 11, 11, 11, 604, 11, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
            604, 604, 604, 604, 604, 604, 604, 604, 604, 11, 11, 604, 604, 604
        },
        {
            605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605,
            605, 605, 13, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 13, 605, 605, 605,
            605, 605, 605, 605, 605, 605, 605, 13, 605, 12, 605, 605, 605, 605, 605, 605, 605, 605
        },
        {
            606, 606, 606, 14, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606,
            606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606,
            606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606
        },
        {
            607, 607, 607, 607, 607, 15, 16, 17, 18, 19, 20, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607,
            607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607,
            607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607
        },
        {
            608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608,
            608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 21, 22, 23, 608, 608, 608, 608, 608, 608, 608, 608,
            608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 24, 25, 608, 608, 608, 608, 608, 608
        },
        {
            609, 609, 609, 27, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 27,
            27, 609, 609, 609, 609, 26, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609,
            609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609
        },
        {
            610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610,
            610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 28, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610,
            610, 610, 610, 610, 610, 610, 610, 29, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610
        },
        {
            611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611,
            611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 30, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611,
            611, 611, 611, 611, 611, 611, 611, 31, 611, 30, 611, 611, 611, 611, 611, 611, 611, 611
        },
        {
            612, 612, 612, 612, 41, 612, 612, 612, 612, 612, 612, 37, 41, 41, 41, 39, 612, 38, 41, 35, 34, 612, 612,
            612, 41, 40, 41, 612, 41, 33, 612, 32, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
            612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 36, 36, 612, 612, 612
        },
        {
            613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613,
            613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 42, 613, 613, 613,
            613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 43, 43, 613, 613, 613
        },
        {
            614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614,
            614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 44, 614, 614, 614,
            614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614
        },
        {
            615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 45, 615,
            615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615,
            615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615
        },
        {
            616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616,
            616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 46, 46, 46, 46, 616, 616, 616, 616, 616, 616, 616, 616,
            616, 616, 616, 46, 616, 616, 46, 616, 616, 616, 46, 46, 616, 616, 616, 616, 616, 616
        },
        {
            617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617,
            617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617,
            617, 617, 617, 617, 617, 617, 617, 617, 48, 617, 47, 617, 617, 617, 617, 617, 617, 617, 617
        },
        {
            618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 49, 618, 618,
            618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618,
            618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618
        },
        {
            619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619,
            619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619,
            619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 619, 50, 51, 619, 619, 619
        },
        {
            620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620,
            620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620,
            620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 620, 52, 53, 620, 620, 620
        },
        {
            621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 54, 621, 621, 621, 621, 621, 621,
            621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621,
            621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621
        },
        {
            622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 55, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622,
            622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622,
            622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622
        },
        {
            623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 56, 57, 57, 623, 623, 623, 623, 623, 623, 623,
            623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623,
            623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623
        },
        {
            624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 58, 59, 624, 624, 624, 624, 624, 624, 624,
            624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624,
            624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624
        },
        {
            625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 60, 625, 625, 625, 625,
            625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625,
            625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625
        },
        {
            626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626,
            626, 626, 626, 626, 61, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626,
            626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626
        },
        {
            627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627,
            627, 627, 627, 627, 627, 627, 627, 627, 62, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627,
            627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627
        },
        {
            628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628,
            628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 63, 63, 63, 63, 628, 628, 628, 628, 628, 628, 628, 628,
            628, 628, 628, 63, 628, 628, 63, 628, 628, 628, 63, 63, 628, 628, 628, 628, 628, 628
        },
        {
            629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629,
            629, 629, 629, 629, 629, 629, 629, 629, 629, 65, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629,
            629, 629, 629, 629, 629, 64, 629, 65, 65, 65, 629, 629, 629, 629, 629, 629, 629, 629
        },
        {
            630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630,
            630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 66, 66, 66, 66, 630, 630, 630, 630, 630, 630, 630, 630,
            630, 630, 630, 66, 630, 630, 66, 630, 630, 630, 66, 66, 630, 630, 630, 630, 630, 630
        },
        {
            631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631,
            631, 631, 631, 631, 631, 631, 631, 631, 631, 68, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631,
            631, 631, 631, 631, 67, 68, 631, 68, 68, 68, 631, 631, 631, 631, 631, 631, 631, 631
        },
        {
            632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632,
            632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 69, 69, 69, 69, 632, 632, 632, 632, 632, 632, 632, 632,
            632, 632, 632, 70, 632, 632, 69, 632, 632, 632, 69, 69, 632, 632, 632, 632, 632, 632
        },
        {
            633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633,
            633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 71, 71, 71, 71, 633, 633, 633, 633, 633, 633, 633, 633,
            633, 633, 633, 633, 633, 633, 71, 633, 633, 633, 71, 71, 633, 633, 633, 633, 633, 633
        },
        {
            634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634,
            634, 634, 634, 634, 634, 634, 634, 634, 634, 73, 634, 634, 634, 634, 634, 634, 634, 634, 634, 72, 72, 72,
            72, 72, 72, 634, 73, 73, 634, 73, 73, 73, 634, 634, 634, 634, 634, 634, 634, 634
        },
        {
            635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635,
            635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 74, 76, 77,
            78, 79, 75, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635
        },
        {
            636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636,
            636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 80, 80, 80, 80, 636, 636, 636, 636, 636, 636, 636, 636,
            636, 636, 636, 636, 636, 636, 80, 636, 636, 636, 80, 80, 636, 636, 636, 636, 636, 636
        },
        {
            637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637,
            637, 637, 637, 637, 637, 637, 637, 637, 637, 83, 637, 637, 637, 637, 81, 82, 637, 637, 637, 83, 83, 83, 83,
            83, 83, 637, 83, 83, 637, 83, 83, 83, 637, 637, 637, 637, 637, 637, 637, 637
        },
        {
            638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638,
            638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 84, 84, 84, 84, 638, 638, 638, 638, 638, 638, 638, 638,
            638, 638, 638, 638, 638, 638, 84, 638, 638, 638, 84, 84, 638, 638, 638, 638, 638, 638
        },
        {
            639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639,
            639, 639, 639, 639, 639, 639, 639, 639, 639, 89, 639, 639, 639, 639, 89, 89, 85, 86, 639, 89, 89, 89, 89,
            89, 89, 639, 89, 89, 639, 89, 89, 89, 639, 639, 87, 639, 639, 88, 639, 639
        },
        {
            640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640,
            640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 90, 91, 91, 91, 640, 640, 640, 640, 640, 640, 640, 640,
            640, 640, 640, 640, 640, 640, 92, 640, 640, 640, 91, 91, 640, 640, 640, 640, 640, 640
        },
        {
            641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641,
            641, 641, 641, 641, 641, 641, 641, 641, 641, 94, 641, 641, 641, 641, 94, 94, 94, 94, 641, 94, 94, 94, 94,
            94, 94, 641, 94, 94, 93, 94, 94, 94, 641, 641, 94, 641, 641, 94, 641, 641
        },
        {
            642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642,
            642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 95, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642,
            642, 642, 642, 642, 642, 642, 642, 96, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642
        },
    };

    std::vector<std::vector<int> > matrizProducciones = {
        {3, 2, 7},
        {5, 2},
        {4, 2},
        {10, 2},
        {-100},
        {1000, 101, 134, 1001, 123, 3},
        {-100},
        {1022, 101, 109, 9, 123, 4},
        {-100},
        {1021, 101, 2001, 6, 1023, 8, 2002, 123, 5},
        {-100},
        {124, 101, 2001, 6},
        {-100},
        {1003, 2000, 13, 1004},
        {1005},
        {1006},
        {1007},
        {1008},
        {1009},
        {1010},
        {102},
        {103},
        {104},
        {125},
        {126},
        {1027, 101, 109, 8, 119, 11, 120, 5, 13, 1028, 10},
        {-100},
        {101, 6, 109, 8, 12},
        {-100},
        {124, 11},
        {-100},
        {101, 2003, 14},
        {28, 123, 13},
        {16, 123, 13},
        {19, 123, 13},
        {20, 123, 13},
        {23, 13},
        {26, 13},
        {22, 13},
        {27, 13},
        {-100},
        {15, 123, 2010, 13},
        {21, 123, 13},
        {109, 2004, 29},
        {1020, 119, 17, 120},
        {29, 18},
        {124, 17},
        {-100},
        {1019, 119, 101, 6, 120},
        {129, 101},
        {130, 101},
        {129},
        {130},
        {1015, 13, 1024, 119, 29, 120, 1016},
        {1011, 119, 29, 120, 13, 24, 25, 1014},
        {1012, 119, 29, 120, 13, 24},
        {-100},
        {1013, 13},
        {-100},
        {1017, 119, 29, 120, 13, 1018},
        {1025, 101, 119, 29, 1030, 29, 120, 13, 1026},
        {1029, 29},
        {31, 30},
        {118, 29},
        {-100},
        {33, 32},
        {117, 31},
        {-100},
        {34},
        {116, 34},
        {37, 35},
        {36, 37},
        {-100},
        {110},
        {115},
        {111},
        {112},
        {113},
        {114},
        {39, 2006, 38},
        {105, 2007, 37},
        {106, 2007, 37},
        {-100},
        {41, 2005, 40},
        {107, 2007, 39},
        {108, 2007, 39},
        {128, 39},
        {133, 39},
        {-100},
        {101, 2003, 42},
        {9, 2003},
        {119, 2008, 29, 120, 2009},
        {119, 43, 120},
        {-100},
        {101, 6},
        {-100},
    };
};


#endif //GRAMMARYZER_H
