#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "debug.h"

/// Special identifier used for ignoring parts of a function's output
extern const std::string ignoreIdentifier;

class State;

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
    /// Throws an exception on failure.
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
    std::string m_name;
public:
    ExpressionVariable(const DebugInfo&, const std::string&);
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
