#include "state.h"
#include "compiler.h"
#include <stdexcept>
#include <sstream>

/// Put bit function
void fn_putb(State& state)
{
    bool b = state.pop();
    std::cout << b;
}

/// Put endline function
void fn_endl(State& state)
{
    std::cout << std::endl;
}

/// Put 8-bit integer function
void fn_puti8(State& state) {
    uint8_t value(0);
    for (size_t i = 0; i < 8; ++i) {
        value >>= 1;
        if (state.pop()) {
            value += 0x80;
        }
    }
    std::cout << int(value);
}

/// Put character function
void fn_putc(State& state) {
    uint8_t value(0);
    for (size_t i = 0; i < 8; ++i) {
        value >>= 1;
        if (state.pop()) {
            value += 0x80;
        }
    }
    std::cout << value;
}

State::State()
{
    m_functions["putb"] = std::make_unique<FunctionExternal>(fn_putb, 1, 0);
    m_functions["puti8"] = std::make_unique<FunctionExternal>(fn_puti8, 8, 0);
    m_functions["putc"] = std::make_unique<FunctionExternal>(fn_putc, 8, 0);
    m_functions["endl"] = std::make_unique<FunctionExternal>(fn_endl, 0, 0);
}

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
    m_stack.push_back(value);
}

bool State::pop()
{
    bool ret = m_stack.back();
    m_stack.pop_back();
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
    m_block = std::move(m_block->push(*this));
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
        m_functions[name] = std::move(func);
    }
}

void State::check()
{
    for (const auto& func : m_functions) {
        func.second->check(*this);
    }
}

size_t State::size()
{
    return m_stack.size();
}
