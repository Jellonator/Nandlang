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

void StatementAssign::resolve(State& state) const
{
    for (const auto& expr : m_expressions) {
        expr->resolve(state);
    }
    Block& block = state.getBlock();
    for (auto iter = m_variables.rbegin(); iter != m_variables.rend(); ++iter) {
        if (*iter != ignoreIdentifier) {
            block.store(*iter, state.pop());
        } else {
            state.pop();
        }
    }
}

void StatementAssign::check(State& state) const
{
    if (m_variables.size() != countOutputs(state, m_expressions)) {
        std::stringstream s;
        s << "Not enough outputs for assignment.";
        throw std::runtime_error(s.str());
    }
    checkExpressions(state, m_expressions);
}

void StatementIf::resolve(State& state) const
{
    m_condition->resolve(state);
    if (state.pop()) {
        state.pushBlock({}, {});
        for (const auto& stmt : m_block) {
            stmt->resolve(state);
        }
        state.popBlock();
    }
}

void StatementIf::check(State& state) const
{
    if (m_condition->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "If statement expects only 1 input.";
        throw std::runtime_error(s.str());
    }
    m_condition->check(state);
    checkStatements(state, m_block);
}

void StatementWhile::resolve(State& state) const
{
    for (;;) {
        m_condition->resolve(state);
        if (!state.pop()) {
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
        throw std::runtime_error(s.str());
    }
    m_condition->check(state);
    checkStatements(state, m_block);
}

void StatementVariable::resolve(State& state) const
{
    for (const auto& expr : m_expressions) {
        expr->resolve(state);
    }
    Block& block = state.getBlock();
    for (auto iter = m_variables.rbegin(); iter != m_variables.rend(); ++iter) {
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
        throw std::runtime_error(s.str());
    }
    checkExpressions(state, m_expressions);
}

void StatementExpression::resolve(State& state) const
{
    m_expression->resolve(state);
}

void StatementExpression::check(State& state) const
{
    if (m_expression->getOutputNum(state) > 0) {
        std::stringstream s;
        s << "Unhandled outputs from expression.";
        throw std::runtime_error(s.str());
    }
    m_expression->check(state);
}
