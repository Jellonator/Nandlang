#include "statement.h"
#include "state.h"
#include <stdexcept>
#include <sstream>

void checkStatements(State& state, const std::vector<StatementPtr>& statements,
    const std::set<std::string>& namecheck)
{
    std::set<std::string> names(namecheck.begin(), namecheck.end());
    for (const auto& stmt : statements) {
        stmt->check(state, names);
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

void StatementAssign::check(State& state, std::set<std::string>& names) const
{
    for (const auto& variable : m_variables) {
        if (variable != ignoreIdentifier) {
            if (names.count(variable) == 0) {
                std::stringstream s;
                s << "Attempt to assign value to undefined variable " << variable;
                throwError(s.str());
            }
        }
    }
    checkExpressions(state, m_expressions, names);
    if (m_variables.size() != countOutputs(state, m_expressions)) {
        std::stringstream s;
        s << "Not enough outputs for assignment.";
        throwError(s.str());
    }
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

void StatementVariable::check(State& state, std::set<std::string>& names) const
{
    for (const auto& variable : m_variables) {
        if (variable != ignoreIdentifier) {
            if (names.count(variable) != 0) {
                std::stringstream s;
                s << "Attempt to declare an already existing variable " << variable;
                throwError(s.str());
            }
            names.insert(variable);
        }
    }
    checkExpressions(state, m_expressions, names);
    if (m_variables.size() != countOutputs(state, m_expressions)) {
        std::stringstream s;
        s << "Not enough outputs for variables.";
        throwError(s.str());
    }
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

void StatementIf::check(State& state, std::set<std::string>& names) const
{
    if (m_condition->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "If statement expects only 1 input.";
        throwError(s.str());
    }
    m_condition->check(state, names);
    checkStatements(state, m_block, names);
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

void StatementWhile::check(State& state, std::set<std::string>& names) const
{
    if (m_condition->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "While statement expects only 1 input.";
        throwError(s.str());
    }
    m_condition->check(state, names);
    checkStatements(state, m_block, names);
}

StatementExpression::StatementExpression(
    ExpressionPtr&& expr)
: Statement(expr->getDebugInfo())
, m_expression(std::move(expr)) {}

void StatementExpression::resolve(State& state) const
{
    m_expression->resolve(state);
}

void StatementExpression::check(State& state, std::set<std::string>& names) const
{
    if (m_expression->getOutputNum(state) > 0) {
        std::stringstream s;
        s << "Unhandled outputs from expression.";
        throwError(s.str());
    }
    m_expression->check(state, names);
}
