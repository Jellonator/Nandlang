#pragma once
#include <set>
#include "debug.h"
#include "expression.h"

class State;

/// A statement. Unlike an expression, a statement does not have any outputs.
class Statement : public Debuggable {
public:
    Statement(const DebugInfo& info);
    /// Resolve this statement. This is similar to calling a function.
    virtual void resolve(State&) const = 0;
    /// Check the statement to ensure consistency and integrity.
    /// Throws an exception on failure. The first argument is the execution
    /// state, and the second argument is a set of variable names.
    virtual void check(State&) const = 0;
};

/// Unique pointer to a statement
typedef std::unique_ptr<Statement> StatementPtr;

/// Check the given statements to integrity errors. Since checkStatement is used
/// for blocks and blocks may declare their own variables, the given namecheck
/// will not be modified.
void checkStatements(State& state, const std::vector<StatementPtr>& statements);

/// An assignment statement. Assigns a value to a variable
class StatementAssign : public Statement {
    std::vector<size_t> m_variables;
    std::vector<ExpressionPtr> m_expressions;
public:
    StatementAssign(const DebugInfo&, std::vector<size_t>&&,
        std::vector<ExpressionPtr>&&);
    void resolve(State& state) const override;
    void check(State&) const override;
};

/// A var statement. Declares a variable.
class StatementVariable : public Statement {
    std::vector<size_t> m_variables;
    std::vector<ExpressionPtr> m_expressions;
public:
    StatementVariable(const DebugInfo&, std::vector<size_t>&&,
        std::vector<ExpressionPtr>&&);
    void resolve(State& state) const override;
    void check(State&) const override;
};

/// An if statement. Checks a condition to execute a block of statements
class StatementIf : public Statement {
    ExpressionPtr m_condition;
    std::vector<StatementPtr> m_block;
    std::vector<StatementPtr> m_else;
public:
    StatementIf(const DebugInfo&, ExpressionPtr,
        std::vector<StatementPtr>&& block,
        std::vector<StatementPtr>&& elseblock);
    void resolve(State& state) const override;
    void check(State&) const override;
};

/// A while statement. Executes a block of statements while a condition is true.
class StatementWhile : public Statement {
    ExpressionPtr m_condition;
    std::vector<StatementPtr> m_block;
public:
    StatementWhile(const DebugInfo&, ExpressionPtr,
        std::vector<StatementPtr>&&);
    void resolve(State& state) const override;
    void check(State&) const override;
};

/// A statement that is simply an expression
class StatementExpression : public Statement {
    ExpressionPtr m_expression;
public:
    StatementExpression(ExpressionPtr&& expr);
    void resolve(State& state) const override;
    void check(State&) const override;
};
