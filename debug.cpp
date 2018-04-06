#include "debug.h"
#include <sstream>
#include <stdexcept>

DebugInfo::DebugInfo()
: line(0), filename(nullptr) {}

std::ostream& operator<<(std::ostream& stream, const DebugInfo& info)
{
    if (info.filename) {
        stream << " in file " << *info.filename;
    }
    if (info.line) {
        stream << " on line " << info.line;
    }
    return stream;
}

void throwError(const DebugInfo& info, const std::string& what)
{
    std::stringstream s;
    s << "Error" << info << ":\n" << what;
    throw std::runtime_error(s.str());
}

void Debuggable::throwError(const std::string& what) const
{
    ::throwError(m_debuginfo, what);
}

DebugInfo Debuggable::getDebuginfo() const
{
    return m_debuginfo;
}

void Debuggable::setDebugLine(size_t line)
{
    m_debuginfo.line = line;
}

void Debuggable::setDebugFile(std::shared_ptr<std::string> filename)
{
    m_debuginfo.filename = filename;
}

void Debuggable::setDebugInfo(const DebugInfo& info)
{
    m_debuginfo = info;
}
