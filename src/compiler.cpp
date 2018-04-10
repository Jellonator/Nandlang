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

NameStack::NameStack()
: m_prev(nullptr) {}

NameStack::NameStack(NameStack& other)
: m_prev(&other) {}

bool NameStack::isNameInUse(const std::string& name) const
{
    return m_names.count(name);
}

bool NameStack::isNameDefined(const std::string& name) const
{
    if (m_names.count(name) || name == ignoreIdentifier) {
        return true;
    } else if (m_prev) {
        return m_prev->isNameDefined(name);
    } else {
        return false;
    }
}

size_t NameStack::insert(const std::string& name)
{
    if (name == ignoreIdentifier) {
        return ignorePosition;
    }
    size_t ret = m_names.size();
    m_names[name] = ret;
    return ret;
}

size_t NameStack::insertToken(const Token& token)
{
    if (isNameInUse(token.getIdentifier())) {
        std::stringstream s;
        s << "Attempt to define already existing variable "
          << token.getIdentifier();
        token.throwError(s.str());
        return 0;
    }
    return insert(token.getIdentifier());
}

size_t NameStack::getPosition(const std::string& pos)
{
    if (m_prev && m_names.count(pos) == 0) {
        return m_prev->getPosition(pos);
    }
    return m_names.at(pos);
}

size_t NameStack::getPositionToken(const Token& token)
{
    if (token.getIdentifier() == ignoreIdentifier) {
        std::stringstream s;
        s << "Attempt to get value of ignored variable";
        token.throwError(s.str());
        return 0;
    }
    if (!isNameDefined(token.getIdentifier())) {
        std::stringstream s;
        s << "Attempt to use undefined variable "
          << token.getIdentifier();
        token.throwError(s.str());
        return 0;
    } else {
        return getPosition(token.getIdentifier());
    }
}

size_t NameStack::size()
{
    size_t ret = m_names.size();
    if (m_prev) {
        return ret + m_prev->size();
    } else {
        return ret;
    }
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
                // assign debug info for center token
                *info = t.getDebugInfo();
            }
            break;
        } else {
            left.push(std::move(t));
        }
    }
    // everything after is now on the right, even if it is the split symbol.
    while (tokens) {
        Token t = tokens.pop();
        right.push(std::move(t));
    }
    return std::pair<TokenTaker, TokenTaker>(left, right);
}

/// Take tokens from the token taker until the given symbol is met
TokenTaker takeUntil(TokenTaker& tokens, Symbol at) {
    if (!tokens) {
        throwErrorNoInfo("Expected symbols");
    }
    TokenTaker ret;
    while (tokens) {
        Token t = tokens.pop();
        if (!tokens && t.getSymbol() != at) {
            std::stringstream s;
            s << "Expected " << at << "; got " << t << " instead";
            t.throwError(s.str());
        }
        if (t.getSymbol() == at) {
            break;
        } else {
            ret.push(std::move(t));
        }
    }
    return ret;
}

/// Split a TokenTaker into multiple TokenTakers at the given symbol.
/// This will exhaust the given TokenTaker.
/// If require_end is true, then the last symbol must be the delimiter.
void splitMultiple(TokenTaker& tokens, Symbol at, bool require_end,
    std::function<void(TokenTaker)> func)
{
    TokenTaker taker;
    while (tokens) {
        Token t = tokens.pop();
        if (t.getSymbol() != at) {
            // add all other symbols to the TokenTaker
            taker.push(std::move(t));
        }
        if (t.getSymbol() == at || (!tokens && taker)) {
            // call the function if the split symbol is found, or if we have
            // ran out of tokens and there are still symbols that need to be
            // parsed.
            try {
                func(taker);
            } catch (InfolessError& e) {
                t.throwError(e.what());
            }
            taker.clear();
        }
        if (t.getSymbol() != at && require_end && !tokens) {
            // There is an error if the split symbol is expected at the end,
            // but the last symbol is not the split symbol.
            std::stringstream s;
            s << "Expected " << at;
            t.throwError(s.str());
        }
    }
}

std::pair<std::string, FunctionPtr> parseFunction(TokenTaker& tokens)
{
    NameStack names;
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
    TokenTaker itaker;
    TokenTaker otaker;
    TokenTaker argtaker(std::move(token_arguments.takeBlock()));
    // inputs are separated from outputs by the :  symbol (IOSEP)
    // The : symbol is not required. If it is not there, then all paramters are
    // treated as inputs
    std::tie(itaker, otaker) = splitAt(
        std::move(argtaker), Symbol::IOSEP, nullptr);
    splitMultiple(itaker, Symbol::COMMA, false, [&names](auto tokens) {
        // inputs are comma delimited
        assertNotEmpty(tokens, "identifier");
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        names.insertToken(t);
    });
    size_t num_inputs = names.size();
    splitMultiple(otaker, Symbol::COMMA, false, [&names](auto tokens) {
        // outputs are also comma delimited
        assertNotEmpty(tokens, "identifier");
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        names.insertToken(t);
    });
    size_t num_outputs = names.size() - num_inputs;
    // parse statements
    TokenTaker blocktaker(std::move(token_block.takeBlock()));
    std::vector<StatementPtr> block = parseBlock(blocktaker, names);
    // return
    return std::pair<std::string, FunctionPtr>(token_fname.getIdentifier(),
        std::make_unique<FunctionInternal>(num_inputs, num_outputs,
        std::move(block)));
}

ExpressionPtr parseExpression(TokenTaker& tokens, NameStack& names)
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
        left = parseExpression(left_taker, names);
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
            [&values, &names](auto tokens) {
                assertNotEmpty(tokens, "expression before comma");
                // expressions are comma delimited
                values.push_back(parseExpression(tokens, names));
                assertEmpty(tokens);
            });
            left = std::make_unique<ExpressionFunction>(
                t.getDebugInfo(), t.getIdentifier(), std::move(values));
        } else {
            // its a variable
            left = std::make_unique<ExpressionVariable>(
                t.getDebugInfo(), names.getPositionToken(t));
        }
    } else if (first == Symbol::LITERAL) {
        // literal value
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
        ExpressionPtr right = parseExpression(tokens, names);
        return std::make_unique<ExpressionNand>(
            t.getDebugInfo(), std::move(left), std::move(right));
    } else {
        return left;
    }
}

StatementPtr parseConditional(TokenTaker& tokens, NameStack& names)
{
    // if or while
    Token t = tokens.pop();
    bool is_while = t.getSymbol() == Symbol::WHILE;
    // condition
    try {
        assertNotEmpty(tokens, "condition after IF statement");
    } catch (InfolessError& e) {
        throwError(t.getDebugInfo(), e.what());
    }
    ExpressionPtr expr = parseExpression(tokens, names);
    // block
    Token token_block = tokens.pop();
    assertToken(token_block, Symbol::BLOCK);
    TokenTaker blocktaker(std::move(token_block.takeBlock()));
    NameStack subnames(names);
    std::vector<StatementPtr> block = parseBlock(blocktaker, subnames);
    // return
    if (is_while) {
        return std::make_unique<StatementWhile>(
            t.getDebugInfo(), std::move(expr), std::move(block));
    } else {
        std::vector<StatementPtr> elseblock;
        if (tokens.peek() == Symbol::ELSE) {
            tokens.pop();
            Token else_token = tokens.pop();
            assertToken(else_token, Symbol::BLOCK);
            TokenTaker elseblocktaker(std::move(token_block.takeBlock()));
            NameStack elsesubnames(names);
            elseblock = parseBlock(elseblocktaker, elsesubnames);
        }
        return std::make_unique<StatementIf>(
            t.getDebugInfo(), std::move(expr),
            std::move(block), std::move(elseblock));
    }
}

StatementPtr parseAssign(TokenTaker& tokens, NameStack& names)
{
    // assignment
    bool is_var = false;
    if (tokens.peek() == Symbol::VAR) {
        is_var = true;
        tokens.pop();
    }
    std::vector<size_t> positions;
    std::vector<ExpressionPtr> values;
    // split tokens at assignment symbol
    TokenTaker nametaker;
    TokenTaker valuetaker;
    DebugInfo info;
    std::tie(nametaker, valuetaker) = splitAt(
        std::move(tokens), Symbol::ASSIGN, &info);
    splitMultiple(nametaker, Symbol::COMMA, false,
        [&positions, &names, is_var](auto tokens) {
        // expressions are comma delimited
        assertNotEmpty(tokens, "identifier before comma");
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        assertEmpty(tokens);
        if (is_var) {
            // if is a var, should create name
            positions.push_back(names.insertToken(t));
        } else {
            // if not a var, then it should already be defined
            positions.push_back(names.getPositionToken(t));
        }
    });
    splitMultiple(valuetaker, Symbol::COMMA, false,
        [&values, &names](auto tokens) {
        // expressions are comma delimited
        assertNotEmpty(tokens, "expression after assignment");
        values.push_back(parseExpression(tokens, names));
        assertEmpty(tokens);
    });
    // return
    if (is_var) {
        return std::make_unique<StatementVariable>(
            info, std::move(positions), std::move(values));
    } else {
        return std::make_unique<StatementAssign>(
            info, std::move(positions), std::move(values));
    }
}

StatementPtr parseStatement(TokenTaker& tokens, NameStack& names)
{
    auto first = tokens.peek();
    if (first == Symbol::WHILE || first == Symbol::IF) {
        // only if and while do not require semicolons
        return parseConditional(tokens, names);
    } else {
        TokenTaker subtokens = takeUntil(tokens, Symbol::LINESEP);
        if (subtokens.contains(Symbol::ASSIGN)) {
            return parseAssign(subtokens, names);
        } else {
            // this is probably an expression. An error will result if it is not.
            auto expr = parseExpression(subtokens, names);
            assertEmpty(subtokens);
            return std::make_unique<StatementExpression>(std::move(expr));
        }
    }
}

std::vector<StatementPtr> parseBlock(TokenTaker& tokens, NameStack& names)
{
    std::vector<StatementPtr> block;
    while (tokens) {
        block.push_back(parseStatement(tokens, names));
    }
    return block;
}
