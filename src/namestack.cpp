#include "namestack.h"
#include <sstream>

NameStack::NameStack()
: m_prev(nullptr), m_size(0) {}

NameStack::NameStack(NameStack& other)
: m_prev(&other), m_size(0) {}

bool NameStack::isNameInUse(const std::string& name) const
{
    return m_names.count(name);
}

bool NameStack::isNameDefined(const std::string& name) const
{
    if (m_names.count(name) || name == ignoreIdentifier) {
        return true;
    } else if (m_prev) {
        return m_prev->isNameDefined(name);
    } else {
        return false;
    }
}

NameStackDef NameStack::insert(const Token& token)
{
    return insertIndexed(token, 1);
}

NameStackDef NameStack::insertIndexed(const Token& token, size_t index)
{
    if (isNameInUse(token.getIdentifier())) {
        std::stringstream s;
        s << "Attempt to define already existing variable "
          << token.getIdentifier();
        token.throwError(s.str());
    }
    if (index == 0) {
        std::stringstream s;
        s << "Array can not have 0 size";
        token.throwError(s.str());
    }
    NameStackDef def;
    def.size = index;
    if (token.getIdentifier() == ignoreIdentifier) {
        def.pos = ignorePosition;
    } else {
        def.pos = size();
        m_size += index;
        m_names[token.getIdentifier()] = def;
    }
    return def;
}

NameStackDef NameStack::getPosition(const Token& token) const
{
    if (token.getIdentifier() == ignoreIdentifier) {
        NameStackDef def;
        def.pos = ignorePosition;
        def.size = 1;
        return def;
    }
    if (!isNameDefined(token.getIdentifier())) {
        std::stringstream s;
        s << "Attempt to use undefined variable "
          << token.getIdentifier();
        token.throwError(s.str());
    } else {
        if (m_prev && m_names.count(token.getIdentifier()) == 0) {
            return m_prev->getPosition(token);
        } else {
            return m_names.at(token.getIdentifier());
        }
    }
}

NameStackDef NameStack::getPositionIndexed(const Token& token, size_t index) const
{
    if (token.getIdentifier() == ignoreIdentifier) {
        NameStackDef def;
        def.pos = ignorePosition;
        def.size = index;
        return def;
    }
    if (!isNameDefined(token.getIdentifier())) {
        std::stringstream s;
        s << "Attempt to use undefined variable "
          << token.getIdentifier();
        token.throwError(s.str());
    } else {
        if (m_prev && m_names.count(token.getIdentifier()) == 0) {
            return m_prev->getPosition(token);
        } else {
            if (m_prev && m_names.count(token.getIdentifier()) == 0) {
                return m_prev->getPositionIndexed(token, index);
            } else {
                NameStackDef def = m_names.at(token.getIdentifier());
                if (index >= def.size) {
                    std::stringstream s;
                    s << "Index out of bounds ";
                    token.throwError(s.str());
                }
                def.pos += index;
                def.size = 1;
                return def;
            }
        }
    }
}

void NameStack::removeName(const std::string& name)
{
    m_names.erase(name);
}

size_t NameStack::size()
{
    if (m_prev) {
        return m_size + m_prev->size();
    } else {
        return m_size;
    }
}
