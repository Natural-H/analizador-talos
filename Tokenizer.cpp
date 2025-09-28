#include "Tokenizer.h"

#include "Utils.h"

Token Tokenizer::findNextToken()
{
    char* offset = &text[0];
    const char* end = offset + text.length();

    // leftOff will be keeping track of where the reading stopped
    if (leftOff == nullptr)
        leftOff = offset;

    for (char* c = leftOff; c <= end;)
    {
        int state = 0;
        int column = 0;
        const char* tokenBegin = c;

        do
        {
            if (state == 0)
                tokenBegin = c;

            column = got(*c);
            state = transitionTable[state][column];
        }
        while (canContinue(state, &c, end));

        int length = c - tokenBegin;

        if (length < 1 || std::find(nonDifferentStates.begin(), nonDifferentStates.end(), state) !=
            nonDifferentStates.end())
        {
            length++;
            c++; // I need to skip contiguous tokens and non-different exiting states
        }

        const auto token = std::string(tokenBegin, length);
        const int index = tokenBegin - offset;
        const auto currentLine = std::count(text.begin(), text.begin() + index, '\n');
        leftOff = c;

        if (state == 100)
        {
            const int* keywordState = getKeyFromValue(tokenMap, token);
            if (keywordState != nullptr) // is the token a keyword?
            {
                state = *keywordState;

                Token t = {
                    token, tokenMap[100], state, index, length, static_cast<int>(currentLine),
                    static_cast<int>(findLineIndex(text, '\n', index))
                };
                emit tokenFound(t); // for updating tables on UI
                return t;
            }

            state = 101;
        }

        std::map<int, std::string>& stateMap = state >= 500 ? errorMap : tokenMap; // if state >= 500, it's an error token

        Token t = {
            token, stateMap[state], state, index, length, static_cast<int>(currentLine),
            static_cast<int>(findLineIndex(text, '\n', index))
        };
        emit tokenFound(t); // for updating tables on UI
        return t;
    }

    Token t = {
        "", tokenMap[499], 499, static_cast<int>(text.length()) - 1, 1,
        static_cast<int>(std::count(text.begin(), text.end(), '\n')),
        static_cast<int>(findLineIndex(text, '\n', static_cast<int>(text.length()) - 1)) - 1
    }; // EOF already reached
    return t;
}

void Tokenizer::setText(std::string text)
{
    this->text = std::move(text);
    this->leftOff = nullptr;
}

std::string Tokenizer::getText()
{
    return this->text;
}

bool Tokenizer::canContinue(const int state, char** c, const char* end)
{
    if (state > 25)
        return false;

    (*c)++;
    return *c <= end;
}

int Tokenizer::got(const char c)
{
    switch (c)
    {
    case 'e': return 4;
    case 'E': return 5;
    case '+': return 6;
    case '-': return 7;
    case '*': return 8;
    case '/': return 9;
    case '%': return 10;
    case '=': return 11;
    case '<': return 12;
    case '>': return 13;
    case '!': return 14;
    case '&': return 15;
    case '|': return 16;
    case '#': return 17;
    case '(': return 18;
    case ')': return 19;
    case '[': return 20;
    case ']': return 21;
    case ';': return 22;
    case ',': return 23;
    case '\'': return 24;
    case '"': return 25;
    case '_': return 26;
    case '.': return 27;
    case '\b': return 28;
    case ' ': return 29;
    case '\t': return 30;
    case '\n': return 31;
    case '\0': return 32;
    default: ;
    }

    if (c >= 'a' && c <= 'z')
        return 1;

    if (c >= 'A' && c <= 'Z')
        return 2;

    if (c >= '0' && c <= '9')
        return 3;

    return 0;
}

std::map<int, std::string> Tokenizer::tokenMap = {
    {palabraReservada, "Palabra reservada"},
    {identificador, "Identificador"},
    {entero, "Entero"},
    {real, "Real"},
    {notacionCientifica, "Notación científica"},
    {suma, "Suma"},
    {resta, "Resta"},
    {multiplicacion, "Multiplicación"},
    {division, "Division"},
    {asignacion, "Asignación"},
    {igual, "Igual a"},
    {menorQue, "Menor que"},
    {menorIgualQue, "Menor igual que"},
    {mayorQue, "Mayor que"},
    {mayorIgualQue, "Mayor igual que"},
    {distintoDe, "Distinto de"},
    {notB, "Not"},
    {andB, "And"},
    {orB, "Or"},
    {parentesisDeApertura, "Paréntesis de apertura"},
    {parentesisDeCierre, "Paréntesis de cierre"},
    {corcheteDeApertura, "Corchete de apertura"},
    {corcheteDeCierre, "Corchete de cierre"},
    {puntoYComa, "Punto y coma"},
    {coma, "Coma"},
    {constanteCaracter, "Constante carácter"},
    {constanteString, "Constante  string"},
    {sharp, "Sharp"},
    {modulo, "Modulo"},
    {incrementoUnario, "Incremento unario"},
    {decrementoUnario, "Decremento unario"},
    {comentarioDeLinea, "Comentario de linea"},
    {comentarioDeBloque, "Comentario de bloque"},
    {potencia, "Potencia"},
    {punto, "Punto"},
    {endOfFile, "EOF, this shouldn't be reachable for printing though"},
    {includeKw, "include"},
    {libKw, "lib"},
    {endlibKw, "endlib"},
    {classKw, "class"},
    {endclassKw, "endclass"},
    {intKw, "int"},
    {floatKw, "float"},
    {charKw, "char"},
    {stringKw, "string"},
    {boolKw, "bool"},
    {voidKw, "void"},
    {ifKw, "if"},
    {elseifKw, "elseif"},
    {elseKw, "else"},
    {endifKw, "endif"},
    {doKw, "do"},
    {enddoKw, "enddo"},
    {whileKw, "while"},
    {endwhileKw, "endwhile"},
    {readKw, "read"},
    {writeKw, "write"},
    {defKw, "def"},
    {constKw, "const"},
    {ofKw, "of"},
    {dowhileKw, "dowhile"},
    {forKw, "for"},
    {endforKw, "endfor"},
    {functionKw, "function"},
    {endfunctionKw, "endfunction"},
    {returnKw, "return"},
    {toKw, "to"},
};

std::map<int, std::string> Tokenizer::errorMap = {
    {500, "Valor flotante espera dígitos"},
    {501, "Notación científica espera E o e"},
    {502, "Notación científica espera + o -"},
    {503, "And se esperaba ampersand"},
    {504, "Or se esperaba |"},
    {505, "Constante carácter no puede estar vacía"},
    {506, "No reconocido por este lenguaje"},
    {507, "Se esperaba ' después del carácter"},
    {508, "\" no cerrada"},
    {509, "Comentario de bloque no cerrado"},
};

std::vector<int> Tokenizer::nonDifferentStates = {
    110, 112, 114, 115, 117, 118, 119, 120, 121, 122, 123, 124, 125, 128, 129, 130, 132, 133, 134, 127, 505,
    506
};

int Tokenizer::transitionTable[26][33] = {
    {
        506, 1, 2, 3, 1, 2, 19, 20, 25, 21, 128, 9, 10, 11, 12, 13, 14, 127, 119, 120, 121, 122, 123, 124, 15, 17,
        506, 134, 0, 0, 0, 0, 499
    },
    {
        100, 1, 2, 2, 1, 2, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        100, 100, 100, 2, 100, 100, 100, 100, 100, 100
    },
    {
        101, 2, 2, 2, 2, 2, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101,
        101, 101, 101, 2, 101, 101, 101, 101, 101, 101
    },
    {
        102, 102, 102, 3, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
        102, 102, 102, 102, 102, 4, 102, 102, 102, 102, 102
    },
    {
        500, 500, 500, 5, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
        500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
    },
    {
        103, 103, 103, 5, 6, 6, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103,
        103, 103, 103, 103, 103, 103, 103, 103, 103, 103
    },
    {
        501, 501, 501, 8, 501, 501, 7, 7, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501,
        501, 501, 501, 501, 501, 501, 501, 501, 501, 501
    },
    {
        502, 502, 502, 8, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502,
        502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502
    },
    {
        104, 104, 104, 8, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104,
        104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104
    },
    {
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 110, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109
    },
    {
        111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 112, 111, 111, 111, 111, 111, 111, 111, 111, 111,
        111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111
    },
    {
        113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 114, 113, 113, 113, 113, 113, 113, 113, 113, 113,
        113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113
    },
    {
        116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 115, 116, 116, 116, 116, 116, 116, 116, 116, 116,
        116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116
    },
    {
        503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 117, 503, 503, 503, 503, 503,
        503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503
    },
    {
        504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 118, 504, 504, 504, 504,
        504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504
    },
    {
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 505, 16, 16,
        16, 16, 16, 16, 507, 507
    },
    {
        507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507, 507,
        507, 507, 507, 125, 507, 507, 507, 507, 507, 507, 507, 507
    },
    {
        17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 17,
        17, 17, 17, 17, 508, 508
    },
    {
        126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126,
        126, 126, 126, 126, 17, 126, 126, 126, 126, 126, 126, 126
    },
    {
        105, 105, 105, 105, 105, 105, 129, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105,
        105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105
    },
    {
        106, 106, 106, 106, 106, 106, 106, 130, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106,
        106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106
    },
    {
        108, 108, 108, 108, 108, 108, 108, 108, 23, 22, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108,
        108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108
    },
    {
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 131, 131
    },
    {
        23, 23, 23, 23, 23, 23, 23, 23, 24, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 509
    },
    {
        23, 23, 23, 23, 23, 23, 23, 23, 24, 132, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 509
    },
    {
        107, 107, 107, 107, 107, 107, 107, 107, 133, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107,
        107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107
    }
};
