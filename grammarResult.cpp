#include "grammarResult.h"

std::string grammarOk::build() {
    return "La gramática es correcta";
}

std::string faultyTokenError::build() {
    std::ostringstream oss;
    oss << "Error léxico: '" << token.content << "' (" + Tokenizer::errorMap[token.state] << ")" <<
            " en índice " << std::to_string(token.index) << " (" << token.line + 1 <<
            ":" << token.indexLine + 1 << ") con estado " << std::to_string(token.state);
    return oss.str();
}

std::string unexpectedTokenError::build() {
    std::ostringstream oss;
    oss << "Error: Se esperaba Token " << expected << " (" << Tokenizer::tokenMap[expected] << ")" <<
            " pero se encontró Token " << currentToken.state << " (" << Tokenizer::tokenMap[currentToken.state] <<
            ") en índice " << currentToken.index << " (" << currentToken.line + 1 << ":" << currentToken.indexLine + 1
            << ")";
    return oss.str();
}

std::string unknownTokenError::build() {
    std::ostringstream oss;
    oss << "Error: Token " << std::to_string(token.state) << " (" << Tokenizer::tokenMap[token.state] <<
            ") no reconocido en índice " << token.index << " (" << token.line + 1 << ":" << token.indexLine + 1 << ")";
    return oss.str();
}

std::string wrongProductionError::build() {
    const std::vector<int> &tokens = expectedTokens[production];
    std::ostringstream oss;

    oss << "Error " << production << ": Token " << token.state << " (" << Tokenizer::tokenMap[token.state] << ") " <<
            "inesperada en índice " << std::to_string(token.index) << " (" << token.line + 1 << ":" << token.indexLine +
            1 << "), posibles tokens esperados:\n";

    std::for_each(tokens.cbegin(), tokens.cend(), [&](const int token) {
        oss << token << " (" << Tokenizer::tokenMap[token] << ")\n";
    });

    return oss.str();
}

std::map<int, std::vector<int>> wrongProductionError::expectedTokens = {
    {600, {1000, 1003, 1021, 1022, 1027}},
    {601, {1003, 1021, 1022, 1027}},
    {602, {1000, 1003, 1021, 1022, 1027}},
    {603, {1003, 1021, 1022, 1027}},
    {
        604,
        {
            1003, 1004, 1011, 1012, 1013, 1014, 1015, 1017, 1018, 1019, 1020, 1021, 1022, 1024, 1025, 1026, 1027, 1028,
            1029, 101, 129, 130
        }
    },
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