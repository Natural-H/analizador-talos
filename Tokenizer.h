#ifndef ANALYZER_H
#define ANALYZER_H

#include <iostream>
#include <qstring.h>
#include <string>
#include <map>
#include <QObject>
#include <vector>
#include <utility>

#include "Token.h"

class Tokenizer : public QObject
{
    Q_OBJECT
public:
    // static void analyze(const QString& qString, std::vector<Token>& tokens);
    static std::map<int, std::string> tokenMap;
    static std::map<int, std::string> errorMap;
    static std::vector<int> nonDifferentStates;
    static int transitionTable[26][33];

    void setText(std::string text);
    std::string getText();

signals:
    void tokenFound(const Token& token);

public slots:
    Token findNextToken();

private:
    static bool canContinue(int state, char** c, const char* end);
    static int got(char c);

    std::string text;
    char* leftOff = nullptr;
public:
    enum States
    {
        palabraReservada = 100,
        identificador = 101,
        entero = 102,
        real = 103,
        notacionCientifica = 104,
        suma = 105,
        resta = 106,
        multiplicacion = 107,
        division = 108,
        asignacion = 109,
        igual = 110,
        menorQue = 111,
        menorIgualQue = 112,
        mayorQue = 113,
        mayorIgualQue = 114,
        distintoDe = 115,
        notB = 116,
        andB = 117,
        orB = 118,
        parentesisDeApertura = 119,
        parentesisDeCierre = 120,
        corcheteDeApertura = 121,
        corcheteDeCierre = 122,
        puntoYComa = 123,
        coma = 124,
        constanteCaracter = 125,
        constanteString = 126,
        sharp = 127,
        modulo = 128,
        incrementoUnario = 129,
        decrementoUnario = 130,
        comentarioDeLinea = 131,
        comentarioDeBloque = 132,
        potencia = 133,
        punto = 134,
        endOfFile = 499,
        includeKw = 1000,
        libKw = 1001,
        endlibKw = 1002,
        classKw = 1003,
        endclassKw = 1004,
        intKw = 1005,
        floatKw = 1006,
        charKw = 1007,
        stringKw = 1008,
        boolKw = 1009,
        voidKw = 1010,
        ifKw = 1011,
        elseifKw = 1012,
        elseKw = 1013,
        endifKw = 1014,
        doKw = 1015,
        enddoKw = 1016,
        whileKw = 1017,
        endwhileKw = 1018,
        readKw = 1019,
        writeKw = 1020,
        defKw = 1021,
        constKw = 1022,
        ofKw = 1023,
        dowhileKw = 1024,
        forKw = 1025,
        endforKw = 1026,
        functionKw = 1027,
        endfunctionKw = 1028,
        returnKw = 1029,
        toKw = 1030,
    };
};


#endif //ANALYZER_H
