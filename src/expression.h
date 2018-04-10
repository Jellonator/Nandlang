#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <set>
#include "debug.h"

class State;
class Function;

/// An expression. An expression has inputs and outputs.
class Expression : public Debuggable {
public:
    Expression(const DebugInfo& info);
    /// Call this expression. Will take getInputNum() values from the stack,
    /// then push getOutputNum() values onto the stack.
    virtual void resolve(State&) const = 0;
    /// Get the number of inputs.
    virtual uint64_t getInputNum(State&) const = 0;
    /// Get the number of outputs.
    virtual uint64_t getOutputNum(State&) const = 0;
    /// Check the expression to ensure consistency and integrity.
    /// Throws an exception on failure. First argument is the execution state,
    /// and the second argument is a set of variable names.
    virtual void check(State&) const = 0;
};

typedef std::unique_ptr<Expression> ExpressionPtr;

/// Count the number of outputs that the given list of expressions has.
/// This is because an expression can have a variable number of outputs.
size_t countOutputs(State& state,
    const std::vector<ExpressionPtr>& expressions);

/// Apply the check function for all of the given expressions
void checkExpressions(State& state,
    const std::vector<ExpressionPtr>& expressions);

/// A NAND expression. NANDS two values together
class ExpressionNand : public Expression {
    ExpressionPtr m_left;
    ExpressionPtr m_right;
public:
    ExpressionNand(const DebugInfo&, ExpressionPtr&&, ExpressionPtr&&);
    void resolve(State&) const override;
    uint64_t getInputNum(State&) const override;
    uint64_t getOutputNum(State&) const override;
    void check(State&) const override;
};

/// A function expression. Calls a function when evaluated
class ExpressionFunction : public Expression {
    std::string m_functionName;
    std::vector<ExpressionPtr> m_arguments;
    // mutable is fine here since this is just a cache so the function doesn't
    // have to be looked up every time
    mutable Function *m_function;
public:
    ExpressionFunction(const DebugInfo&, const std::string&,
        std::vector<ExpressionPtr>&&);
    void resolve(State&) const override;
    uint64_t getInputNum(State&) const override;
    uint64_t getOutputNum(State&) const override;
    void check(State&) const override;
};

/// A variable expression. Represents a variable
class ExpressionVariable : public Expression {
    size_t m_pos;
public:
    ExpressionVariable(const DebugInfo&, size_t pos);
    void resolve(State&) const override;
    uint64_t getInputNum(State&) const override;
    uint64_t getOutputNum(State&) const override;
    void check(State&) const override;
};

/// A variable expression. Represents a variable
class ExpressionArray : public Expression {
    size_t m_pos;
    size_t m_size;
public:
    ExpressionArray(const DebugInfo&, size_t pos, size_t size);
    void resolve(State&) const override;
    uint64_t getInputNum(State&) const override;
    uint64_t getOutputNum(State&) const override;
    void check(State&) const override;
};

/// A literal expression
class ExpressionLiteral : public Expression {
    bool m_value;
public:
    ExpressionLiteral(const DebugInfo&, bool);
    void resolve(State&) const override;
    uint64_t getInputNum(State&) const override;
    uint64_t getOutputNum(State&) const override;
    void check(State&) const override;
};
