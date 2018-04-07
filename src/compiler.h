#pragma once
#include <tuple>
#include "function.h"
#include "symbol.h"

/// A class that allows for Tokens to be taken from a TokenBlock in FIFO order,
/// as well as some other useful functions.
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

/// Parses a single function from the given tokens. Returns a pair containint
/// the function's name and the function itself.
std::pair<std::string, FunctionPtr> parseFunction(TokenTaker& tokens);

/// Unlike parseStatement, this function does not exhaust the given TokenTaker
/// Make sure to use the assertEmpty function afterwards if need be.
ExpressionPtr parseExpression(TokenTaker& tokens);

/// Parses a statement. Will exhaust the given TokenTaker.
/// Make sure that TokenTaker is not exhausted before calling this function.
StatementPtr parseStatement(TokenTaker& tokens);

/// Parse the given TokenTaker as a block of statements. Will exhaust the given
/// TokenTaker.
std::vector<StatementPtr> parseBlock(TokenTaker& tokens);
