#include "statement.h"
#include "state.h"
#include <stdexcept>
#include <sstream>

void checkStatements(const State& state,
    const std::vector<StatementPtr>& statements)
{
    for (const auto& stmt : statements) {
        stmt->check(state);
    }
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
