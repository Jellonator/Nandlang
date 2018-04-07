#include "statement.h"
#include "state.h"
#include <stdexcept>
#include <sstream>

void checkStatements(State& state, const std::vector<StatementPtr>& statements)
{
    for (const auto& stmt : statements) {
        stmt->check(state);
    }
}

Statement::Statement(const DebugInfo& info) : Debuggable(info) {}

StatementAssign::StatementAssign(const DebugInfo& info,
    std::vector<std::string>&& names,
    std::vector<ExpressionPtr>&& expressions)
: Statement(info)
, m_variables(std::move(names))
, m_expressions(std::move(expressions)) {}

void StatementAssign::resolve(State& state) const
{
    for (const auto& expr : m_expressions) {
        expr->resolve(state);
    }
    Block& block = state.getBlock();
    for (auto iter = m_variables.rbegin(); iter != m_variables.rend(); ++iter) {
        // pop in reverse order
        bool value = state.pop();
        if (*iter != ignoreIdentifier) {
            block.store(*iter, value);
        }
    }
}

void StatementAssign::check(State& state) const
{
    if (m_variables.size() != countOutputs(state, m_expressions)) {
        std::stringstream s;
        s << "Not enough outputs for assignment.";
        throwError(s.str());
    }
    checkExpressions(state, m_expressions);
}

StatementVariable::StatementVariable(const DebugInfo& info,
    std::vector<std::string>&& names,
    std::vector<ExpressionPtr>&& expressions)
: Statement(info)
, m_variables(std::move(names))
, m_expressions(std::move(expressions)) {}

void StatementVariable::resolve(State& state) const
{
    for (const auto& expr : m_expressions) {
        expr->resolve(state);
    }
    Block& block = state.getBlock();
    for (auto iter = m_variables.rbegin(); iter != m_variables.rend(); ++iter) {
        // pop in reverse order
        if (*iter != ignoreIdentifier) {
            block.create(*iter, state.pop());
        } else {
            state.pop();
        }
    }
}

void StatementVariable::check(State& state) const
{
    if (m_variables.size() != countOutputs(state, m_expressions)) {
        std::stringstream s;
        s << "Not enough outputs for variables.";
        throwError(s.str());
    }
    checkExpressions(state, m_expressions);
}

StatementIf::StatementIf(const DebugInfo& info, ExpressionPtr cond,
    std::vector<StatementPtr>&& block)
: Statement(info)
, m_condition(std::move(cond))
, m_block(std::move(block)) {}

void StatementIf::resolve(State& state) const
{
    // check condition
    m_condition->resolve(state);
    if (state.pop()) {
        // push an empty block
        state.pushBlock({}, {});
        // call statements
        for (const auto& stmt : m_block) {
            stmt->resolve(state);
        }
        // pop block
        state.popBlock();
    }
}

void StatementIf::check(State& state) const
{
    if (m_condition->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "If statement expects only 1 input.";
        throwError(s.str());
    }
    m_condition->check(state);
    checkStatements(state, m_block);
}

StatementWhile::StatementWhile(const DebugInfo& info, ExpressionPtr cond,
    std::vector<StatementPtr>&& block)
: Statement(info)
, m_condition(std::move(cond))
, m_block(std::move(block)) {}

void StatementWhile::resolve(State& state) const
{
    // same as for if, but in a loop
    for (;;) {
        m_condition->resolve(state);
        if (!state.pop()) {
            // only difference is how the exit condition is handled
            return;
        }
        state.pushBlock({}, {});
        for (const auto& stmt : m_block) {
            stmt->resolve(state);
        }
        state.popBlock();
    }
}

void StatementWhile::check(State& state) const
{
    if (m_condition->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "While statement expects only 1 input.";
        throwError(s.str());
    }
    m_condition->check(state);
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

void StatementExpression::check(State& state) const
{
    if (m_expression->getOutputNum(state) > 0) {
        std::stringstream s;
        s << "Unhandled outputs from expression.";
        throwError(s.str());
    }
    m_expression->check(state);
}
