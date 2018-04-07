#include "function.h"
#include "state.h"
#include <set>
#include <sstream>

FunctionExternal::FunctionExternal(
    std::function<void(State&)> func,
    uint64_t inputs, uint64_t outputs)
: m_inputNum(inputs)
, m_outputNum(outputs)
, m_function(func) {}

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

void FunctionExternal::check(State& state) const
{
    // nothing to do
}

FunctionInternal::FunctionInternal(
    std::vector<std::string>&& inputs,
    std::vector<std::string>&& outputs,
    std::vector<StatementPtr>&& block)
: m_inputNames(std::move(inputs))
, m_outputNames(std::move(outputs))
, m_block(std::move(block)) {}

uint64_t FunctionInternal::getInputNum() const
{
    return m_inputNames.size();
}

uint64_t FunctionInternal::getOutputNum() const
{
    return m_outputNames.size();
}

void FunctionInternal::call(State& state) const
{
    state.pushBlock(m_inputNames, m_outputNames);
    for (const auto& stmt : m_block) {
        stmt->resolve(state);
    }
    state.popBlock();
}

void FunctionInternal::check(State& state) const
{
    std::set<std::string> namecheck;
    for (const auto& name : m_inputNames) {
        if (namecheck.count(name)) {
            std::stringstream s;
            s << "Duplicate parameter of name " << name;
            throwError(s.str());
        }
        namecheck.insert(name);
    }
    for (const auto& name : m_outputNames) {
        if (namecheck.count(name)) {
            std::stringstream s;
            s << "Duplicate output of name " << name;
            throwError(s.str());
        }
        namecheck.insert(name);
    }
    checkStatements(state, m_block);
}
