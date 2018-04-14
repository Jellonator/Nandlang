#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <set>
#include "debug.h"

class State;
class Function;

/// Level of a constant expression.
/// GLOBAL means that this expression affects or is affected by the global state
/// LOCAL means that this expression affects or is affected the local state
/// CONSTANT means that this is a fully constant expression.
/// This allows for the following opimization rules:
///     * LOCAL functions can be pre-calculated
///     * CONSTANT expressions can be pre-calculated
///     * CONSTANT statements can be removed
enum class ConstantLevel {
    // Order matters. The lower in this list, the more 'constant' it is.
    GLOBAL,
    LOCAL,
    CONSTANT
};

/// An expression. An expression has inputs and outputs.
class Expression : public Debuggable {
public:
    Expression(const DebugInfo& info);
    /// Call this expression. Will take getInputNum() values from the stack,
    /// then push getOutputNum() values onto the stack.
    virtual void resolve(State&) const = 0;
    /// Get the number of outputs.
    virtual uint64_t getOutputNum(const State&) const = 0;
    /// Check the expression to ensure consistency and integrity.
    /// Throws an exception on failure. First argument is the execution state,
    /// and the second argument is a set of variable names.
    virtual void check(const State&) const = 0;
    /// Returns the constant-ness of this expression
    virtual ConstantLevel getConstantLevel(const State&) const = 0;
};

typedef std::unique_ptr<Expression> ExpressionPtr;

/// Count the number of outputs that the given list of expressions has.
/// This is because an expression can have a variable number of outputs.
size_t countOutputs(const State& state,
    const std::vector<ExpressionPtr>& expressions);

/// Apply the check function for all of the given expressions
void checkExpressions(const State& state,
    const std::vector<ExpressionPtr>& expressions);
/// Get the constantness of the given expression list
ConstantLevel getExpressionsConstantLevel(const State& state,
    const std::vector<ExpressionPtr>& expressions);

/// A NAND expression. NANDS two values together
class ExpressionNand : public Expression {
    ExpressionPtr m_left;
    ExpressionPtr m_right;
public:
    ExpressionNand(const DebugInfo&, ExpressionPtr&&, ExpressionPtr&&);
    void resolve(State&) const override;
    uint64_t getOutputNum(const State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
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
    uint64_t getOutputNum(const State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
};

/// A variable expression. Represents a variable
class ExpressionVariable : public Expression {
    size_t m_pos;
public:
    ExpressionVariable(const DebugInfo&, size_t pos);
    void resolve(State&) const override;
    uint64_t getOutputNum(const State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
};

/// A variable expression. Represents a variable
class ExpressionArray : public Expression {
    size_t m_pos;
    size_t m_size;
public:
    ExpressionArray(const DebugInfo&, size_t pos, size_t size);
    void resolve(State&) const override;
    uint64_t getOutputNum(const State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
};

/// A literal expression
class ExpressionLiteral : public Expression {
    bool m_value;
public:
    ExpressionLiteral(const DebugInfo&, bool);
    void resolve(State&) const override;
    uint64_t getOutputNum(const State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
};

/// A literal array expression
class ExpressionLiteralArray : public Expression {
    std::vector<bool> m_values;
public:
    /// Constructor expects values in reverse order
    ExpressionLiteralArray(const DebugInfo&, std::vector<bool>&&);
    void resolve(State&) const override;
    uint64_t getOutputNum(const State&) const override;
    void check(const State&) const override;
    ConstantLevel getConstantLevel(const State&) const override;
};
