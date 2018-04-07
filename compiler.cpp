#include "compiler.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <functional>

void foobar(State&) {}

TokenTaker::TokenTaker(TokenBlock&& block)
: m_tokens(block) {}

TokenTaker::TokenTaker() {}

void TokenTaker::push(Token&& token)
{
    m_tokens.push_back(std::move(token));
}

void TokenTaker::clear()
{
    m_tokens.clear();
}

Token TokenTaker::pop()
{
    if (*this) {
        Token ret = m_tokens.front();
        m_tokens.pop_front();
        return ret;
    } else {
        throw std::runtime_error("Attempt to pop from an empty TokenTaker");
    }
}

Symbol TokenTaker::peek() const
{
    if (*this) {
        return m_tokens.front().getSymbol();
    } else {
        return Symbol::NONE;
    }
}

const Token& TokenTaker::front() const
{
    if (*this) {
        return m_tokens.front();
    } else {
        throw std::runtime_error("Attempt to peek into an empty TokenTaker");
    }
}

const Token& TokenTaker::back() const
{
    if (*this) {
        return m_tokens.back();
    } else {
        throw std::runtime_error("Attempt to peek into an empty TokenTaker");
    }
}

bool TokenTaker::contains(Symbol s) const
{
    for (const auto& token : m_tokens) {
        if (s == token.getSymbol()) {
            return true;
        }
    }
    return false;
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
        token.throwError(s.str());
    }
}

/// Makes sure that the given TokenTaker is empty.
void assertEmpty(const TokenTaker& tokens)
{
    if (tokens) {
        std::stringstream s;
        const Token& t = tokens.front();
        s << "Unexpected " << t;
        t.throwError(s.str());
    }
}

/// Makes sure that the given TokenTaker is not empty.
/// Will throw an InfolessError, make sure that these errors are caught and
/// passed on in order to give them debug information.
void assertNotEmpty(
    const TokenTaker& tokens, const std::string& expected)
{
    if (!tokens) {
        std::stringstream s;
        s << "Expected " << expected;
        throwErrorNoInfo(s.str());
    }
}

/// Split a TokenTaker into two TokenTakers at the given symbol.
/// This will exhaust the given TokenTaker.
/// If no split symbol is found, then the second return will be empty.
/// The third parameter is a pointer to a DebugInfo that will be assigned to the
/// splitting token's debug info.
std::pair<TokenTaker, TokenTaker> splitAt(
    TokenTaker&& tokens, Symbol at, DebugInfo *info)
{
    TokenTaker left;
    TokenTaker right;
    while (tokens) {
        Token t = tokens.pop();
        if (t.getSymbol() == at) {
            if (info) {
                *info = t.getDebugInfo();
            }
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
/// If require_end is true, then the last symbol must be the delimiter.
void splitMultiple(TokenTaker& tokens, Symbol at, bool require_end,
    std::function<void(TokenTaker)> func)
{
    std::vector<TokenTaker> ret{};
    TokenTaker taker;
    while (tokens) {
        Token t = tokens.pop();
        if (t.getSymbol() != at) {
            taker.push(std::move(t));
        }
        if (t.getSymbol() == at || (!tokens && taker)) {
            try {
                func(taker);
            } catch (InfolessError& e) {
                t.throwError(e.what());
            }
            taker.clear();
        }
        if (t.getSymbol() != at && require_end && !tokens) {
            std::stringstream s;
            s << "Expected " << at;
            t.throwError(s.str());
        }
    }
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
    // Semicolons are inserted by the parser after all blocks. Programmers
    // will not actually be inserting semicolons after functions.
    Token token_linesep   = tokens.pop();
    // check tokens
    assertToken(token_function, Symbol::FUNCTION);
    assertToken(token_fname, Symbol::IDENTIFIER);
    assertToken(token_arguments, Symbol::PARENTHESIS);
    assertToken(token_block, Symbol::BLOCK);
    assertToken(token_linesep, Symbol::LINESEP);
    // parse tokens
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    TokenTaker itaker;
    TokenTaker otaker;
    TokenTaker argtaker(std::move(token_arguments.takeBlock()));
    // inputs are separated from outputs by the :  symbol (IOSEP)
    // The : symbol is not required. If it is not there, then all paramters are
    // treated as inputs
    std::tie(itaker, otaker) = splitAt(
        std::move(argtaker), Symbol::IOSEP, nullptr);
    splitMultiple(itaker, Symbol::COMMA, false, [&inputs](auto tokens) {
        // inputs are comma delimited
        assertNotEmpty(tokens, "identifier");
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        inputs.push_back(t.getIdentifier());
    });
    splitMultiple(otaker, Symbol::COMMA, false, [&outputs](auto tokens) {
        // outputs are also comma delimited
        assertNotEmpty(tokens, "identifier");
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        outputs.push_back(t.getIdentifier());
    });
    // parse statements
    TokenTaker blocktaker(std::move(token_block.takeBlock()));
    std::vector<StatementPtr> block = parseBlock(blocktaker);
    // return
    return std::pair<std::string, FunctionPtr>(token_fname.getIdentifier(),
        std::make_unique<FunctionInternal>(std::move(inputs),
        std::move(outputs), std::move(block)));
}

/// Unlike parseStatement, this function does not exhaust the given TokenTaker
/// Make sure to use the assertEmpty function afterwards if need be.
ExpressionPtr parseExpression(TokenTaker& tokens)
{
    auto first = tokens.peek();
    ExpressionPtr left = nullptr;
    if (first == Symbol::PARENTHESIS) {
        // parenthesis, parse inside
        Token t = tokens.pop();
        TokenTaker left_taker(std::move(t.takeBlock()));
        try {
            assertNotEmpty(left_taker, "expression inside parentheses");
        } catch (InfolessError& e) {
            throwError(t.getDebugInfo(), e.what());
        }
        left = parseExpression(left_taker);
    } else if (first == Symbol::IDENTIFIER) {
        // an identifier is either a function call or a variable
        Token t = tokens.pop();
        auto next = tokens.peek();
        if (next == Symbol::PARENTHESIS) {
            Token args = tokens.pop();
            // its a function call, parse inside of parenthesis as parameters
            TokenTaker param_taker(std::move(args.takeBlock()));
            std::vector<ExpressionPtr> values;
            splitMultiple(param_taker, Symbol::COMMA, false,
            [&values](auto tokens) {
                assertNotEmpty(tokens, "expression before comma");
                // expressions are comma delimited
                values.push_back(parseExpression(tokens));
                assertEmpty(tokens);
            });
            left = std::make_unique<ExpressionFunction>(
                t.getDebugInfo(), t.getIdentifier(), std::move(values));
        } else {
            // its a variable
            left = std::make_unique<ExpressionVariable>(
                t.getDebugInfo(), t.getIdentifier());
        }
    } else if (first == Symbol::LITERAL) {
        Token t = tokens.pop();
        left = std::make_unique<ExpressionLiteral>(
            t.getDebugInfo(), t.getValue());
    } else {
        std::stringstream s;
        s << "Unexpected " << first << " in expression.";
        tokens.front().throwError(s.str());
    }
    auto second = tokens.peek();
    if (second == Symbol::NAND) {
        // If a NAND operator follows the left expression, then parse the
        // expression to the right of the NAND symbol and create a NAND
        // expression.
        Token t = tokens.pop();
        try {
            assertNotEmpty(tokens, "expression after NAND operator");
        } catch (InfolessError& e) {
            throwError(t.getDebugInfo(), e.what());
        }
        ExpressionPtr right = parseExpression(tokens);
        return std::make_unique<ExpressionNand>(
            t.getDebugInfo(), std::move(left), std::move(right));
    } else {
        return left;
    }
}

/// Parse the given TokenTaker as a block of statements. Will exhaust the given
/// TokenTaker.
std::vector<StatementPtr> parseBlock(TokenTaker& tokens)
{
    std::vector<StatementPtr> block;
    splitMultiple(tokens, Symbol::LINESEP, true, [&block](auto tokens) {
        assertNotEmpty(tokens, "statement before semicolon");
        block.push_back(parseStatement(tokens));
    });
    return block;
}

/// Parses a statement. Will exhaust the given TokenTaker.
/// Make sure that TokenTaker is not exhausted before calling this function.
StatementPtr parseStatement(TokenTaker& tokens)
{
    auto first = tokens.peek();
    if (first == Symbol::WHILE || first == Symbol::IF) {
        // if or while
        bool is_while = first == Symbol::WHILE;
        Token t = tokens.pop();
        // condition
        try {
            assertNotEmpty(tokens, "condition after IF statement");
        } catch (InfolessError& e) {
            throwError(t.getDebugInfo(), e.what());
        }
        ExpressionPtr expr = parseExpression(tokens);
        // block
        Token token_block = tokens.pop();
        assertToken(token_block, Symbol::BLOCK);
        TokenTaker blocktaker(std::move(token_block.takeBlock()));
        std::vector<StatementPtr> block = parseBlock(blocktaker);
        assertEmpty(tokens);
        // return
        if (is_while) {
            return std::make_unique<StatementWhile>(
                t.getDebugInfo(), std::move(expr), std::move(block));
        } else {
            return std::make_unique<StatementIf>(
                t.getDebugInfo(), std::move(expr), std::move(block));
        }
    } else if (tokens.contains(Symbol::ASSIGN)) {
        // assignment
        bool is_var = false;
        if (first == Symbol::VAR) {
            is_var = true;
            tokens.pop();
        }
        std::vector<std::string> names;
        std::vector<ExpressionPtr> values;
        // split tokens at assignment symbol
        TokenTaker nametaker;
        TokenTaker valuetaker;
        DebugInfo info;
        std::tie(nametaker, valuetaker) = splitAt(
            std::move(tokens), Symbol::ASSIGN, &info);
        splitMultiple(nametaker, Symbol::COMMA, false, [&names](auto tokens) {
            // expressions are comma delimited
            assertNotEmpty(tokens, "identifier before comma");
            Token t = tokens.pop();
            assertToken(t, Symbol::IDENTIFIER);
            assertEmpty(tokens);
            names.push_back(t.getIdentifier());
        });
        splitMultiple(valuetaker, Symbol::COMMA, false, [&values](auto tokens) {
            // expressions are comma delimited
            assertNotEmpty(tokens, "expression after assignment");
            values.push_back(parseExpression(tokens));
            assertEmpty(tokens);
        });
        // return
        if (is_var) {
            return std::make_unique<StatementVariable>(
                info, std::move(names), std::move(values));
        } else {
            return std::make_unique<StatementAssign>(
                info, std::move(names), std::move(values));
        }
    } else {
        auto expr = parseExpression(tokens);
        assertEmpty(tokens);
        return std::make_unique<StatementExpression>(std::move(expr));
    }
}
