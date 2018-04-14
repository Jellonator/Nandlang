#include "function.h"
#include "state.h"
#include <set>
#include <sstream>

Function::Function() : m_recurse(0) {}

FunctionExternal::FunctionExternal(std::function<void(State&)> func,
    uint64_t inputs, uint64_t outputs, ConstantLevel constant)
: Function(), m_inputNum(inputs), m_outputNum(outputs), m_function(func)
, m_constant(constant) {}

size_t Function::getRecursion() const
{
    return m_recurse;
}

uint64_t FunctionExternal::getInputNum() const
{
    return m_inputNum;
}

uint64_t FunctionExternal::getOutputNum() const
{
    return m_outputNum;
}

void FunctionExternal::call(State& state) const
{
    m_function(state);
}

void FunctionExternal::check(const State& state) const
{
    // nothing to do
}

ConstantLevel FunctionExternal::getConstantLevel(const State&) const
{
    return m_constant;
}

void FunctionExternal::optimize(State& state)
{
    // nothing to do
}

FunctionInternal::FunctionInternal(
    size_t inputs, size_t outputs,
    std::vector<StatementPtr>&& block)
: Function(), m_inputs(inputs), m_outputs(outputs), m_block(std::move(block))
, m_hasCalculatedConstant(false) {}

uint64_t FunctionInternal::getInputNum() const
{
    return m_inputs;
}

uint64_t FunctionInternal::getOutputNum() const
{
    return m_outputs;
}

void FunctionInternal::call(State& state) const
{
    // get the current size of the state
    // :[previous][inputs]
    size_t prev_size = state.size();
    // set the variable offset to include all of this function's inputs
    // [previous]:[inputs]
    size_t prev_var = state.setVarOffset(prev_size - m_inputs);
    // put values for outputs onto the stack
    // [previous]:[inputs][outputs]
    for (size_t i = 0; i < m_outputs; ++i) {
        state.push(0);
    }
    // resolve statements
    for (const auto& stmt : m_block) {
        stmt->resolve(state);
    }
    // put outputs onto the stack
    // [previous]:[outputs][garbage]
    for (size_t i = 0; i < m_outputs; i ++) {
        bool value = state.getVar(i + m_inputs);
        state.setVar(i, value);
    }
    // put variable offset to its previous value
    // :[previous][outputs][garbage]
    state.setVarOffset(prev_var);
    // resize stack to include only the outputs
    // :[previous][outputs]
    state.resize(prev_size - m_inputs + m_outputs);
}

void FunctionInternal::check(const State& state) const
{
    checkStatements(state, m_block);
}

ConstantLevel FunctionInternal::getConstantLevel(const State& state) const
{
    if (m_hasCalculatedConstant) {
        return m_constant;
    }
    if (m_recurse) {
        // Recursive function can not be optimized
        return ConstantLevel::GLOBAL;
    }
    ++ m_recurse;
    auto ret = getStatementsConstantLevel(state, m_block);
    -- m_recurse;
    m_constant = ret;
    return ret;
}

void FunctionInternal::optimize(State& state)
{
    optimizeStatements(state, m_block);
}
