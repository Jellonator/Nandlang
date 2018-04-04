#include "expression.h"
#include "state.h"
#include <algorithm>
#include <sstream>

const std::string ignoreIdentifier = "_";

size_t countOutputs(State& state, const std::vector<ExpressionPtr>& expressions)
{
    size_t ret = 0;
    for (const auto& expr : expressions) {
        ret += expr->getOutputNum(state);
    }
    return ret;
}

void checkExpressions(State& state, const std::vector<ExpressionPtr>& expressions)
{
    for (const auto& expr : expressions) {
        expr->check(state);
    }
}

void ExpressionNand::resolve(State& state) const
{
    m_left->resolve(state);
    m_right->resolve(state);
    state.push(~(state.pop() & state.pop()));
}

uint64_t ExpressionNand::getInputNum(State& state) const
{
    return 2;
}

uint64_t ExpressionNand::getOutputNum(State& state) const
{
    return 1;
}

void ExpressionNand::check(State& state) const
{
    if (m_left->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "Left operand to NAND operator should have 1 output.";
        throw std::runtime_error(s.str());
    }
    if (m_right->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "Right operand to NAND operator should have 1 output.";
        throw std::runtime_error(s.str());
    }
    m_left->check(state);
    m_right->check(state);
}

void ExpressionFunction::resolve(State& state) const
{
    Function& func = state.getFunction(m_functionName);
    for (auto iter = m_arguments.rbegin(); iter != m_arguments.rend(); iter ++) {
        (*iter)->resolve(state);
    }
    func.call(state);
}

uint64_t ExpressionFunction::getInputNum(State& state) const
{
    Function& func = state.getFunction(m_functionName);
    return func.getInputNum();
}

uint64_t ExpressionFunction::getOutputNum(State& state) const
{
    Function& func = state.getFunction(m_functionName);
    return func.getOutputNum();
}

void ExpressionFunction::check(State& state) const
{
    Function& func = state.getFunction(m_functionName);
    if (func.getInputNum() != countOutputs(state, m_arguments)) {
        std::stringstream s;
        s << "Function expected different number of inputs.";
        throw std::runtime_error(s.str());
    }
    checkExpressions(state, m_arguments);
    func.check(state);
}

void ExpressionLiteral::resolve(State& state) const
{
    state.push(m_value);
}

uint64_t ExpressionLiteral::getInputNum(State& state) const
{
    return 0;
}

uint64_t ExpressionLiteral::getOutputNum(State& state) const
{
    return 1;
}

void ExpressionVariable::check(State& state) const
{
    // nothing to do
}

void ExpressionVariable::resolve(State& state) const
{
    state.push(state.getBlock().load(m_name));
}

uint64_t ExpressionVariable::getInputNum(State& state) const
{
    return 0;
}

uint64_t ExpressionVariable::getOutputNum(State& state) const
{
    return 1;
}

void ExpressionLiteral::check(State& state) const
{
    // nothing to do
}
