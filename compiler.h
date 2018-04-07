#pragma once
#include <tuple>
#include "function.h"
#include "symbol.h"

class TokenTaker {
    TokenBlock m_tokens;
public:
    TokenTaker(TokenBlock&& block);
    TokenTaker();
    /// Push a symbol to this taker
    void push(Token&& token);
    /// Clear this TokenTaker's tokens
    void clear();
    /// Get the front-most token
    Token pop();
    /// Get the front-most token's without removing it.
    /// If the TokenTaker is empty, will return Symbol::NONE
    Symbol peek() const;
    /// Get the back-most token without removing it.
    /// Will do bounds checking, use at your own risk.
    const Token& front() const;
    /// Get the back-most token without removing it.
    /// Will do bounds checking, use at your own risk.
    const Token& back() const;
    /// Returns true if this TokenTaker contains the given symbol
    bool contains(Symbol) const;
    /// Returns true if this TokenTaker is out of tokens
    bool empty() const;
    operator bool() const;
};

std::pair<std::string, FunctionPtr> parseFunction(TokenTaker& tokens);
ExpressionPtr parseExpression(TokenTaker& tokens);
StatementPtr parseStatement(TokenTaker& tokens);
std::vector<StatementPtr> parseBlock(TokenTaker& tokens);
