#pragma once
#include <iostream>
#include <string>
#include <memory>

/// Contains debugging information
class DebugInfo {
public:
    DebugInfo();
    size_t line;
    size_t column;
    size_t position;
    std::shared_ptr<std::string> filename;
};
std::ostream& operator<<(std::ostream&, const DebugInfo&);

/// Throw an error for the given debugging info object and the given description
void throwError(const DebugInfo& info, const std::string& what);
/// Throw an error that will have debug information tacked on later. This is
/// useful for errors that occur from a lack of proper Tokens, where a token
/// does not exist that can provide useful information.
void throwErrorNoInfo(const std::string& what);

/// Represents a class that contains debugging information.
class Debuggable {
    DebugInfo m_debuginfo;
public:
    Debuggable();
    Debuggable(const DebugInfo&);
    /// Throw an exception with the given description
    void throwError(const std::string& what) const;
    /// Get this object's debug info
    DebugInfo getDebugInfo() const;
    /// Set the debug info
    void setDebugInfo(const DebugInfo&);
};

/// An error that contains debug information
class DebugError : public Debuggable {
    std::string m_err;
public:
    DebugError(const DebugInfo&, const std::string&);
    const std::string& what() const;
};

/// An error that does not contain any locational information
/// Should be caught and given info later
class InfolessError {
    std::string m_err;
public:
    InfolessError(const std::string&);
    const std::string& what() const;
};
