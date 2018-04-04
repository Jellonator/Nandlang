#pragma once
#include <vector>
#include <map>
#include <string>
#include <memory>

class State;
class Block;

typedef std::unique_ptr<Block> BlockPtr;

/// Represents a Block. Has a number of inputs and outputs. A function creates
/// a block, as well as while and if statements.
class Block {
    std::vector<bool> m_values;
    std::map<std::string, size_t> m_names;
    BlockPtr m_previous;
    size_t m_numOutputs;
public:
    Block(const std::vector<std::string>& input_names,
          const std::vector<std::string>& output_names,
          std::unique_ptr<Block> block, State& state);
    void create(const std::string& name, bool value);
    void store(const std::string& name, bool value);
    bool load(const std::string& name) const;
    BlockPtr push(State& state);
};
