#include "Grammaryzer.h"

#include <sstream>

#include "Utils.h"

Grammaryzer::Grammaryzer()
{
    tokenizer = new Tokenizer();
    asserter = new Asserter();
}

Grammaryzer::~Grammaryzer()
{
    delete tokenizer;
    delete asserter;
}

std::string Grammaryzer::checkGrammar() const
{
    std::stack<int> stack;
    stack.emplace(499); // Push EOF token
    stack.emplace(1); // Push initial state
    Token currentToken = tokenizer->findNextToken();

    asserter->variablesTypes.clear();

    while (!asserter->typesStack.empty())
        asserter->typesStack.pop();

    while (!asserter->operatorsStack.empty())
        asserter->operatorsStack.pop();

    do
    {
        if (currentToken.isError())
        {
            return "Error léxico: '" + currentToken.content + "' (" + Tokenizer::errorMap[currentToken.state] + ")" +
                " en índice " + std::to_string(currentToken.index) + " (" + std::to_string(currentToken.line + 1)
                + ":" + std::to_string(currentToken.indexLine + 1) + ") con estado "
                + std::to_string(currentToken.state);
        }

        if (currentToken.state == Tokenizer::States::comentarioDeLinea ||
            currentToken.state == Tokenizer::States::comentarioDeBloque)
        {
            currentToken = tokenizer->findNextToken();
            continue;
        }

        while (stack.top() >= 2000)
        {
            std::vector<ProductionAction> filtered;
            std::copy_if(afterStateActions.begin(), afterStateActions.end(), std::back_inserter(filtered), [&](ProductionAction action) {
                return std::find(action.triggers.cbegin(), action.triggers.cend(), stack.top()) != action.triggers.cend();
            });

            for_each(filtered.begin(), filtered.end(), [&](ProductionAction action) {
                action.action(currentToken);
            });

            if (!filtered.empty())
                stack.pop();
            else
                break;
        }

        if (stack.top() > 43)
        {
            if (currentToken.state == 499 && stack.top() == 499) // EOF
                return "La gramática es válida!";

            if (stack.top() != currentToken.state)
            {
                return "Error: Se esperaba Token " + std::to_string(stack.top()) + " (" + Tokenizer::tokenMap[stack.top()] + ")" +
                       " pero se encontró Token " + std::to_string(currentToken.state) + " (" +
                       Tokenizer::tokenMap[currentToken.state] + ")" + " en índice " + std::to_string(currentToken.index)
                       + " (" + std::to_string(currentToken.line + 1) + ":" +
                       std::to_string(currentToken.indexLine + 1) + ")";
            }

            stack.pop();

            while (stack.top() >= 2000)
            {
                std::vector<ProductionAction> filtered;
                std::copy_if(onTopActions.begin(), onTopActions.end(), std::back_inserter(filtered), [&](ProductionAction action) {
                    return std::find(action.triggers.cbegin(), action.triggers.cend(), stack.top()) != action.triggers.cend();
                });

                for_each(filtered.begin(), filtered.end(), [&](ProductionAction action) {
                    action.action(currentToken);
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
        if (row == stateToRow.end())
        {
            // was unused token
            return "Error: Token " + std::to_string(currentToken.state) + " (" +
                Tokenizer::tokenMap[currentToken.state] + ") no reconocido en índice " +
                std::to_string(currentToken.index) + " (" + std::to_string(currentToken.line + 1) + ":" +
                std::to_string(currentToken.indexLine + 1) + ")";
        }
        const int production = matrizPredictiva[stack.top() - 1][row->second];

        if (600 <= production && production <= 999)
        {
            const std::vector<int>& tokens = expectedTokens[production];
            std::ostringstream oss;

            std::for_each(tokens.cbegin(), tokens.cend(), [&](const int token)
            {
                oss << token << " (" << Tokenizer::tokenMap[token] << ")\n";
            });

            return "Error " + std::to_string(production) + ": Token " + std::to_string(currentToken.state) + " (" +
                Tokenizer::tokenMap[currentToken.state] + ") " +
                "inesperada en índice " + std::to_string(currentToken.index) + " (" + std::to_string(
                    currentToken.line + 1) + ":" + std::to_string(currentToken.indexLine + 1) +
                "), posibles tokens esperados: \n" + oss.str();
        }

        auto array = matrizProducciones[production - 1];
        stack.pop();
        if (array[0] == -100) // Empty production
            continue;

        for_each(array.rbegin(), array.rend(), [&](const int prod)
        {
            stack.emplace(prod);
        });
    }
    while (currentToken.state != 499 || !stack.empty());

    return "Welp, something broke really bad"; // this shouldn't be reachable, and I would be really scared if it is
}

std::map<int, std::vector<int>> Grammaryzer::expectedTokens = {
    {600, {1000, 1003, 1021, 1022, 1027}},
    {601, {1003, 1021, 1022, 1027}},
    {602, {1000, 1003, 1021, 1022, 1027}},
    {603, {1003, 1021, 1022, 1027}},
    {604, {1003, 1004, 1011, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020, 1021, 1022, 1024, 1025, 1026, 1027, 1028,1029, 101, 129, 130}},
    {605, {1023, 109, 120, 124}},
    {606, {1003}},
    {607, {1005, 1006, 1007, 1008, 1009, 1010}},
    {608, {102, 103, 104, 125, 126}},
    {609, {1003, 1021, 1022, 1027}},
    {610, {101, 120}},
    {611, {101, 120, 124}},
    {612, {1004, 1011, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020, 1024, 1025, 1026, 1028, 1029, 101, 129, 130}},
    {613, {109, 129, 130}},
    {614, {109}},
    {615, {1020}},
    {616, {101, 102, 103, 104, 116, 119, 125, 126}},
    {617, {120, 124}},
    {618, {1019}},
    {619, {129, 130}},
    {620, {129, 130}},
    {621, {1015}},
    {622, {1011}},
    {623, {1012, 1013, 1014}},
    {624, {1013, 1014}},
    {625, {1017}},
    {626, {1025}},
    {627, {1029}},
    {628, {101, 102, 103, 104, 116, 119, 125, 126}},
    {629, {1030, 118, 120, 123, 124}},
    {630, {101, 102, 103, 104, 116, 119, 125, 126}},
    {631, {1030, 117, 118, 120, 123, 124}},
    {632, {101, 102, 103, 104, 116, 119, 125, 126}},
    {633, {101, 102, 103, 104, 119, 125, 126}},
    {634, {1030, 110, 111, 112, 113, 114, 115, 117, 118, 120, 123, 124}},
    {635, {110, 111, 112, 113, 114, 115}},
    {636, {101, 102, 103, 104, 119, 125, 126}},
    {637, {1030, 105, 106, 110, 111, 112, 113, 114, 115, 117, 118, 120, 123, 124}},
    {638, {101, 102, 103, 104, 119, 125, 126}},
    {639, {1030, 105, 106, 107, 108, 110, 111, 112, 113, 114, 115, 117, 118, 120, 123, 124, 128, 133}},
    {640, {101, 102, 103, 104, 119, 125, 126}},
    {641, {1030, 105, 106, 107, 108, 110, 111, 112, 113, 114, 115, 117, 118, 119, 120, 123, 124, 128, 133}},
    {642, {101, 120}},
};
