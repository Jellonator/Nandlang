#include "debug.h"
#include <sstream>

DebugInfo::DebugInfo()
: line(0), column(0), position(0), filename(nullptr) {}

std::ostream& operator<<(std::ostream& stream, const DebugInfo& info)
{
    // output filename only if file is known
    if (info.filename) {
        stream << " in file " << *info.filename;
    }
    // output line if it is non-zero (known lines are not zero)
    if (info.line) {
        stream << " on line " << info.line << ":" << info.column;
    }
    return stream;
}

InfolessError::InfolessError(const std::string& what)
: m_err(what) {}

const char* InfolessError::what() const noexcept
{
    return m_err.c_str();
}

void throwError(const DebugInfo& info, const std::string& what)
{
    throw DebugError(info, what);
}

void throwErrorNoInfo(const std::string& what)
{
    throw InfolessError(what);
}

Debuggable::Debuggable()
: m_debuginfo() {}

Debuggable::Debuggable(const DebugInfo& info)
: m_debuginfo(info) {}

void Debuggable::throwError(const std::string& what) const
{
    ::throwError(m_debuginfo, what);
}

DebugInfo Debuggable::getDebugInfo() const
{
    return m_debuginfo;
}

void Debuggable::setDebugInfo(const DebugInfo& info)
{
    m_debuginfo = info;
}

DebugError::DebugError(const DebugInfo& info, const std::string& what)
: Debuggable(info) {
    std::stringstream s;
    s << "Error" << info << ":\n" << what;
    m_err = s.str();
}

const char* DebugError::what() const noexcept
{
    return m_err.c_str();
}
