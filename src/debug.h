#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

/// Contains debugging information
class DebugInfo {
public:
    DebugInfo();
    // line in file
    size_t line;
    /// column in file
    size_t column;
    /// byte position in file
    size_t position;
    /// name of the file. This is a shared pointer so that memory/processing
    /// isn't wasted too much by copying strings around.
    std::shared_ptr<std::string> filename;
};
std::ostream& operator<<(std::ostream&, const DebugInfo&);

/// Throw an error for the given debugging info object and the given description
[[noreturn]] void throwError(const DebugInfo& info, const std::string& what);
/// Throw an error that will have debug information tacked on later. This is
/// useful for errors that occur from a lack of proper Tokens, where a token
/// does not exist that can provide useful information.
[[noreturn]] void throwErrorNoInfo(const std::string& what);

/// Represents a class that contains debugging information.
class Debuggable {
    DebugInfo m_debuginfo;
public:
    Debuggable();
    Debuggable(const DebugInfo&);
    /// Throw an exception with the given description
    [[noreturn]] void throwError(const std::string& what) const;
    /// Get this object's debug info
    DebugInfo getDebugInfo() const;
    /// Set the debug info
    void setDebugInfo(const DebugInfo&);
};

/// An error that contains debug information
class DebugError : public Debuggable, public std::exception {
    std::string m_err;
public:
    DebugError(const DebugInfo&, const std::string&);
    /// Error message
    const char* what() const noexcept;
};

/// An error that does not contain any locational information
/// Should be caught and given info later
class InfolessError : public std::exception {
    std::string m_err;
public:
    InfolessError(const std::string&);
    /// Error message
    const char* what() const noexcept;
};
