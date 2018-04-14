#include "statement.h"
#include "state.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

void checkStatements(const State& state,
    const std::vector<StatementPtr>& statements)
{
    for (const auto& stmt : statements) {
        stmt->check(state);
    }
}

void optimizeStatements(State& state,
    std::vector<StatementPtr>& statements)
{
    statements.erase(std::remove_if(statements.begin(), statements.end(),
        [&](const StatementPtr& stmt) {
            return stmt->getConstantLevel(state) >= ConstantLevel::CONSTANT;
        }
    ), statements.end());
    for (auto& stmt : statements) {
        stmt->optimize(state);
    }
}

ConstantLevel getStatementsConstantLevel(const State& state,
    const std::vector<StatementPtr>& statements)
{
    ConstantLevel ret = ConstantLevel::CONSTANT;
    for (const auto& stmt : statements) {
        ret = std::min(ret, stmt->getConstantLevel(state));
        if (ret == ConstantLevel::GLOBAL) {
            break;
        }
    }
    return ret;
}

Statement::Statement(const DebugInfo& info) : Debuggable(info) {}

StatementAssign::StatementAssign(const DebugInfo& info,
    std::vector<size_t>&& vars,
    std::vector<ExpressionPtr>&& expressions)
: Statement(info)
, m_variables(std::move(vars))
, m_expressions(std::move(expressions)) {}

void StatementAssign::resolve(State& state) const
{
    for (const auto& expr : m_expressions) {
        expr->resolve(state);
    }
    for (auto iter = m_variables.rbegin(); iter != m_variables.rend(); ++iter) {
        bool value = state.pop();
        if (*iter != ignorePosition) {
            state.setVar(*iter, value);
        }
    }
}

void StatementAssign::check(const State& state) const
{
    checkExpressions(state, m_expressions);
    if (m_variables.size() != countOutputs(state, m_expressions)) {
        std::stringstream s;
        s << "Not enough outputs for assignment.";
        throwError(s.str());
    }
}

ConstantLevel StatementAssign::getConstantLevel(const State& state) const
{
    ConstantLevel ret = ConstantLevel::CONSTANT;
    for (size_t pos : m_variables) {
        if (pos != ignorePosition) {
            ret = ConstantLevel::LOCAL;
            break;
        }
    }
    return std::min(ret, getExpressionsConstantLevel(state, m_expressions));
}

void StatementAssign::optimize(State& state)
{
    optimizeExpressions(state, m_expressions);
}

StatementVariable::StatementVariable(const DebugInfo& info,
    std::vector<size_t>&& vars,
    std::vector<ExpressionPtr>&& expressions)
: Statement(info)
, m_variables(std::move(vars))
, m_expressions(std::move(expressions)) {}

void StatementVariable::resolve(State& state) const
{
    for (auto iter = m_variables.rbegin(); iter != m_variables.rend(); ++iter) {
        if (*iter != ignorePosition) {
            state.push(0);
        }
    }
    for (const auto& expr : m_expressions) {
        expr->resolve(state);
    }
    for (auto iter = m_variables.rbegin(); iter != m_variables.rend(); ++iter) {
        bool value = state.pop();
        if (*iter != ignorePosition) {
            state.setVar(*iter, value);
        }
    }
}

void StatementVariable::check(const State& state) const
{
    checkExpressions(state, m_expressions);
    if (m_variables.size() != countOutputs(state, m_expressions)) {
        std::stringstream s;
        s << "Not enough outputs for variables.";
        throwError(s.str());
    }
}

ConstantLevel StatementVariable::getConstantLevel(const State& state) const
{
    ConstantLevel ret = ConstantLevel::CONSTANT;
    for (size_t pos : m_variables) {
        if (pos != ignorePosition) {
            ret = ConstantLevel::LOCAL;
            break;
        }
    }
    return std::min(ret, getExpressionsConstantLevel(state, m_expressions));
}

void StatementVariable::optimize(State& state)
{
    optimizeExpressions(state, m_expressions);
}

StatementIf::StatementIf(const DebugInfo& info, ExpressionPtr cond,
    std::vector<StatementPtr>&& block,
    std::vector<StatementPtr>&& elseblock)
: Statement(info)
, m_condition(std::move(cond))
, m_block(std::move(block))
, m_else(std::move(elseblock)) {}

void StatementIf::resolve(State& state) const
{
    size_t prev = state.size();
    // check condition
    m_condition->resolve(state);
    if (state.pop()) {
        // call statements
        for (const auto& stmt : m_block) {
            stmt->resolve(state);
        }
    } else {
        // call else statements
        for (const auto& stmt : m_else) {
            stmt->resolve(state);
        }
    }
    state.resize(prev);
}

void StatementIf::check(const State& state) const
{
    m_condition->check(state);
    if (m_condition->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "If statement expects only 1 input.";
        throwError(s.str());
    }
    checkStatements(state, m_block);
    checkStatements(state, m_else);
}

ConstantLevel StatementIf::getConstantLevel(const State& state) const
{
    return std::min({
        getStatementsConstantLevel(state, m_block),
        getStatementsConstantLevel(state, m_else),
        m_condition->getConstantLevel(state)});
}

void StatementIf::optimize(State& state)
{
    if (m_condition->getConstantLevel(state) == ConstantLevel::CONSTANT) {
        m_condition->resolve(state);
        m_condition = std::move(std::make_unique<ExpressionLiteral>(
            m_condition->getDebugInfo(), state.pop()));
    } else {
        m_condition->optimize(state);
    }
    optimizeStatements(state, m_block);
    optimizeStatements(state, m_else);
}

StatementWhile::StatementWhile(const DebugInfo& info, ExpressionPtr cond,
    std::vector<StatementPtr>&& block)
: Statement(info)
, m_condition(std::move(cond))
, m_block(std::move(block)) {}

void StatementWhile::resolve(State& state) const
{
    // same as for if, but in a loop
    size_t prev = state.size();
    for (;;) {
        m_condition->resolve(state);
        if (!state.pop()) {
            // only difference is how the exit condition is handled
            return;
        }
        for (const auto& stmt : m_block) {
            stmt->resolve(state);
        }
    }
    state.resize(prev);
}

void StatementWhile::check(const State& state) const
{
    m_condition->check(state);
    if (m_condition->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "While statement expects only 1 input.";
        throwError(s.str());
    }
    checkStatements(state, m_block);
}

ConstantLevel StatementWhile::getConstantLevel(const State& state) const
{
    return std::min(
        getStatementsConstantLevel(state, m_block),
        m_condition->getConstantLevel(state));
}

void StatementWhile::optimize(State& state)
{
    if (m_condition->getConstantLevel(state) == ConstantLevel::CONSTANT) {
        m_condition->resolve(state);
        m_condition = std::move(std::make_unique<ExpressionLiteral>(
            m_condition->getDebugInfo(), state.pop()));
    } else {
        m_condition->optimize(state);
    }
    optimizeStatements(state, m_block);
}

StatementExpression::StatementExpression(
    ExpressionPtr&& expr)
: Statement(expr->getDebugInfo())
, m_expression(std::move(expr)) {}

void StatementExpression::resolve(State& state) const
{
    m_expression->resolve(state);
}

void StatementExpression::check(const State& state) const
{
    m_expression->check(state);
    if (m_expression->getOutputNum(state) > 0) {
        std::stringstream s;
        s << "Unhandled outputs from expression.";
        throwError(s.str());
    }
}

ConstantLevel StatementExpression::getConstantLevel(const State& state) const
{
    return m_expression->getConstantLevel(state);
}

void StatementExpression::optimize(State& state)
{
    m_expression->optimize(state);
}
