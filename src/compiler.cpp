#include "compiler.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <functional>
#include <cmath>

/// Returns the number of bits required to store the given value.
size_t getRequiredBits(size_t value)
{
    if (value == 0) {
        return 1;
    } else {
        return std::log2(value) + 1;
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
        std::stringstream s;
        s << "Expected " << at;
        throwErrorNoInfo(s.str());
    }
    TokenTaker ret;
    while (tokens) {
        Token t = tokens.pop();
        if (!tokens && t.getSymbol() != at) {
            // error if reaching end of tokens but haven't found symbol
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
        if (tokens.peek() == Symbol::INDEX) {
            Token tokenIndex = tokens.pop();
            names.insertIndexed(t, tokenIndex.getValue());
        } else {
            names.insert(t);
        }
        assertEmpty(tokens);
    });
    size_t num_inputs = names.size();
    splitMultiple(otaker, Symbol::COMMA, false, [&names](auto tokens) {
        // outputs are also comma delimited
        assertNotEmpty(tokens, "identifier");
        Token t = tokens.pop();
        assertToken(t, Symbol::IDENTIFIER);
        if (tokens.peek() == Symbol::INDEX) {
            Token tokenIndex = tokens.pop();
            names.insertIndexed(t, tokenIndex.getValue());
        } else {
            names.insert(t);
        }
        assertEmpty(tokens);
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

/// Parse a function expression
ExpressionPtr parseExpressionFunction(
    const Token& token_function, TokenTaker& tokens, NameStack& names)
{
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
    return std::make_unique<ExpressionFunction>(
        token_function.getDebugInfo(), token_function.getIdentifier(),
        std::move(values));
}

/// Parse an identifier expression
ExpressionPtr parseExpressionIdentifier(
    const Token& token_id, TokenTaker& tokens, NameStack& names)
{
    NameStackDef def;
    if (tokens.peek() == Symbol::INDEX) {
        Token tokenIndex = tokens.pop();
        def = names.getPositionIndexed(token_id, tokenIndex.getValue());
    } else {
        def = names.getPosition(token_id);
    }
    if (def.pos == ignorePosition) {
        std::stringstream s;
        s << "Attempt to get value of ignored variable";
        token_id.throwError(s.str());
    }
    if (def.size == 1) {
        return std::make_unique<ExpressionVariable>(
            token_id.getDebugInfo(), def.pos);
    } else {
        return std::make_unique<ExpressionArray>(
            token_id.getDebugInfo(), def.pos, def.size);
    }
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
        assertEmpty(left_taker);
    } else if (first == Symbol::IDENTIFIER) {
        // an identifier is either a function call or a variable
        Token t = tokens.pop();
        auto next = tokens.peek();
        if (next == Symbol::PARENTHESIS) {
            left = parseExpressionFunction(t, tokens, names);
        } else {
            left = parseExpressionIdentifier(t, tokens, names);
        }
    } else if (first == Symbol::LITERAL) {
        // literal value
        Token t = tokens.pop();
        size_t bits_max = 1;
        size_t bits_required = getRequiredBits(t.getValue());
        if (tokens.peek() == Symbol::INDEX) {
            bits_max = tokens.pop().getValue();
        }
        if (bits_required > bits_max) {
            std::stringstream s;
            s << "Value " << t.getValue() << " is too big to fit in "
              << bits_max << " bits";
            t.throwError(s.str());
        }
        if (bits_max == 1) {
            left = std::make_unique<ExpressionLiteral>(
                t.getDebugInfo(), t.getValue());
        } else {
            std::vector<bool> arr;
            arr.reserve(bits_max);
            size_t value = t.getValue();
            for (size_t i = 0; i < bits_max; ++i) {
                arr.push_back(value & 0x01);
                value >>= 1;
            }
            left = std::make_unique<ExpressionLiteralArray>(
                t.getDebugInfo(), std::move(arr));
        }
    } else {
        std::stringstream s;
        s << "Unexpected " << first << " in expression";
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

/// Parse a statement based on a condition, e.g. a while loop or if statement
StatementPtr parseStatementConditional(TokenTaker& tokens, NameStack& names)
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
            TokenTaker elseblocktaker(std::move(else_token.takeBlock()));
            NameStack elsesubnames(names);
            elseblock = parseBlock(elseblocktaker, elsesubnames);
            assertEmpty(elseblocktaker);
        }
        return std::make_unique<StatementIf>(
            t.getDebugInfo(), std::move(expr),
            std::move(block), std::move(elseblock));
    }
}

/// Parse a variable assignment or declaration statement
StatementPtr parseStatementAssign(TokenTaker& tokens, NameStack& names)
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
        NameStackDef def;
        bool is_indexed = false;
        size_t index;
        if (tokens.peek() == Symbol::INDEX) {
            Token tokenIndex = tokens.pop();
            index = tokenIndex.getValue();
            is_indexed = true;
        }
        if (is_var) {
            // if is a var, should create name
            if (is_indexed) {
                def = names.insertIndexed(t, index);
            } else {
                def = names.insert(t);
            }
        } else {
            // if not a var, then it should already be defined
            if (is_indexed) {
                def = names.getPositionIndexed(t, index);
            } else {
                def = names.getPosition(t);
            }
        }
        for (size_t i = 0; i < def.size; ++i) {
            size_t pos = def.pos;
            if (pos != ignorePosition) {
                pos += i;
            }
            positions.push_back(pos);
        }
        assertEmpty(tokens);
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
        return parseStatementConditional(tokens, names);
    } else {
        TokenTaker subtokens = takeUntil(tokens, Symbol::LINESEP);
        if (subtokens.contains(Symbol::ASSIGN)) {
            return parseStatementAssign(subtokens, names);
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
