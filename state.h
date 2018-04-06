#pragma once
#include <vector>
#include <string>
#include <map>
#include "function.h"
#include "symbol.h"
#include "block.h"

/// Represents the execution state
/// Always push in forward order, and always pop in reverse order.
class State {
    /// Value stack
    std::vector<bool> m_stack;
    /// Maps names to functions
    std::map<std::string, FunctionPtr> m_functions;
    /// The current block
    BlockPtr m_block;
public:
    State();
    /// Get a function from name
    Function& getFunction(const std::string& name);
    /// push value to stack
    void push(bool value);
    /// Pop value from stack
    bool pop();
    /// Push a block onto the stack
    void pushBlock(const std::vector<std::string>& input_names,
                   const std::vector<std::string>& output_names);
    /// Pop a block from the stack
    void popBlock();
    /// Parse a file to create functions
    void parse(TokenBlock&& tokens);
    /// Get the current block
    Block& getBlock();
    /// check this state for consistency and integrity
    /// will throw an exception if one of the following rules are broken:
    /// * inputs and outputs are mismatched in number
    void check();
};
