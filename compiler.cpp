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

Symbol TokenTaker::peek() const
{
    if (*this) {
        return m_tokens.front().getSymbol();
    } else {
        return Symbol::NONE;
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
/// If require_end is true, then the last symbol must be the delimiter.
std::vector<TokenTaker> splitMultiple(TokenTaker& tokens, Symbol at, bool require_end)
{
    std::vector<TokenTaker> ret{TokenTaker{}};
    while (tokens) {
        Token t = tokens.pop();
        if (t.getSymbol() == at) {
            ret.emplace_back();
        } else {
            ret.back().push(std::move(t));
        }
    }
    if (!ret.back()) {
        ret.pop_back();
    } else if (require_end) {
        std::stringstream s;
        s << "Expected semicolon";
        throw std::runtime_error(s.str());
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
    // parse tokens
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    TokenTaker itaker;
    TokenTaker otaker;
    TokenTaker argtaker(std::move(token_arguments.takeBlock()));
    // inputs are separated from outputs by the :  symbol (IOSEP)
    // The : symbol is not required. If it is not there, then all paramters are
    // treated as inputs
    std::tie(itaker, otaker) = splitAt(std::move(argtaker), Symbol::IOSEP);
    for (auto& tokens : splitMultiple(itaker, Symbol::COMMA, false)) {
        // inputs are comma delimited
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        inputs.push_back(t.getIdentifier());
    }
    for (auto& tokens : splitMultiple(otaker, Symbol::COMMA, false)) {
        // outputs are also comma delimited
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        outputs.push_back(t.getIdentifier());
    }
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
            for (auto& other : splitMultiple(param_taker, Symbol::COMMA, false)) {
                // expressions are comma delimited
                values.push_back(parseExpression(other));
                assertEmpty(other);
            }
            left = std::make_unique<ExpressionFunction>(
                t.getIdentifier(), std::move(values));
        } else {
            // its a variable
            left = std::make_unique<ExpressionVariable>(t.getIdentifier());
        }
    } else if (first == Symbol::LITERAL) {
        Token t = tokens.pop();
        left = std::make_unique<ExpressionLiteral>(t.getValue());
    } else {
        std::stringstream s;
        s << "Expected " << first << " in expression.";
        throw std::runtime_error(s.str());
    }
    auto second = tokens.peek();
    if (second == Symbol::NAND) {
        // If a NAND operator follows the left expression, then parse the
        // expression to the right of the NAND symbol and create a NAND
        // expression.
        tokens.pop();
        ExpressionPtr right = parseExpression(tokens);
        return std::make_unique<ExpressionNand>(
            std::move(left), std::move(right));
    } else {
        return left;
    }
}

/// Parse the given TokenTaker as a block of statements. Will exhaust the given
/// TokenTaker.
std::vector<StatementPtr> parseBlock(TokenTaker& tokens)
{
    std::vector<StatementPtr> block;
    for (auto& tokens : splitMultiple(tokens, Symbol::LINESEP, true)) {
        while (tokens) {
            block.push_back(parseStatement(tokens));
        }
    }
    return block;
}

/// Parses a statement. Will usually exhaust the given TokenTaker, but there
/// are cases where it will not, such as after an if or while statement.
/// Care should be taken to handle this case.
StatementPtr parseStatement(TokenTaker& tokens)
{
    auto first = tokens.peek();
    if (first == Symbol::WHILE || first == Symbol::IF) {
        // if or while
        bool is_while = first == Symbol::WHILE;
        tokens.pop();
        // condition
        ExpressionPtr expr = parseExpression(tokens);
        // block
        Token token_block = tokens.pop();
        assertToken(token_block, Symbol::BLOCK);
        TokenTaker blocktaker(std::move(token_block.takeBlock()));
        std::vector<StatementPtr> block = parseBlock(blocktaker);
        // return
        if (is_while) {
            return std::make_unique<StatementWhile>(
                std::move(expr), std::move(block));
        } else {
            return std::make_unique<StatementIf>(
                std::move(expr), std::move(block));
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
        std::tie(nametaker, valuetaker) = splitAt(
            std::move(tokens), Symbol::ASSIGN);
        for (auto& tokens : splitMultiple(nametaker, Symbol::COMMA, false)) {
            // expressions are comma delimited
            Token t = tokens.pop();
            assertToken(t, Symbol::IDENTIFIER);
            assertEmpty(tokens);
            names.push_back(t.getIdentifier());
        }
        for (auto& tokens : splitMultiple(valuetaker, Symbol::COMMA, false)) {
            // expressions are comma delimited
            values.push_back(parseExpression(tokens));
            assertEmpty(tokens);
        }
        // return
        if (is_var) {
            return std::make_unique<StatementVariable>(
                std::move(names), std::move(values));
        } else {
            return std::make_unique<StatementAssign>(
                std::move(names), std::move(values));
        }
    } else {
        auto expr = parseExpression(tokens);
        assertEmpty(tokens);
        return std::make_unique<StatementExpression>(std::move(expr));
    }
}
