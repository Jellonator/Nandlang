#pragma once
#include <iostream>
#include <string>
#include <memory>

class DebugInfo {
public:
    DebugInfo();
    size_t line;
    std::shared_ptr<std::string> filename;
};
std::ostream& operator<<(std::ostream&, const DebugInfo&);

/// Throw an error for the given debugging info object and the given description
void throwError(const DebugInfo& info, const std::string& what);

/// Represents a class that contains debugging information.
class Debuggable {
    DebugInfo m_debuginfo;
public:
    /// Throw an exception with the given description
    void throwError(const std::string& what) const;
    /// Get this object's debug info
    DebugInfo getDebuginfo() const;
    /// Set the line number
    void setDebugLine(size_t);
    /// Set the file
    void setDebugFile(std::shared_ptr<std::string>);
    /// Set the debug info
    void setDebugInfo(const DebugInfo&);
};
