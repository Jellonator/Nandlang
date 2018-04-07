#include "parse.h"
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <limits>
#include <iomanip>

const char UNDERSCORE = '_';
const char SQUOTE = '\'';
const char ESCAPE = '\\';

/// Maps character escape codes to their representations
const std::map<char, char> ESCAPE_LOOKUP = {
    {'0', '\0'},
    {'t', '\t'},
    {'n', '\n'},
    {'v', '\v'},
    {'f', '\f'},
    {'r', '\f'},
};

/// Returns true if the given character is allowed to be used for an identifier
bool isCharacterAllowed(char c)
{
    return std::isalnum(c) || c == UNDERSCORE;
}

/// Get the actual char value from a char string
/// Given string should NOT be surrounded in single quotes
char getCharLiteralFromString(const std::string& id)
{
    // can't be valid if it is empty
    if (id.size() < 1) {
        return 0;
    }
    // Test if first character is escape sequence
    bool is_escape = id[0] == ESCAPE;
    if (is_escape) {
        // Escape sequence must be two characters
        if (id.size() != 2) {
            return 0;
        }
        // Lookup in escape table
        if (ESCAPE_LOOKUP.count(id[1])) {
            return ESCAPE_LOOKUP.at(id[1]);
        } else {
            return 0;
        }
    } else {
        // Return character as literal
        if (id.size() != 1) {
            return 0;
        }
        return id[0];
    }
}

/// Return true if the given string is a valid identifier
/// This means all characters in the string are alphanumeric or underscores, and
/// does not begin with a digit.
bool isStringIdentifier(const std::string& id)
{
    for (auto c : id) {
        if (!std::isalnum(c) && c != UNDERSCORE) {
            return false;
        }
    }
    return (id.size() > 0 && !std::isdigit(id[0]));
}

/// Append the given identifier to the token block. The given identifier may
/// be transformed into other token types, e.g. Symbol::WHILE if it matches a
/// keyword, or a literal if it is a boolean literal.
void appendIdentifier(TokenBlock& block, std::string& id, DebugInfo info)
{
    if (id.empty()) {
        return;
    }
    if (id == "0") {
        block.push_back(Token(Symbol::LITERAL, false, info));
    } else if (id == "1") {
        block.push_back(Token(Symbol::LITERAL, true, info));
    } else if (keywordMap.count(id)) {
        block.push_back(Token(keywordMap.at(id), info));
    } else if (isStringIdentifier(id)) {
        block.push_back(Token(Symbol::IDENTIFIER, id, info));
    } else {
        std::stringstream s;
        s << "Bad identifier " << id;
        throwError(info, s.str());
    }
    id = "";
}

/// parse a character literal, e.g. 'a', from the given stream.
void parseChar(std::istream& stream, TokenBlock& block, DebugInfo& context)
{
    char c;
    std::string charstring;
    const DebugInfo info = context;
    while (stream.get(c)) {
        context.column += 1;
        context.position += 1;
        if (c == SQUOTE) {
            break;
        }
        if (std::isprint(c)) {
            // isprint allows all text characters and space
            charstring += c;
        } else {
            std::stringstream s;
            s << "Unexpected character 0x" << std::hex << std::setw(2)
              << std::setfill('0') << std::uppercase << int(c)
              << " in character literal";
            throwError(info, s.str());
        }
    }
    c = getCharLiteralFromString(charstring);
    if (c) {
        // Character literals are expanded to 8 bool literals
        for (size_t i = 0; i < 8; ++i) {
            // Most significant bit comes first, since this language behaves in
            // a big-endian way.
            bool b = c & 0x80;
            c <<= 1;
            if (i != 0) {
                // only 7 commas need be inserted, so only insert it when the
                // index is not 0
                block.push_back(Token(Symbol::COMMA, info));
            }
            block.push_back(Token(Symbol::LITERAL, b, info));
        }
    } else {
        std::stringstream s;
        s << "Invalid character literal " << charstring;
        throwError(info, s.str());
    }
}

/// Parse tokens until the given ending chracter.
TokenBlock _parseTokens(std::istream& stream, DebugInfo& context, char endc)
{
    TokenBlock block;
    std::string identifier;
    DebugInfo id_info;
    char c;
    // Having three different DebugInfo objects is a little confusing, but there
    // is VERY good reason for doing so. Here they are explained:
    //  * id_info - DebugInfo for the beginning of the identifier
    //  * context - DebugInfo that gets incremented whenever an end of line
    //              character is met and whenever a character is read.
    //  * info    - Per-character information. Since context must be increment
    //              right after every read, context can not be used as it
    //              will actually point to the NEXT character. So, there must be
    //              an info object that points to the current one.
    while (stream.get(c)) {
        const DebugInfo info = context;
        context.column += 1;
        context.position += 1;
        if (c == '/' && stream.peek() == '/') {
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            context.line += 1;
            // column set to 0 since it will be incremented later
            context.column = 1;
        } else if (c == endc) {
            appendIdentifier(block, identifier, id_info);
            return block;
        } else if (c == SQUOTE) {
            appendIdentifier(block, identifier, id_info);
            parseChar(stream, block, context);
        } else if (std::isspace(c)) {
            appendIdentifier(block, identifier, id_info);
            if (c == '\n') {
                context.line += 1;
                // column set to 0 since it will be incremented later
                context.column = 1;
            }
        } else if (symbolMap.count(c)) {
            appendIdentifier(block, identifier, id_info);
            Symbol symbol = symbolMap.at(c);
            if (symbolBlocks.count(symbol)) {
                // Next few characters as the inside of this symbol
                char ending = symbolBlocks.at(symbol);
                TokenBlock inner = _parseTokens(stream, context, ending);
                block.push_back(Token(symbol, inner, info));
                if (symbol == Symbol::BLOCK) {
                    // Line separators are implicitly inserted after all blocks.
                    // This is important for while loops and if statements.
                    block.push_back(Token(Symbol::LINESEP, info));
                }
            } else {
                block.push_back(Token(symbol, info));
            }
        } else {
            if (isCharacterAllowed(c)) {
                if (identifier.empty()) {
                    id_info = context;
                }
                identifier += c;
            } else {
                std::stringstream s;
                s << "Unknown character '" << c << "'";
                throwError(info, s.str());
            }
        }
    }
    if (endc) {
        std::stringstream s;
        s << "Expected " << endc << " before end of file.";
        throwError(context, s.str());
    }
    appendIdentifier(block, identifier, id_info);
    return block;
}

TokenBlock parseTokens(std::istream& stream, DebugInfo info)
{
    return _parseTokens(stream, info, 0);
}
