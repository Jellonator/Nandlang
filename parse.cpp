#include "parse.h"
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <limits>

bool isCharacterAllowed(char c)
{
    return std::isalnum(c) || c == '_';
}

bool isStringIdentifier(const std::string& id)
{
    return (id.size() > 0 && !std::isdigit(id[0]));
}

void appendIdentifier(TokenBlock& block, std::string& id)
{
    if (id.empty()) {
        return;
    }
    if (id == "0") {
        block.push_back(Token(Symbol::LITERAL, false));
    } else if (id == "1") {
        block.push_back(Token(Symbol::LITERAL, true));
    } else if (keywordMap.count(id)) {
        block.push_back(Token(keywordMap.at(id)));
    } else if (isStringIdentifier(id)) {
        block.push_back(Token(Symbol::IDENTIFIER, id));
    } else {
        throw std::runtime_error("Bad identifier.");
    }
    id = "";
}

TokenBlock _parseTokens(std::istream& stream, char endc, int& line)
{
    TokenBlock block;
    std::string identifier;
    char c;
    while (stream.get(c)) {
        if (c == '/' && stream.peek() == '/') {
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            line += 1;
        } else if (c == endc) {
            appendIdentifier(block, identifier);
            return block;
        } else if (std::isspace(c)) {
            appendIdentifier(block, identifier);
            if (c == '\n') {
                line += 1;
            }
        } else if (symbolMap.count(c)) {
            appendIdentifier(block, identifier);
            Symbol symbol = symbolMap.at(c);
            if (symbolBlocks.count(symbol)) {
                char ending = symbolBlocks.at(symbol);
                TokenBlock inner = _parseTokens(stream, ending, line);
                block.push_back(Token(symbol, inner));
            } else {
                block.push_back(Token(symbol));
            }
        } else {
            if (isCharacterAllowed(c)) {
                identifier += c;
            } else {
                std::stringstream s;
                s << "Unknown character: '" << c << "'" << " on line " << line;
                throw std::runtime_error(s.str());
            }
        }
    }
    if (endc) {
        throw std::runtime_error("Expected ending character.");
    }
    appendIdentifier(block, identifier);
    return block;
}

TokenBlock parseTokens(std::istream& stream)
{
    int line = 1;
    return _parseTokens(stream, 0, line);
}
