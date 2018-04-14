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

/// Get character function
void fn_getc(State& state) {
    char c;
    std::cin.get(c);
    for (size_t i = 0; i < 8; ++i) {
        // Most significant bit comes first, since this language behaves in
        // a big-endian way.
        bool b = c & 0x80;
        c <<= 1;
        state.push(b);
    }
}

/// Gets whether or not std::cin is able to be read
void fn_iogood(State& state) {
    state.push(bool(std::cin));
}

const std::map<std::string, FunctionExternal> stdlib = {
    {"putb",   {fn_putb,   1, 0}},
    {"puti8",  {fn_puti8,  8, 0}},
    {"endl",   {fn_endl,   0, 0}},
    {"putc",   {fn_putc,   8, 0}},
    {"getc",   {fn_getc,   0, 8}},
    {"iogood", {fn_iogood, 0, 1}}
};

State::State()
: m_varOffset(0)
{
    // load functions
    for (const auto& p : stdlib) {
        // FunctionExternal is copy-able, since std::function is copyable and
        // Debuggable is copyable.
        m_functions[p.first] = std::make_unique<FunctionExternal>(p.second);
    }
}

bool State::hasFunction(const std::string& name) const
{
    return m_functions.count(name) > 0;
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

const Function& State::getFunction(const std::string& name) const
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

size_t State::setVarOffset(size_t pos)
{
    size_t ret = m_varOffset;
    m_varOffset = pos;
    return ret;
}

void State::setVar(size_t pos, bool value)
{
    m_stack.at(m_varOffset + pos) = value;
}

bool State::getVar(size_t pos) const
{
    return m_stack.at(m_varOffset + pos);
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
    if (m_functions.count("main") == 0) {
        throwErrorNoInfo("No main function has been declared");
    }
    Function& mainfunc = getFunction("main");
    if (mainfunc.getInputNum() > 0) {
        throwErrorNoInfo("Main function should have 0 inputs");
    }
    if (mainfunc.getOutputNum() > 0) {
        throwErrorNoInfo("Main function should have 0 outputs");
    }
}

size_t State::size() const
{
    return m_stack.size();
}

void State::resize(size_t size) {
    m_stack.resize(size, 0);
}
