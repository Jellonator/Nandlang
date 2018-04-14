#pragma once
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include "debug.h"
#include "statement.h"

class State;

/// A function that can be called
/// Has a set number of inputs and outputs
class Function : public Debuggable {
protected:
    mutable size_t m_recurse;
public:
    Function();
    /// get number of inputs
    virtual uint64_t getInputNum() const = 0;
    /// get number of outputs
    virtual uint64_t getOutputNum() const = 0;
    /// call this function
    virtual void call(State&) const = 0;
    /// Check the function to ensure consistency and integrity.
    /// Throws an exception on failure.
    virtual void check(const State&) const = 0;
    /// Returns the constant-ness of this function
    virtual ConstantLevel getConstantLevel(const State&) const = 0;
    /// Optimize this function
    virtual void optimize(State& state) = 0;
    /// Get the recursion level of this function
    size_t getRecursion() const;
};

typedef std::unique_ptr<Function> FunctionPtr;

/// An external C++ function
class FunctionExternal : public Function {
    uint64_t m_inputNum;
    uint64_t m_outputNum;
    std::function<void(State&)> m_function;
    ConstantLevel m_constant;
public:
    FunctionExternal(std::function<void(State&)> func,
                     uint64_t inputs, uint64_t outputs, ConstantLevel constant);
    uint64_t getInputNum() const override;
    uint64_t getOutputNum() const override;
    void call(State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
    void optimize(State& state) override;
};

/// An internal Nandlang function
class FunctionInternal : public Function {
    size_t m_inputs;
    size_t m_outputs;
    std::vector<StatementPtr> m_block;
    mutable ConstantLevel m_constant;
    mutable bool m_hasCalculatedConstant;
public:
    FunctionInternal(size_t inputs, size_t outputs,
                     std::vector<StatementPtr>&& block);
    uint64_t getInputNum() const override;
    uint64_t getOutputNum() const override;
    void call(State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
    void optimize(State& state) override;
};
