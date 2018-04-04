#include "compiler.h"
#include <iostream>
#include <stdexcept>
#include <sstream>

void foobar(State&) {}

TokenTaker::TokenTaker(TokenBlock&& block)
: m_tokens(block) {}

TokenTaker::TokenTaker() {}

void TokenTaker::push(Token&& token)
{
    m_tokens.push_back(std::move(token));
}

Token TokenTaker::pop()
{
    Token ret = m_tokens.front();
    m_tokens.pop_front();
    return ret;
}

const Token& TokenTaker::peek() const
{
    return m_tokens.front();
}

bool TokenTaker::empty() const
{
    return m_tokens.empty();
}

TokenTaker::operator bool() const
{
    return !empty();
}

/// Makes sure that the given token has the given symbol.
void assertToken(Token& token, Symbol expected)
{
    if (token.getSymbol() != expected) {
        std::stringstream s;
        s << "Unexpected " << token << ", expected " << expected;
        throw std::runtime_error(s.str());
    }
}

/// Makes sure that the given token has the given symbol.
void assertEmpty(const TokenTaker& tokens)
{
    if (tokens) {
        std::stringstream s;
        s << "Unexpected " << tokens.peek();
        throw std::runtime_error(s.str());
    }
}

/// Split a TokenTaker into two TokenTakers at the given symbol.
/// This will exhaust the given TokenTaker.
std::pair<TokenTaker, TokenTaker> splitAt(TokenTaker&& tokens, Symbol at)
{
    TokenTaker left;
    TokenTaker right;
    while (tokens) {
        Token t = tokens.pop();
        if (t.getSymbol() == at) {
            break;
        } else {
            left.push(std::move(t));
        }
    }
    while (tokens) {
        Token t = tokens.pop();
        right.push(std::move(t));
    }
    return std::pair<TokenTaker, TokenTaker>(left, right);
}

/// Split a TokenTaker into multiple TokenTakers at the given symbol.
/// This will exhaust the given TokenTaker.
std::vector<TokenTaker> splitMultiple(TokenTaker& tokens, Symbol at)
{
    std::vector<TokenTaker> ret(TokenTaker{});
    while (tokens) {
        Token t = tokens.pop();
        if (t.getSymbol() == at) {
            ret.push_back(TokenTaker{});
        } else {
            ret.back().push(std::move(t));
        }
    }
    if (!ret.back()) {
        ret.pop_back();
    }
    return ret;
}

/// Parses a single function from the given tokens. Returns a pair containint
/// the function's name and the function itself.
std::pair<std::string, FunctionPtr> parseFunction(TokenTaker& tokens)
{
    // get tokens
    Token token_function  = tokens.pop();
    Token token_fname     = tokens.pop();
    Token token_arguments = tokens.pop();
    Token token_block     = tokens.pop();
    // check tokens
    assertToken(token_function, Symbol::FUNCTION);
    assertToken(token_fname, Symbol::IDENTIFIER);
    assertToken(token_arguments, Symbol::PARENTHESIS);
    assertToken(token_block, Symbol::BLOCK);
    // TODO: actually parse function
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<StatementPtr> block;

    TokenTaker itaker;
    TokenTaker otaker;
    TokenTaker argtaker = TokenTaker(std::move(token_arguments.getBlockMut()));
    std::tie(itaker, otaker) = splitAt(std::move(argtaker), Symbol::IOSEP);
    for (auto tokens : splitMultiple(itaker, Symbol::COMMA)) {
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        inputs.push_back(t.getIdentifier());
    }
    for (auto tokens : splitMultiple(otaker, Symbol::COMMA)) {
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        outputs.push_back(t.getIdentifier());
    }

    return std::pair<std::string, FunctionPtr>(token_fname.getIdentifier(),
        std::make_unique<FunctionInternal>(std::move(inputs),
        std::move(outputs), std::move(block)));
}

ExpressionPtr parseExpression(TokenTaker& tokens)
{

}

ExpressionPtr parseStatement(TokenTaker& tokens)
{

}
