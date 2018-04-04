#include "block.h"
#include "state.h"
#include <stdexcept>
#include <sstream>

Block::Block(const std::vector<std::string>& input_names,
             const std::vector<std::string>& output_names,
             std::unique_ptr<Block> block, State& state)
{
    for (const auto& name : output_names) {
        m_names[name] = m_values.size();
        m_values.push_back(0);
        m_numOutputs ++;
    }
    for (const auto& name : input_names) {
        m_names[name] = m_values.size();
        m_values.push_back(state.pop());
    }
    m_previous = std::move(block);
}

void Block::create(const std::string& name, bool value)
{
    if (m_names.count(name)) {
        std::stringstream s;
        s << "Variable " << name << " has already been defined.";
        throw std::runtime_error(s.str());
    } else {
        m_names[name] = m_values.size();
        m_values.push_back(value);
    }
}

void Block::store(const std::string& name, bool value)
{
    if (m_names.count(name)) {
        m_names.at(name) = value;
    } else if (m_previous) {
        m_previous->store(name, value);
    } else {
        std::stringstream s;
        s << "Could not store value for variable of name " << name;
        throw std::runtime_error(s.str());
    }
}

bool Block::load(const std::string& name) const
{
    if (m_names.count(name)) {
        return m_names.at(name);
    } else if (m_previous) {
        return m_previous->load(name);
    } else {
        std::stringstream s;
        s << "Could not store value for variable of name " << name;
        throw std::runtime_error(s.str());
        return false;
    }
}

BlockPtr Block::push(State& state)
{
    for (size_t i = 0; i < m_numOutputs; ++i) {
        state.push(m_values[i]);
    }
    return std::move(m_previous);
}
