#pragma once
#include <map>
#include "symbol.h"

struct NameStackDef {
    size_t pos;
    size_t size;
};

class NameStack {
    NameStack *m_prev;
    std::map<std::string, NameStackDef> m_names;
    size_t m_size;
public:
    NameStack();
    NameStack(NameStack&);
    /// Return true if the given name is used by this NameStack
    bool isNameInUse(const std::string&) const;
    /// Return true if the given name is defined by any NameStack
    bool isNameDefined(const std::string&) const;
    /// Insert a token's identifier and return its index
    /// Will check to make sure the given name is not already defined
    NameStackDef insert(const Token& token);
    /// Similar to insert, but takes an index as well
    NameStackDef insertIndexed(const Token& token, size_t index);
    /// Get the position of the given variable
    NameStackDef getPosition(const Token& token) const;
    /// Similar to getPosition, but takes an index as well
    NameStackDef getPositionIndexed(const Token& token, size_t index) const;
    /// Remove the given position without changing any other positions
    // (good for overriding positions)
    void removeName(const std::string& name);
    /// Get the number of elements in this specific NameStack (not parents)
    size_t size();
};
