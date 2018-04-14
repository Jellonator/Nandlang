#include "expression.h"
#include "state.h"
#include <algorithm>
#include <sstream>

size_t countOutputs(const State& state,
    const std::vector<ExpressionPtr>& expressions)
{
    size_t ret = 0;
    for (const auto& expr : expressions) {
        ret += expr->getOutputNum(state);
    }
    return ret;
}

void checkExpressions(const State& state,
    const std::vector<ExpressionPtr>& expressions)
{
    for (const auto& expr : expressions) {
        expr->check(state);
    }
}

void optimizeExpressions(State& state,
    std::vector<ExpressionPtr>& expressions)
{
    auto iter_begin = expressions.begin();
    auto iter = expressions.begin();
    size_t num_used = 0;
    size_t num_bits = 0;
    auto insert = [&](){
        if (num_used > 0) {
            std::vector<bool> values;
            for (size_t i = 0; i < num_bits; ++i) {
                values.push_back(state.pop());
            }
            DebugInfo info = (*iter_begin)->getDebugInfo();
            iter = expressions.erase(iter_begin, iter);
            expressions.insert(iter_begin,
                std::make_unique<ExpressionLiteralArray>(
                    info, std::move(values)));
            ++iter;
        }
        iter_begin = iter+1;
        num_used = 0;
        num_bits = 0;
    };
    while (iter < expressions.end()) {
        if ((*iter)->getConstantLevel(state) >= ConstantLevel::CONSTANT) {
            ++num_used;
            (*iter)->resolve(state);
            num_bits += (*iter)->getOutputNum(state);
        } else {
            insert();
        }
        ++iter;
    }
    insert();
    for (auto& expr : expressions) {
        expr->optimize(state);
    }
}

ConstantLevel getExpressionsConstantLevel(const State& state,
    const std::vector<ExpressionPtr>& expressions)
{
    ConstantLevel ret = ConstantLevel::CONSTANT;
    for (const auto& expr : expressions) {
        ret = std::min(ret, expr->getConstantLevel(state));
        if (ret == ConstantLevel::GLOBAL) {
            break;
        }
    }
    return ret;
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

uint64_t ExpressionNand::getOutputNum(const State& state) const
{
    return 1;
}

void ExpressionNand::check(const State& state) const
{
    m_left->check(state);
    m_right->check(state);
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
}

ConstantLevel ExpressionNand::getConstantLevel(const State& state) const
{
    // Returns the constantness of the least-constant expression.
    return std::min({ConstantLevel::CONSTANT,
        m_left->getConstantLevel(state), m_left->getConstantLevel(state)});
}

void ExpressionNand::optimize(State& state)
{
    m_left->optimize(state);
    m_right->optimize(state);
}

ExpressionFunction::ExpressionFunction(
    const DebugInfo& info, const std::string& name,
    std::vector<ExpressionPtr>&& args)
: Expression(info), m_functionName(name),
m_arguments(std::move(args)), m_function(nullptr) {}

void ExpressionFunction::resolve(State& state) const
{
    for (auto iter = m_arguments.begin(); iter != m_arguments.end(); iter ++) {
        // push in forward order
        (*iter)->resolve(state);
    }
    if (!m_function) {
        Function& func = state.getFunction(m_functionName);
        m_function = &func;
    }
    m_function->call(state);
}

uint64_t ExpressionFunction::getOutputNum(const State& state) const
{
    const Function& func = state.getFunction(m_functionName);
    return func.getOutputNum();
}

void ExpressionFunction::check(const State& state) const
{
    if (!state.hasFunction(m_functionName)) {
        std::stringstream s;
        s << "Call to non-existent function " << m_functionName;
        throwError(s.str());
    }
    checkExpressions(state, m_arguments);
    const Function& func = state.getFunction(m_functionName);
    size_t inputnum = countOutputs(state, m_arguments);
    if (func.getInputNum() != inputnum) {
        std::stringstream s;
        s << "Function " << m_functionName << " expected "
          << func.getInputNum() << " inputs; got " << inputnum;
        throwError(s.str());
    }
    // Do NOT check func here because functions are already checked by State
}

ConstantLevel ExpressionFunction::getConstantLevel(const State& state) const
{
    const Function& func = state.getFunction(m_functionName);
    ConstantLevel func_const = func.getConstantLevel(state);
    ConstantLevel arg_const = getExpressionsConstantLevel(state, m_arguments);
    if (func_const == ConstantLevel::LOCAL) {
        // function only affects itself, so this expression is CONSTANT.
        func_const = ConstantLevel::CONSTANT;
    }
    return std::min(func_const, arg_const);
}

void ExpressionFunction::optimize(State& state)
{
    optimizeExpressions(state, m_arguments);
}

ExpressionVariable::ExpressionVariable(
    const DebugInfo& info, size_t pos)
: Expression(info), m_pos(pos) {}

void ExpressionVariable::check(const State& state) const
{
    // nothing to do
}

void ExpressionVariable::resolve(State& state) const
{
    state.push(state.getVar(m_pos));
}

uint64_t ExpressionVariable::getOutputNum(const State& state) const
{
    return 1;
}

ConstantLevel ExpressionVariable::getConstantLevel(const State&) const
{
    return ConstantLevel::LOCAL;
}

void ExpressionVariable::optimize(State& state)
{
    // nothing to do
}

ExpressionArray::ExpressionArray(
    const DebugInfo& info, size_t pos, size_t size)
: Expression(info), m_pos(pos), m_size(size) {}

void ExpressionArray::check(const State& state) const
{
    // nothing to do
}

void ExpressionArray::resolve(State& state) const
{
    for (size_t i = 0; i < m_size; ++i) {
        state.push(state.getVar(m_pos + i));
    }
}

uint64_t ExpressionArray::getOutputNum(const State& state) const
{
    return m_size;
}

ConstantLevel ExpressionArray::getConstantLevel(const State&) const
{
    return ConstantLevel::LOCAL;
}

void ExpressionArray::optimize(State& state)
{
    // nothing to do
}

ExpressionLiteral::ExpressionLiteral(const DebugInfo& info, bool value)
: Expression(info), m_value(value) {}

void ExpressionLiteral::resolve(State& state) const
{
    state.push(m_value);
}

uint64_t ExpressionLiteral::getOutputNum(const State& state) const
{
    return 1;
}

void ExpressionLiteral::check(const State& state) const
{
    // nothing to do
}

ConstantLevel ExpressionLiteral::getConstantLevel(const State&) const
{
    return ConstantLevel::LITERAL;
}

void ExpressionLiteral::optimize(State& state)
{
    // nothing to do
}

ExpressionLiteralArray::ExpressionLiteralArray(
    const DebugInfo& info, std::vector<bool>&& values)
: Expression(info), m_values(std::move(values)) {}

void ExpressionLiteralArray::resolve(State& state) const
{
    for (auto iter = m_values.rbegin(); iter != m_values.rend(); ++iter) {
        state.push(*iter);
    }
}

uint64_t ExpressionLiteralArray::getOutputNum(const State& state) const
{
    return m_values.size();
}

void ExpressionLiteralArray::check(const State& state) const
{
    // nothing to do
}

ConstantLevel ExpressionLiteralArray::getConstantLevel(const State&) const
{
    return ConstantLevel::LITERAL;
}

void ExpressionLiteralArray::optimize(State& state)
{
    // nothing to do
}
