#ifndef GRAMMARYZER_H
#define GRAMMARYZER_H
#include <map>
#include <vector>
#include <QMessageBox>

#include "Tokenizer.h"
#include "Token.h"

#include <algorithm>
#include <iostream>

#include "asserter.h"
#include <sstream>

#include <functional>
#include <stack>

#include "asserter.h"
#include "asserter.h"
#include "asserter.h"
#include "asserter.h"

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

    void newQuadruples(const std::vector<Asserter::Quadruple *> &quadruples);

public:
    Grammaryzer();

    ~Grammaryzer() override;

    void callActions(const std::vector<ProductionAction> &, std::stack<int> &, Token &currentToken);

    GrammarResults checkGrammar();

    Tokenizer *tokenizer;
    Asserter *asserter;

    std::ostringstream logsStream;
    long long rCounter = 0;

    void cleanLogs();

    void printTypesStack();

    void printOperatorsStack();

    void printJumpStack();

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
                logsStream << "Got Type: " << Asserter::typeToString[gotType] << std::endl;
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
                            break;
                        case Tokenizer::States::constanteCaracter:
                            state = Asserter::Type::Char;
                            break;
                        default:
                            state = Asserter::Type::Unassigned;
                    }

                    asserter->varStack.push_back({t.content, state});
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
                    asserter->varStack.push_back({t.content, Asserter::Type::Float});
                    printTypesTable();
                    return;
                }

                asserter->varStack.push_back(*variable);
                printTypesStack();
            }
        },
        {
            {2004}, [&](const Token &t) {
                logsStream << "Got equal sign: " << t.content << std::endl;

                asserter->operatorsStack.emplace_back(Asserter::Operator::Assign);
                printOperatorsStack();
            }
        },
        {
            {2007}, [&](Token &t) {
                logsStream << "Got item: " << t.content << " added to operatorsStack" << std::endl;
                asserter->operatorsStack.emplace_back(static_cast<Asserter::Operator>(t.state - 105));
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
                printOperatorsStack();
            }
        },
        {
            {2010}, [&](Token &t) {
                logsStream << "Got item: " << t.content << " time to check the operation" << std::endl;

                if (asserter->operatorsStack.top() != Asserter::Operator::Assign) {
                    logsStream << "Top of operators Stack wasn't equals sign, it was: " << Asserter::operatorToString[
                        asserter->operatorsStack.top()] << std::endl;
                    asserter->errors.emplace_back(
                        "Top of operators Stack wasn't equals sign, it was: " + Asserter::operatorToString[
                            asserter->operatorsStack.top()]);
                    return;
                }

                asserter->operatorsStack.pop();

                const auto &operand1 = asserter->varStack.pop();
                const auto &operand2 = asserter->varStack.pop();

                if (operand2.type == operand1.type) {
                    if (!asserter->hasErrors())
                        asserter->quadruples.emplace_back(new Asserter::AssignQuadruple(
                            Asserter::Operator::Assign, operand2, operand1.name
                        ));

                    logsStream << "Assigned!" << std::endl;
                } else {
                    logsStream << "Error: Types [" << Asserter::typeToString[operand2.type] << "] and ["
                            << Asserter::typeToString[operand1.type] << "] aren't equal!" << std::endl;
                    asserter->errors.emplace_back(
                        "Error (L: " + std::to_string(t.line + 1) + "): Types [" + Asserter::typeToString[operand2.type]
                        + "] and [" + Asserter::typeToString[operand1.type] + "] aren't equal!");
                }
            }
        },
        {
            {2015}, [&](Token &t) {
                logsStream << "Got a write statement, adding MFF op into operatorsStack" << std::endl;
                asserter->operatorsStack.emplace_back(Asserter::Operator::Mff);
                printOperatorsStack();
            }
        },
        {
            {2017}, [&](Token &t) {
                logsStream << "Got the end of write, removing MFF" << std::endl;
                asserter->operatorsStack.pop();
                printOperatorsStack();
            }
        },
        {
            {2999}, [&](Token &t) {
                if (!asserter->varStack.empty()) {
                    logsStream << "FATAL: typesStack is not empty! it has " << asserter->varStack.size() <<
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
                if (!asserter->jumpStack.empty()) {
                    logsStream << "FATAL: jumpStack is not empty! it has " << asserter->jumpStack.
                            size() << " items, top: " << asserter->jumpStack.top() << std::endl;
                    asserter->errors.emplace_back("FATAL: jumpStack is not empty! Check Logs");
                    printJumpStack();
                }
            }
        },
        {
            {3000}, [&](Token &t) {
                logsStream << "Got an If statement, adding MFF" << std::endl;
                asserter->operatorsStack.emplace_back(Asserter::Operator::Mff);
                printOperatorsStack();
            }
        },
        {
            {3001}, [&](Token &t) {
                logsStream << "Finished condition for If statement, adding SF without destiny" << std::endl;
                asserter->jumpStack.emplace_back(asserter->quadruples.size());
                printJumpStack();

                asserter->quadruples.emplace_back(new Asserter::SFQuadruple(
                    asserter->varStack.pop(),
                    -1));
            }
        },
        {
            {3002}, [&](Token &t) {
                logsStream << "Found else in If Statement, adding SI without destiny" << std::endl;
                logsStream << "Adding " << asserter->quadruples.size() << " to jumpStack" << std::endl;
                asserter->jumpStack.emplace_back(asserter->quadruples.size());
                asserter->quadruples.emplace_back(new Asserter::SIQuadruple(-1));
                logsStream << "Replacing jump of inst: " << asserter->jumpStack[asserter->jumpStack.size() - 2] <<
                        " by " << asserter->quadruples.size() - 1 << std::endl;

                if (const auto sf = dynamic_cast<Asserter::SFQuadruple *>
                        (asserter->quadruples[asserter->jumpStack[asserter->jumpStack.size() - 2]])) {
                    sf->destiny = static_cast<long long>(asserter->quadruples.size());
                    asserter->jumpStack.remove(asserter->jumpStack.size() - 2);
                    printJumpStack();
                    return;
                }

                asserter->errors.emplace_back("FATAL: instruction " +
                                              std::to_string(asserter->jumpStack[asserter->jumpStack.size() - 2]) +
                                              " wasn't an SF!");
            }
        },
        {
            {3004}, [&](Token &t) {
                logsStream << "Got endif keyword, trying to remove MFF" << std::endl;
                const auto removed = asserter->operatorsStack.pop();

                logsStream << "Removing: " << Asserter::operatorToString[removed] << std::endl;

                if (removed != Asserter::Operator::Mff) {
                    asserter->errors.emplace_back(
                        "FATAL: Removed " + Asserter::operatorToString[removed] + " instead of MFF!");
                }

                printOperatorsStack();
            }
        },
        {
            {3100}, [&](Token &t) {
                logsStream << "Got while keyword, adding next instruction to jumpStack and a MFF" << std::endl;
                asserter->jumpStack.emplace_back(asserter->quadruples.size());
                asserter->operatorsStack.emplace_back(Asserter::Operator::Mff);

                printOperatorsStack();
            }
        },
        {
            {3101}, [&](Token &t) {
                logsStream << "Got end of condition of while, adding SF without destiny and adding it to jumpStack" <<
                        std::endl;
                asserter->quadruples.push_back(new Asserter::SFQuadruple(asserter->varStack.pop(), -1));
                asserter->jumpStack.emplace_back(asserter->quadruples.size() - 1);
            }
        },
        {
            {3102}, [&](Token &t) {
                logsStream << "Got endwhile keyword adding SI, removing MFF and replacing SF from " << asserter->
                        jumpStack.top()
                        << " to " << asserter->quadruples.size() << std::endl;
                asserter->operatorsStack.pop();

                const auto sfPos = asserter->jumpStack.pop();
                const auto beginConditionPos = asserter->jumpStack.pop();
                asserter->quadruples.push_back(new Asserter::SIQuadruple(beginConditionPos));

                if (const auto sf = dynamic_cast<Asserter::SFQuadruple *>
                        (asserter->quadruples[sfPos])) {
                    sf->destiny = static_cast<long long>(asserter->quadruples.size());
                    printJumpStack();
                    return;
                }

                asserter->errors.emplace_back("FATAL: instruction " + std::to_string(asserter->jumpStack.top()) +
                                              " wasn't an SF!");
            }
        },
        {
            {3200}, [&](Token &t) {
                logsStream << "Got Do Statement, Adding: " << asserter->quadruples.size() << "to jumpstack" <<
                        std::endl;
                asserter->jumpStack.emplace_back(asserter->quadruples.size());
                printJumpStack();
            }
        },
        {
            {3201}, [&](Token &t) {
                logsStream << "Found the Do While conditional statement, adding an MFF so this does not explode";
                asserter->operatorsStack.emplace_back(Asserter::Operator::Mff);
                printOperatorsStack();
            }
        },
        {
            {3202}, [&](Token &t) {
                logsStream << "Found the end of Do While conditional statement, trying to add the condition quadruple"
                        << std::endl;
                asserter->quadruples.emplace_back(
                    new Asserter::SVQuadruple(asserter->varStack.pop(), asserter->jumpStack.pop()));
            }
        },
        {
            {3203}, [&](Token &t) {
                logsStream << "Found End of Do, attempting to remove MFF";
                const auto removed = asserter->operatorsStack.pop();

                logsStream << "Removing: " << Asserter::operatorToString[removed] << std::endl;
                if (removed != Asserter::Operator::Mff) {
                    asserter->errors.emplace_back(
                        "FATAL: Removed " + Asserter::operatorToString[removed] + " instead of MFF!");
                }

                printOperatorsStack();
            }
        },
        {
            {2300}, [&](Token &t) {
                logsStream << "Found the for keyword, adding an mff to opStack" << std::endl;
                asserter->operatorsStack.emplace_back(Asserter::Operator::Mff);
            }
        },
        {
            {2301}, [&](Token &t) {
                logsStream << "Got ( for the for statement, adding a = to opStack to trick it into an assignation" <<
                        std::endl;
                asserter->operatorsStack.emplace_back(Asserter::Operator::Assign);
            }
        },
        {
            {3302}, [&](Token &t) {
                logsStream << "Got the " << t.content << " keyword adding next quad to jumpStack" << std::endl;
                asserter->jumpStack.emplace_back(asserter->quadruples.size());
            }
        },
        {
            {3304}, [&](Token &t) {
                logsStream << "Adding a SV without destiny" << std::endl;
                asserter->quadruples.emplace_back(new Asserter::SVQuadruple(asserter->varStack.pop(), -1));
                asserter->jumpStack.emplace_back(asserter->quadruples.size() - 1);
            }
        },
        {
            {3305}, [&](Token &t) {
                logsStream << "Got endfor keyword adding SI, removing MFF and replacing SV from " << asserter->
                        jumpStack.top()
                        << " to " << asserter->quadruples.size() << std::endl;
                asserter->operatorsStack.pop();

                const auto svPos = asserter->jumpStack.pop();
                const auto beginConditionPos = asserter->jumpStack.pop();
                asserter->quadruples.push_back(new Asserter::SIQuadruple(beginConditionPos));

                if (const auto sv = dynamic_cast<Asserter::SVQuadruple *>
                        (asserter->quadruples[svPos])) {
                    sv->destiny = static_cast<long long>(asserter->quadruples.size());
                    printJumpStack();
                    return;
                }

                asserter->errors.emplace_back("FATAL: instruction " + std::to_string(asserter->jumpStack.top()) +
                                              " wasn't an SV!");
            }
        },
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

                tryApplyOperators(expectedOperators, t);
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

                tryApplyOperators(expectedOperators, t);
            }
        },
        {
            {2011}, [&](Token &t) {
                logsStream << "Trying to apply Or operator." << std::endl;
                printTypesStack();
                printOperatorsStack();

                const std::vector expectedOperators = {
                    Asserter::Operator::Or,
                };

                tryApplyOperators(expectedOperators, t);
            }
        },
        {
            {2012}, [&](Token &t) {
                logsStream << "Trying to apply And operator." << std::endl;
                printTypesStack();
                printOperatorsStack();

                const std::vector expectedOperators = {
                    Asserter::Operator::And,
                };

                tryApplyOperators(expectedOperators, t);
            }
        },
        {
            {2013}, [&](Token &t) {
                // Not Operator reserved action, maybe we won't need this
            }
        },
        {
            {2014}, [&](Token &t) {
                logsStream << "Trying to apply an OpRel operator." << std::endl;
                printTypesStack();
                printOperatorsStack();

                const std::vector expectedOperators = {
                    Asserter::Operator::Equals,
                    Asserter::Operator::NotEquals,
                    Asserter::Operator::Lesser,
                    Asserter::Operator::LesserEq,
                    Asserter::Operator::Greater,
                    Asserter::Operator::GreaterEq,
                };

                tryApplyOperators(expectedOperators, t);
            }
        },
        {
            {2016}, [&](Token &t) {
                logsStream << "Trying to make a write quadruple" << std::endl;

                const auto [name, type] = asserter->varStack.pop();

                if (!asserter->errors.empty()) {
                    logsStream << "But I have errors, so I'm not making it" << std::endl;
                    return;
                }

                asserter->quadruples.emplace_back(new Asserter::WriteQuadruple(name));
                printOperatorsStack();
            }
        },
        {
            {3003},
            [&](Token &t) {
                logsStream << "Found end of instructions, replacing " << asserter->
                        jumpStack.top() << " by " << asserter
                        ->quadruples.size() - 1
                        << std::endl;

                printJumpStack();

                if (const auto sf = dynamic_cast<Asserter::SFQuadruple *>
                        (asserter->quadruples[asserter->jumpStack.top()])) {
                    sf->destiny = static_cast<long long>(asserter->quadruples.size());
                    asserter->jumpStack.remove(asserter->jumpStack.size() - 1);
                    printJumpStack();
                    return;
                }
                if (const auto si = dynamic_cast<Asserter::SIQuadruple *>
                        (asserter->quadruples[asserter->jumpStack.top()])) {
                    si->destiny = static_cast<long long>(asserter->quadruples.size());
                    asserter->jumpStack.remove(asserter->jumpStack.size() - 1);
                    printJumpStack();
                    return;
                }
                asserter->errors.emplace_back("FATAL: instruction " + std::to_string(asserter->jumpStack[1]) +
                                              asserter->errors.emplace_back(
                                                  "FATAL: instruction " + std::to_string(asserter->jumpStack[1]) +
                                                  " wasn't an SF or SI!"));
            }
        },
        {
            {2303}, [&](Token &t) {
                logsStream << "Got idk what I'm doing, but adding a > to opStack" << std::endl;
                asserter->operatorsStack.emplace_back(Asserter::Operator::Greater);
            }
        }
    };

private:
    void tryApplyOperators(const std::vector<Asserter::Operator> &expectedOperators, const Token &t) {
        if (asserter->operatorsStack.empty())
            return;

        while (std::find(expectedOperators.cbegin(), expectedOperators.cend(), asserter->operatorsStack.top())
               != expectedOperators.cend()) {
            const auto &operand2 = asserter->varStack.pop();
            const auto &operand1 = asserter->varStack.pop();

            const auto &op = asserter->operatorsStack.pop();

            logsStream << "Applying " << Asserter::typeToString[operand1.type] << ", " << Asserter::typeToString[
                operand2.type] << " with operator: " << Asserter::operatorToString[op] << std::endl;

            const auto &result = asserter->applyOperator(operand1.type, operand2.type, op);
            if (result == Asserter::Type::Error) {
                logsStream << "Error: incompatible types, cannot do [" << Asserter::typeToString[operand1.type] <<
                        " " << Asserter::operatorToString[op] << " " << Asserter::typeToString[operand2.type] << "]"
                        << std::endl;

                asserter->errors.emplace_back(
                    "Error (L: " + std::to_string(t.line + 1) + "): incompatible types, cannot do [" + Asserter::
                    typeToString[operand1.type] + " " + Asserter::operatorToString[op] + " " + Asserter::
                    typeToString[operand2.type] + "]");

                logsStream << "Patching with Float in here..." << std::endl;

                asserter->varStack.push_back({"r" + std::to_string(rCounter++), Asserter::Type::Float});
                printTypesStack();
                printOperatorsStack();
                return;
            }

            asserter->varStack.push_back({"r" + std::to_string(rCounter), result});

            if (!asserter->hasErrors())
                asserter->quadruples.push_back(new Asserter::OperationQuadruple(
                    op, operand1, operand2, "r" + std::to_string(rCounter++)
                ));

            printTypesStack();
            printOperatorsStack();
        }
    }

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
            1, 600, 600, 1, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 1,
            1,
            600, 600, 600, 600, 1, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,
            600,
            600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600
        },
        {
            601, 601, 601, 5, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601,
            2, 3,
            601, 601, 601, 601, 4, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601,
            601,
            601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601, 601
        },
        {
            6, 602, 602, 7, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 7,
            7,
            602, 602, 602, 602, 7, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602,
            602,
            602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602, 602
        },
        {
            603, 603, 603, 9, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603,
            9,
            8, 603, 603, 603, 603, 9, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603,
            603,
            603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603, 603
        },
        {
            604, 604, 604, 11, 11, 604, 604, 604, 604, 604, 604, 11, 11, 11, 11, 11, 604, 11, 11, 11, 11, 10, 11,
            604,
            11, 11, 11, 11, 11, 11, 604, 11, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604, 604,
            604,
            604, 604, 604, 604, 604, 604, 604, 604, 604, 11, 11, 604, 604, 604
        },
        {
            605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605,
            605, 605, 13, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 605, 13, 605, 605,
            605,
            605, 605, 605, 605, 605, 605, 605, 13, 605, 12, 605, 605, 605, 605, 605, 605, 605, 605
        },
        {
            606, 606, 606, 14, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606,
            606,
            606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606,
            606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606, 606
        },
        {
            607, 607, 607, 607, 607, 15, 16, 17, 18, 19, 20, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607,
            607,
            607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607,
            607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607, 607
        },
        {
            608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608,
            608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 21, 22, 23, 608, 608, 608, 608, 608, 608, 608,
            608,
            608, 608, 608, 608, 608, 608, 608, 608, 608, 608, 24, 25, 608, 608, 608, 608, 608, 608
        },
        {
            609, 609, 609, 27, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609,
            27,
            27, 609, 609, 609, 609, 26, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609,
            609,
            609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609, 609
        },
        {
            610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610,
            610, 610, 610, 610, 610, 610, 610, 610, 610, 610, 28, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610,
            610,
            610, 610, 610, 610, 610, 610, 610, 29, 610, 610, 610, 610, 610, 610, 610, 610, 610, 610
        },
        {
            611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611,
            611, 611, 611, 611, 611, 611, 611, 611, 611, 611, 30, 611, 611, 611, 611, 611, 611, 611, 611, 611, 611,
            611,
            611, 611, 611, 611, 611, 611, 611, 31, 611, 30, 611, 611, 611, 611, 611, 611, 611, 611
        },
        {
            612, 612, 612, 612, 41, 612, 612, 612, 612, 612, 612, 37, 41, 41, 41, 39, 612, 38, 41, 35, 34, 612, 612,
            612, 41, 40, 41, 612, 41, 33, 612, 32, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 612,
            612,
            612, 612, 612, 612, 612, 612, 612, 612, 612, 612, 36, 36, 612, 612, 612
        },
        {
            613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613,
            613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 42, 613, 613,
            613,
            613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 613, 43, 43, 613, 613, 613
        },
        {
            614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614,
            614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 44, 614, 614,
            614,
            614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614, 614
        },
        {
            615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 45,
            615,
            615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615,
            615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615, 615
        },
        {
            616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 616,
            616, 616, 616, 616, 616, 616, 616, 616, 616, 616, 46, 46, 46, 46, 616, 616, 616, 616, 616, 616, 616,
            616,
            616, 616, 616, 46, 616, 616, 46, 616, 616, 616, 46, 46, 616, 616, 616, 616, 616, 616
        },
        {
            617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617,
            617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617, 617,
            617, 617, 617, 617, 617, 617, 617, 617, 48, 617, 47, 617, 617, 617, 617, 617, 617, 617, 617
        },
        {
            618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 618, 49, 618,
            618,
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
            621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 54, 621, 621, 621, 621, 621,
            621,
            621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621,
            621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621, 621
        },
        {
            622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 55, 622, 622, 622, 622, 622, 622, 622, 622, 622,
            622,
            622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622,
            622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622, 622
        },
        {
            623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 56, 57, 57, 623, 623, 623, 623, 623, 623,
            623,
            623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623,
            623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623, 623
        },
        {
            624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 58, 59, 624, 624, 624, 624, 624, 624,
            624,
            624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624,
            624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624, 624
        },
        {
            625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 60, 625, 625, 625,
            625,
            625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625,
            625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625, 625
        },
        {
            626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626,
            626, 626, 626, 626, 61, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626,
            626,
            626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626, 626
        },
        {
            627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627,
            627, 627, 627, 627, 627, 627, 627, 627, 62, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627,
            627,
            627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627, 627
        },
        {
            628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 628,
            628, 628, 628, 628, 628, 628, 628, 628, 628, 628, 63, 63, 63, 63, 628, 628, 628, 628, 628, 628, 628,
            628,
            628, 628, 628, 63, 628, 628, 63, 628, 628, 628, 63, 63, 628, 628, 628, 628, 628, 628
        },
        {
            629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629,
            629, 629, 629, 629, 629, 629, 629, 629, 629, 65, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629, 629,
            629,
            629, 629, 629, 629, 629, 64, 629, 65, 65, 65, 629, 629, 629, 629, 629, 629, 629, 629
        },
        {
            630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 630,
            630, 630, 630, 630, 630, 630, 630, 630, 630, 630, 66, 66, 66, 66, 630, 630, 630, 630, 630, 630, 630,
            630,
            630, 630, 630, 66, 630, 630, 66, 630, 630, 630, 66, 66, 630, 630, 630, 630, 630, 630
        },
        {
            631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631,
            631, 631, 631, 631, 631, 631, 631, 631, 631, 68, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631, 631,
            631,
            631, 631, 631, 631, 67, 68, 631, 68, 68, 68, 631, 631, 631, 631, 631, 631, 631, 631
        },
        {
            632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 632,
            632, 632, 632, 632, 632, 632, 632, 632, 632, 632, 69, 69, 69, 69, 632, 632, 632, 632, 632, 632, 632,
            632,
            632, 632, 632, 70, 632, 632, 69, 632, 632, 632, 69, 69, 632, 632, 632, 632, 632, 632
        },
        {
            633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 633,
            633, 633, 633, 633, 633, 633, 633, 633, 633, 633, 71, 71, 71, 71, 633, 633, 633, 633, 633, 633, 633,
            633,
            633, 633, 633, 633, 633, 633, 71, 633, 633, 633, 71, 71, 633, 633, 633, 633, 633, 633
        },
        {
            634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634, 634,
            634, 634, 634, 634, 634, 634, 634, 634, 634, 73, 634, 634, 634, 634, 634, 634, 634, 634, 634, 72, 72,
            72,
            72, 72, 72, 634, 73, 73, 634, 73, 73, 73, 634, 634, 634, 634, 634, 634, 634, 634
        },
        {
            635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635,
            635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 74, 76,
            77,
            78, 79, 75, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635, 635
        },
        {
            636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 636,
            636, 636, 636, 636, 636, 636, 636, 636, 636, 636, 80, 80, 80, 80, 636, 636, 636, 636, 636, 636, 636,
            636,
            636, 636, 636, 636, 636, 636, 80, 636, 636, 636, 80, 80, 636, 636, 636, 636, 636, 636
        },
        {
            637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637, 637,
            637, 637, 637, 637, 637, 637, 637, 637, 637, 83, 637, 637, 637, 637, 81, 82, 637, 637, 637, 83, 83, 83,
            83,
            83, 83, 637, 83, 83, 637, 83, 83, 83, 637, 637, 637, 637, 637, 637, 637, 637
        },
        {
            638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 638,
            638, 638, 638, 638, 638, 638, 638, 638, 638, 638, 84, 84, 84, 84, 638, 638, 638, 638, 638, 638, 638,
            638,
            638, 638, 638, 638, 638, 638, 84, 638, 638, 638, 84, 84, 638, 638, 638, 638, 638, 638
        },
        {
            639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639, 639,
            639, 639, 639, 639, 639, 639, 639, 639, 639, 89, 639, 639, 639, 639, 89, 89, 85, 86, 639, 89, 89, 89,
            89,
            89, 89, 639, 89, 89, 639, 89, 89, 89, 639, 639, 87, 639, 639, 88, 639, 639
        },
        {
            640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 640,
            640, 640, 640, 640, 640, 640, 640, 640, 640, 640, 90, 91, 91, 91, 640, 640, 640, 640, 640, 640, 640,
            640,
            640, 640, 640, 640, 640, 640, 92, 640, 640, 640, 91, 91, 640, 640, 640, 640, 640, 640
        },
        {
            641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641, 641,
            641, 641, 641, 641, 641, 641, 641, 641, 641, 94, 641, 641, 641, 641, 94, 94, 94, 94, 641, 94, 94, 94,
            94,
            94, 94, 641, 94, 94, 93, 94, 94, 94, 641, 641, 94, 641, 641, 94, 641, 641
        },
        {
            642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642,
            642, 642, 642, 642, 642, 642, 642, 642, 642, 642, 95, 642, 642, 642, 642, 642, 642, 642, 642, 642, 642,
            642,
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
        {1003, 2000, 13, 1004, 2999},
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
        {1020, 2015, 119, 17, 2016, 120, 2017},
        {29, 18},
        {124, 2016, 17},
        {-100},
        {1019, 119, 101, 6, 120},
        {129, 101},
        {130, 101},
        {129},
        {130},
        {1015, 3200, 13, 1024, 3201, 119, 29, 120, 1016, 3202, 3203},
        {1011, 3000, 119, 29, 120, 3001, 13, 24, 25, 3003, 1014, 3004},
        {1012, 119, 29, 120, 13, 24},
        {-100},
        {1013, 3002, 13},
        {-100},
        {1017, 3100, 119, 29, 120, 3101, 13, 1018, 3102},
        {1025, 2300, 101, 2003, 2003, 119, 2301, 29, 1030, 2010, 3302, 29, 2303, 2014, 120, 3304, 13, 1026, 3305},
        {1029, 29},
        {31, 2011, 30},
        {118, 2007, 29},
        {-100},
        {33, 2012, 32},
        {117, 2007, 31},
        {-100},
        {34},
        {116, 34},
        {37, 35},
        {36, 37, 2014},
        {-100},
        {110, 2007},
        {115, 2007},
        {111, 2007},
        {112, 2007},
        {113, 2007},
        {114, 2007},
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
