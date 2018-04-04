#pragma once
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include "statement.h"

class State;

/// A function that can be called
/// Has a set number of inputs and outputs
class Function {
public:
    /// get number of inputs
    virtual uint64_t getInputNum() const = 0;
    /// get number of outputs
    virtual uint64_t getOutputNum() const = 0;
    /// call this function
    virtual void call(State&) const = 0;
    /// Check the function to ensure consistency and integrity.
    /// Throws an exception on failure.
    virtual void check(State&) const = 0;
};

typedef std::unique_ptr<Function> FunctionPtr;

/// An external C++ function
class FunctionExternal : public Function {
    uint64_t m_inputNum;
    uint64_t m_outputNum;
    std::function<void(State&)> m_function;
public:
    FunctionExternal(std::function<void(State&)> func,
                     uint64_t inputs, uint64_t outputs);
    uint64_t getInputNum() const override;
    uint64_t getOutputNum() const override;
    void call(State&) const override;
    void check(State&) const override;
};

/// An internal Nandlang function
class FunctionInternal : public Function {
    std::vector<std::string> m_inputNames;
    std::vector<std::string> m_outputNames;
    std::vector<StatementPtr> m_block;
public:
    FunctionInternal(std::vector<std::string>&& inputs,
                     std::vector<std::string>&& outputs,
                     std::vector<StatementPtr>&& block);
    uint64_t getInputNum() const override;
    uint64_t getOutputNum() const override;
    void call(State&) const override;
    void check(State&) const override;
};
