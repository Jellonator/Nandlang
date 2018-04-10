#include "tokentaker.h"
#include <stdexcept>
#include <sstream>

TokenTaker::TokenTaker(TokenBlock&& block)
: m_tokens(block) {}

TokenTaker::TokenTaker() {}

void TokenTaker::push(Token&& token)
{
    m_tokens.push_back(std::move(token));
}

void TokenTaker::clear()
{
    m_tokens.clear();
}

Token TokenTaker::pop()
{
    if (*this) {
        Token ret = m_tokens.front();
        m_tokens.pop_front();
        return ret;
    } else {
        throw std::runtime_error("Attempt to pop from an empty TokenTaker");
    }
}

Symbol TokenTaker::peek() const
{
    if (*this) {
        return m_tokens.front().getSymbol();
    } else {
        return Symbol::NONE;
    }
}

const Token& TokenTaker::front() const
{
    if (*this) {
        return m_tokens.front();
    } else {
        throw std::runtime_error("Attempt to peek into an empty TokenTaker");
    }
}

const Token& TokenTaker::back() const
{
    if (*this) {
        return m_tokens.back();
    } else {
        throw std::runtime_error("Attempt to peek into an empty TokenTaker");
    }
}

bool TokenTaker::contains(Symbol s) const
{
    for (const auto& token : m_tokens) {
        if (s == token.getSymbol()) {
            return true;
        }
    }
    return false;
}

bool TokenTaker::empty() const
{
    return m_tokens.empty();
}

TokenTaker::operator bool() const
{
    return !empty();
}
