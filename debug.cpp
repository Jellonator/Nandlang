#include "debug.h"
#include <sstream>
#include <stdexcept>

DebugInfo::DebugInfo()
: line(0), column(0), position(0), filename(nullptr) {}

std::ostream& operator<<(std::ostream& stream, const DebugInfo& info)
{
    if (info.filename) {
        stream << " in file " << *info.filename;
    }
    if (info.line) {
        stream << " on line " << info.line << ":" << info.column;
    }
    return stream;
}

InfolessError::InfolessError(const std::string& what)
: m_err(what) {}

const std::string& InfolessError::what() const
{
    return m_err;
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

const std::string& DebugError::what() const
{
    return m_err;
}
