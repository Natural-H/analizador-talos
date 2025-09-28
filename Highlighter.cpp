#include "Highlighter.h"

Highlighter::Highlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    setDarkThemeFormats();
    analyzer = new Tokenizer();
}

void Highlighter::setEnableStyling(const bool enable)
{
    enableStyling = enable;
    rehighlight();
}

bool Highlighter::getEnableStyling() const
{
    return enableStyling;
}

void Highlighter::highlightBlock(const QString& text)
{
    if (!enableStyling)
        return;

    analyzer->setText(text.toStdString());
    for (Token token = analyzer->findNextToken(); token.state != Tokenizer::States::endOfFile; token = analyzer->
         findNextToken())
    {
        if (token.isError() && token.state != 509)
            setFormat(token.index, token.length, errorFormat);
        else if (token.state >= 1000)
            setFormat(token.index, token.length, keywordFormat);
        else if (token.state == Tokenizer::States::entero ||
            token.state == Tokenizer::States::real ||
            token.state == Tokenizer::States::notacionCientifica)
            setFormat(token.index, token.length, numberFormat);
        else if (119 <= token.state && token.state <= 122)
            setFormat(token.index, token.length, parenthesisFormat);
        else if (token.state == Tokenizer::States::constanteCaracter || token.state == Tokenizer::States::constanteString)
            setFormat(token.index, token.length, stringFormat);
        else if (105 <= token.state && token.state <= 130 || token.state == 133)
            setFormat(token.index, token.length, operatorFormat);
        else if (token.state == Tokenizer::States::comentarioDeLinea)
            setFormat(token.index, token.length, commentFormat);
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = searchFor(text.toStdString(), "/*", 0);

    const int comment = searchFor(text.toStdString(), "//", 0);

    if (comment != -1 && comment < startIndex)
        return;

    while (startIndex >= 0)
    {
        const auto endIndex = searchFor(text.toStdString(), "*/", startIndex);
        long long commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(currentBlockState() | 1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + 2;
        }
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = searchFor(text.toStdString(), "/*", startIndex + commentLength);
    }
}

int Highlighter::searchFor(const std::string& text, const std::string& substr, const int startIndex = 0)
{
    if (substr.length() > text.length())
        return -1;

    for (int i = startIndex; i < text.length(); ++i)
    {
        if (substr.at(0) == text.at(i))
        {
            for (int j = 1; j < substr.length() && (i + j) < text.length(); ++j)
            {
                if (text.at(i + j) != substr.at(j))
                    break;

                if (j == substr.length() - 1)
                    return i;
            }
        }
    }

    return -1;
}

void Highlighter::setDarkThemeFormats()
{
    defaultFormat.setForeground(Qt::white);

    errorFormat.setForeground(Qt::red);
    errorFormat.setFontUnderline(true);
    errorFormat.setUnderlineColor(Qt::red);

    commentFormat.setForeground(Qt::lightGray);

    keywordFormat.setForeground(QColor::fromRgb(0x66B2FF));
    numberFormat.setForeground(QColor::fromRgb(0xFF9933));
    parenthesisFormat.setForeground(Qt::gray);
    operatorFormat.setForeground(QColor::fromRgb(0x009999));
    stringFormat.setForeground(QColor::fromRgb(0x00CC00));

    syntaxErrorFormat.setForeground(QColor::fromRgb(0xFDA172));
    syntaxErrorFormat.setFontUnderline(true);
    syntaxErrorFormat.setUnderlineColor(QColor::fromRgb(0xFDA172));
}
