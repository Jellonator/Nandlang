#include "state.h"
#include "compiler.h"
#include <stdexcept>
#include <sstream>

Function& State::getFunction(const std::string& name)
{
    if (m_functions.count(name)) {
        return *m_functions.at(name);
    } else {
        std::stringstream s;
        s << "No function of name \"" << name << "\"";
        throw std::runtime_error(s.str());
    }
}

void State::push(bool value)
{
    m_stack.push(value);
}

bool State::pop()
{
    bool ret = m_stack.top();
    m_stack.pop();
    return ret;
}

void State::pushBlock(const std::vector<std::string>& input_names,
                      const std::vector<std::string>& output_names)
{
    auto new_block = std::make_unique<Block>(
        input_names, output_names, std::move(m_block), *this);
    m_block = std::move(new_block);
}

void State::popBlock()
{
    m_block = m_block->push(*this);
}

Block& State::getBlock()
{
    return *m_block;
}

void State::parse(TokenBlock&& tokens)
{
    TokenTaker taker(std::move(tokens));
    while (taker) {
        std::string name;
        FunctionPtr func;
        std::tie(name, func) = parseFunction(taker);
        std::cout << name << std::endl;
        m_functions[name] = std::move(func);
    }
}
