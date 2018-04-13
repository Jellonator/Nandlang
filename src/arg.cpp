#include "arg.h"
#include <stdexcept>
#include <sstream>

ArgException::ArgException(Type code, const std::string& msg)
: m_type(code)
{
    std::stringstream s;
    switch (code) {
    case Type::TYPE_OPTION_UNKNOWN:
        s << "Unknown Option: ";
        break;
    case Type::TYPE_OPTION_DUPLICATE:
        s << "Duplicate Option: ";
        break;
    case Type::TYPE_OPTION_EXPECTED_ARG:
        s << "Expected Argument After Option: ";
        break;
    case Type::TYPE_OPTION_INVALID:
        s << "Invalid Option: ";
        break;
    case Type::TYPE_TOO_MANY_ARGS:
        s << "Too many arguments, expected ";
        break;
    case Type::TYPE_NOT_ENOUGH_ARGS:
        s << "Not enough arguments, expected ";
        break;
    }
    s << msg;
    this->m_msg = s.str();
}

ArgException::Type ArgException::type() const
{
    return this->m_type;
}

const char* ArgException::what() const throw()
{
    return this->m_msg.c_str();
}

bool check_full_option(const std::string& val)
{
    if (val.size() >= 2) {
        return val[0] == '-' && val[1] == '-';
    }
    return false;
}

char check_short_option(const std::string& val)
{
    if (val.size() >= 1) {
        if (val.size() >= 3) {
            throw ArgException(ArgException::TYPE_OPTION_UNKNOWN, val);
        }
        if (val[0] == '-') {
            return val[1];
        } else {
            return 0;
        }
    }
    return 0;
}

std::string get_full_name(const std::string& name,
    const std::map<char, std::string>& shorthand)
{
    if (check_full_option(name)) {
        return name.substr(2);
    } else {
        char c = check_short_option(name);
        if (shorthand.count(c) == 0) {
            throw ArgException(ArgException::TYPE_OPTION_UNKNOWN, name);
        }
        return shorthand.at(c);
    }
}

bool is_option(const std::string& name)
{
    return name.size() > 0 && name[0] == '-';
}

ArgChain::ArgChain(const std::vector<std::string>& arguments)
: m_args() {
    for (auto& str : arguments) {
        this->m_args.push(str);
    }
}

ArgBlock ArgChain::parse(int nargs, bool stop_at_end,
    const std::vector<ArgParse>& argdefs)
{
    ArgBlock block;
    block.m_nargs = nargs;
    // Sort out shorthand and longhand names
    // Shorthand simply maps flags to longhand names
    std::map<std::string, const ArgParse&> longhand;
    std::map<char, std::string> shorthand;
    for (auto& def : argdefs) {
        longhand.emplace(def.m_name, def);
        if (def.m_short != '\0') {
            shorthand.emplace(def.m_short, def.m_name);
        }
        block.m_valid.insert(def.m_name);
    }
    while (!this->m_args.empty()) {
        if (nargs <= 0 && stop_at_end) {
            break;
        }
        std::string arg = this->m_args.front();
        if (is_option(arg)) {
            auto fullname = get_full_name(arg, shorthand);
            if (longhand.count(fullname) == 0) {
                throw ArgException(ArgException::TYPE_OPTION_UNKNOWN, arg);
            }
            if (block.m_values.count(fullname) != 0) {
                throw ArgException(ArgException::TYPE_OPTION_DUPLICATE, arg);
            }
            if (longhand.at(fullname).m_hasarg) {
                this->m_args.pop();
                if (this->m_args.empty()) {
                    throw ArgException(ArgException::TYPE_OPTION_EXPECTED_ARG, arg);
                }
                const std::string& newarg = this->m_args.front();
                if (is_option(newarg)) {
                    throw ArgException(ArgException::TYPE_OPTION_EXPECTED_ARG, arg);
                }
                block.m_values[fullname] = newarg;
            } else {
                block.m_values[fullname] = "";
            }
        } else {
            if (nargs <= 0) {
                break;
            }
            nargs --;
            block.m_args.push_back(arg);
        }
        this->m_args.pop();
    }
    return block;
}

ArgBlock ArgChain::parse_no_options(int nargs)
{
    ArgBlock block;
    block.m_nargs = nargs;
    // Sort out shorthand and longhand names
    // Shorthand simply maps flags to longhand names
    while (!this->m_args.empty()) {
        std::string arg = this->m_args.front();
        if (is_option(arg)) {
            break;
        } else {
            if (nargs <= 0) {
                break;
            }
            nargs --;
            block.m_args.push_back(arg);
        }
        this->m_args.pop();
    }
    return block;
}

void ArgChain::assert_finished() const {
    if (!this->m_args.empty()) {
        std::stringstream s;
        s << m_args.size() << " fewer.";
        throw ArgException(ArgException::TYPE_TOO_MANY_ARGS, s.str());
    }
}

void ArgBlock::assert_all_args() const
{
    if (this->m_nargs != int(this->size())) {
        auto t = ArgException::TYPE_NOT_ENOUGH_ARGS;
        if (int(this->size()) > this->m_nargs) {
            t = ArgException::TYPE_TOO_MANY_ARGS;
        }
        throw ArgException(t, std::to_string(this->m_nargs));
    }
}

void ArgBlock::assert_num_args(int n) const
{
    if (n != int(this->size())) {
        auto t = ArgException::TYPE_NOT_ENOUGH_ARGS;
        if (int(this->size()) > n) {
            t = ArgException::TYPE_TOO_MANY_ARGS;
        }
        throw ArgException(t, std::to_string(n));
    }
}

void ArgBlock::assert_least_num_args(int n) const
{
    if (n > int(this->size())) {
        auto t = ArgException::TYPE_NOT_ENOUGH_ARGS;
        throw ArgException(t, std::to_string(n));
    }
}

void ArgBlock::assert_options(
    const std::set<std::string>& exceptions) const
{
    for (auto& namepair : this->m_values) {
        if (exceptions.count(namepair.first) == 0) {
            std::stringstream s;
            s << "Unexpected option '--" << namepair.first << "'";
            throw std::runtime_error(s.str());
            throw ArgException(ArgException::TYPE_OPTION_INVALID,
                "--" + namepair.first);
        }
    }
}

size_t ArgBlock::size() const
{
    return this->m_args.size();
}

bool ArgBlock::match_arg(int n, const std::string& value) const
{
    if (n >= 0 && n < int(this->size())) {
        return this->m_args[n] == value;
    }
    return false;
}

const std::vector<std::string>& ArgBlock::get_arguments() const
{
    return this->m_args;
}

bool ArgBlock::has_option(const std::string& name) const
{
    return this->m_values.count(name) != 0;
}

const std::string emptystr = "";
const std::string& ArgBlock::get_option(const std::string& name) const
{
    if (this->m_values.count(name) != 0) {
        return this->m_values.at(name);
    } else {
        if (this->m_valid.count(name) == 0) {
            throw ArgException(ArgException::TYPE_OPTION_INVALID,
                "--" + name);
        }
        return emptystr;
    }
}

const std::string& ArgBlock::operator[](size_t i) const
{
    return this->m_args[i];
}

const std::string& ArgBlock::operator[](const std::string& name) const
{
    return this->get_option(name);
}
