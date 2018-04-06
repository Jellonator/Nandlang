#include "symbol.h"

const std::map<char, Symbol> symbolMap = {
    {',', Symbol::COMMA},
    {'{', Symbol::BLOCK},
    {':', Symbol::IOSEP},
    {'(', Symbol::PARENTHESIS},
    {'=', Symbol::ASSIGN},
    {';', Symbol::LINESEP},
    {'!', Symbol::NAND}
};

const std::map<std::string, Symbol> keywordMap = {
    {"function", Symbol::FUNCTION},
    {"while",    Symbol::WHILE},
    {"if",       Symbol::IF},
    {"var",      Symbol::VAR}
};

const std::map<Symbol, char> symbolBlocks = {
    {Symbol::BLOCK,       '}'},
    {Symbol::PARENTHESIS, ')'}
};

Token::Token(Symbol symbol) : m_symbol(symbol) {}

Token::Token(Symbol symbol, bool value) : Token(symbol)
{
    setValue(value);
}

Token::Token(Symbol symbol, std::string id) : Token(symbol)
{
    setIdentifier(id);
}

Token::Token(Symbol symbol, TokenBlock block) : Token(symbol)
{
    m_block = block;
}

Symbol Token::getSymbol() const
{
    return m_symbol;
}

const std::string& Token::getIdentifier() const
{
    return m_identifier;
}

bool Token::getValue() const
{
    return m_value;
}

const TokenBlock& Token::getBlock() const
{
    return m_block;
}

TokenBlock Token::takeBlock()
{
    TokenBlock replacement;
    std::swap(m_block, replacement);
    return replacement;
}

void Token::setIdentifier(std::string id)
{
    m_identifier = id;
}

void Token::setValue(bool value)
{
    m_value = value;
}

std::ostream& operator<<(std::ostream& stream, const Symbol& symbol)
{
    switch (symbol) {
        case Symbol::LITERAL:     stream << "boolean literal"; break;
        case Symbol::PARENTHESIS: stream << "parameter list";  break;
        case Symbol::NAND:        stream << "nand operator";   break;
        case Symbol::ASSIGN:      stream << "assignment";      break;
        case Symbol::IDENTIFIER:  stream << "identifier";      break;
        case Symbol::LINESEP:     stream << "semicolon";       break;
        case Symbol::FUNCTION:    stream << "function";        break;
        case Symbol::BLOCK:       stream << "block";           break;
        case Symbol::IOSEP:       stream << "colon";           break;
        case Symbol::COMMA:       stream << "comma";           break;
        case Symbol::NONE:        stream << "ERROR";           break;
        case Symbol::WHILE:       stream << "while";           break;
        case Symbol::VAR:         stream << "var";             break;
        case Symbol::IF:          stream << "if";              break;
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const Token& token)
{
    switch (token.getSymbol()) {
        case Symbol::IDENTIFIER:
            stream << "identifier \"" << token.getIdentifier() << "\"";
            break;
        case Symbol::LITERAL:
            stream << "literal value " << token.getValue();
            break;
        default:
            stream << token.getSymbol();
            break;
    }
    return stream;
}
