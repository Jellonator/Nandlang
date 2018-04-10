#pragma once
#include <tuple>
#include "function.h"
#include "symbol.h"
#include "tokentaker.h"
#include "namestack.h"

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
