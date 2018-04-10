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
    {"var",      Symbol::VAR},
    {"else",      Symbol::ELSE}
};

const std::map<Symbol, char> symbolBlocks = {
    {Symbol::BLOCK,       '}'},
    {Symbol::PARENTHESIS, ')'}
};

Token::Token(Symbol symbol, const DebugInfo& info)
: Debuggable(info), m_symbol(symbol) {}

Token::Token(Symbol symbol, bool value, const DebugInfo& info)
: Token(symbol, info)
{
    setValue(value);
}

Token::Token(Symbol symbol, std::string id, const DebugInfo& info)
: Token(symbol, info)
{
    setIdentifier(id);
}

Token::Token(Symbol symbol, TokenBlock block, const DebugInfo& info)
: Token(symbol, info)
{
    m_block = block;
}

Token::Token(Symbol symbol, size_t index, const DebugInfo& info)
: Token(symbol, info)
{
    m_index = index;
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

size_t Token::getIndex() const
{
    return m_index;
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

void Token::setIdentifier(const std::string& id)
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
        case Symbol::INDEX:       stream << "index";           break;
        case Symbol::WHILE:       stream << "while";           break;
        case Symbol::ELSE:        stream << "else";            break;
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

void printBlock(const TokenBlock& block)
{
    for (const auto& t : block) {
        switch (t.getSymbol()) {
        case Symbol::BLOCK:
            std::cout << "{" << std::endl;
            printBlock(t.getBlock());
            std::cout << "}" << std::endl;
            break;
        case Symbol::PARENTHESIS:
            std::cout << "(";
            printBlock(t.getBlock());
            std::cout << ")";
            break;
        case Symbol::INDEX:
            std::cout << "[" << t.getIndex() << "]";
            break;
        case Symbol::IDENTIFIER: std::cout << t.getIdentifier() << " "; break;
        case Symbol::LINESEP:    std::cout << ";" << std::endl; break;
        case Symbol::LITERAL:    std::cout << t.getValue();     break;
        case Symbol::FUNCTION:   std::cout << "function "; break;
        case Symbol::WHILE:      std::cout << "while ";    break;
        case Symbol::NONE:       std::cout << "ERROR";     break;
        case Symbol::ELSE:       std::cout << "else";     break;
        case Symbol::VAR:        std::cout << "var ";      break;
        case Symbol::IF:         std::cout << "if ";       break;
        case Symbol::COMMA:      std::cout << ","; break;
        case Symbol::IOSEP:      std::cout << ":"; break;
        case Symbol::ASSIGN:     std::cout << "="; break;
        case Symbol::NAND:       std::cout << "!"; break;
        }
    }
}
