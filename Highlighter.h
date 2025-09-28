#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QStyleHints>

#include "Tokenizer.h"
#include "Token.h"

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument* parent = nullptr);
    std::vector<Token> tokens;

    void setEnableStyling(bool enable);
    bool getEnableStyling() const;

    ~Highlighter() override
    {
        delete analyzer;
    }

protected:
    void highlightBlock(const QString& text) override;

private:

    void setDarkThemeFormats();
    bool enableStyling = false;

    Tokenizer* analyzer;

    static int searchFor(const std::string& text, const std::string& substr, int startIndex);

    QTextCharFormat defaultFormat;
    QTextCharFormat errorFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat parenthesisFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat syntaxErrorFormat;
};
