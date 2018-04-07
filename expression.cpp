#include "expression.h"
#include "state.h"
#include <algorithm>
#include <sstream>

const std::string ignoreIdentifier = "_";

size_t countOutputs(State& state,
    const std::vector<ExpressionPtr>& expressions)
{
    size_t ret = 0;
    for (const auto& expr : expressions) {
        ret += expr->getOutputNum(state);
    }
    return ret;
}

void checkExpressions(State& state,
    const std::vector<ExpressionPtr>& expressions,
    std::set<std::string>& names)
{
    for (const auto& expr : expressions) {
        expr->check(state, names);
    }
}

Expression::Expression(const DebugInfo& info) : Debuggable(info) {}

ExpressionNand::ExpressionNand(
    const DebugInfo& info, ExpressionPtr&& left, ExpressionPtr&& right)
: Expression(info), m_left(std::move(left)), m_right(std::move(right)) {}

void ExpressionNand::resolve(State& state) const
{
    m_left->resolve(state);
    m_right->resolve(state);
    bool left = state.pop();
    bool right = state.pop();
    // nand, aka not and
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

void ExpressionNand::check(State& state, std::set<std::string>& names) const
{
    if (m_left->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "Left operand to NAND operator should have 1 output.";
        throwError(s.str());
    }
    if (m_right->getOutputNum(state) != 1) {
        std::stringstream s;
        s << "Right operand to NAND operator should have 1 output.";
        throwError(s.str());
    }
    m_left->check(state, names);
    m_right->check(state, names);
}

ExpressionFunction::ExpressionFunction(
    const DebugInfo& info, const std::string& name,
    std::vector<ExpressionPtr>&& args)
: Expression(info), m_functionName(name), m_arguments(std::move(args)) {}

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

void ExpressionFunction::check(State& state, std::set<std::string>& names) const
{
    Function& func = state.getFunction(m_functionName);
    if (func.getInputNum() != countOutputs(state, m_arguments)) {
        std::stringstream s;
        s << "Function expected different number of inputs.";
        throwError(s.str());
    }
    checkExpressions(state, m_arguments, names);
    // Do NOT check func here because functions are already checked by State
}

ExpressionVariable::ExpressionVariable(
    const DebugInfo& info, const std::string& name)
: Expression(info), m_name(name) {}

void ExpressionVariable::check(State& state, std::set<std::string>& names) const
{
    if (names.count(m_name) == 0) {
        std::stringstream s;
        s << "Use of undefined variable " << m_name;
        throwError(s.str());
    }
}

void ExpressionVariable::resolve(State& state) const
{
    bool value = state.getBlock().load(m_name);
    state.push(value);
}

uint64_t ExpressionVariable::getInputNum(State& state) const
{
    return 0;
}

uint64_t ExpressionVariable::getOutputNum(State& state) const
{
    return 1;
}

ExpressionLiteral::ExpressionLiteral(const DebugInfo& info, bool value)
: Expression(info), m_value(value) {}

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

void ExpressionLiteral::check(State& state, std::set<std::string>& names) const
{
    // nothing to do
}
