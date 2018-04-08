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

class NameStack {
    NameStack *m_prev;
    std::map<std::string, size_t> m_names;
    /// Insert a variable and return its index
    size_t insert(const std::string&);
    /// Get the position of the given variable
    size_t getPosition(const std::string& pos);
public:
    NameStack();
    NameStack(NameStack&);
    /// Return true if the given name is used by this NameStack
    bool isNameInUse(const std::string&) const;
    /// Return true if the given name is defined by any NameStack
    bool isNameDefined(const std::string&) const;
    /// Insert a token's identifier and return its index
    /// Will check to make sure the given name is not already defined
    size_t insertToken(const Token&);
    /// Get the position of the given variable
    size_t getPositionToken(const Token& pos);
    /// Get the number of elements in this specific NameStack (not parents)
    size_t size();
};

/// Parses a single function from the given tokens. Returns a pair containint
/// the function's name and the function itself.
std::pair<std::string, FunctionPtr> parseFunction(TokenTaker& tokens);

/// Unlike parseStatement, this function does not exhaust the given TokenTaker
/// Make sure to use the assertEmpty function afterwards if need be.
ExpressionPtr parseExpression(TokenTaker& tokens, NameStack& names);

/// Parses a statement. Will exhaust the given TokenTaker.
/// Make sure that TokenTaker is not exhausted before calling this function.
StatementPtr parseStatement(TokenTaker& tokens, NameStack& names);

/// Parse the given TokenTaker as a block of statements. Will exhaust the given
/// TokenTaker.
std::vector<StatementPtr> parseBlock(TokenTaker& tokens, NameStack& names);
