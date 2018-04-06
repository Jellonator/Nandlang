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

ExpressionNand::ExpressionNand(ExpressionPtr&& left, ExpressionPtr&& right)
: m_left(std::move(left)), m_right(std::move(right)) {}

void ExpressionNand::resolve(State& state) const
{
    m_left->resolve(state);
    m_right->resolve(state);
    bool left = state.pop();
    bool right = state.pop();
    state.push(!(left && right));
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

ExpressionFunction::ExpressionFunction(const std::string& name,
    std::vector<ExpressionPtr>&& args)
: m_functionName(name), m_arguments(std::move(args)) {}

void ExpressionFunction::resolve(State& state) const
{
    Function& func = state.getFunction(m_functionName);
    for (auto iter = m_arguments.begin(); iter != m_arguments.end(); iter ++) {
        // push in forward order
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

ExpressionVariable::ExpressionVariable(const std::string& name)
: m_name(name) {}

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

ExpressionLiteral::ExpressionLiteral(bool value)
: m_value(value) {}

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

void ExpressionLiteral::check(State& state) const
{
    // nothing to do
}
