#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <queue>

class ArgChain;
class ArgBlock;
struct ArgParse;
class ArgResult;

/// Exception for argument parsing
class ArgException : public std::exception {
public:
    /// Argument exception type
    enum Type {
        TYPE_OPTION_UNKNOWN,
        TYPE_OPTION_DUPLICATE,
        TYPE_OPTION_EXPECTED_ARG,
        TYPE_OPTION_INVALID,
        TYPE_TOO_MANY_ARGS,
        TYPE_NOT_ENOUGH_ARGS,
    };

    ArgException(Type code, const std::string& s);
    const char* what() const throw();

    /// Get the type of this ArgException
    Type type() const;
private:
    std::string m_msg; /// This exception's full message
    Type m_type; /// The type of this message
};

/// A class which is used to parse command line arguments as a chain.
/// The 'chain' can be broken up into 'blocks', where each block has its
/// own parsed arguments and options.
class ArgChain {
    std::queue<std::string> m_args;
public:
    ArgChain(const std::vector<std::string>& arguments);

    /// Parse up to nargs, accepting options from argdefs
    /// Will stop parsing block when either no arguments are left, or nargs
    /// arguments have been parsed. Will throw an exception if an
    /// unrecognized option is met, duplicates of the same option are
    /// found, or an option which expects an argument does not have an
    /// argument following it.
    /// If stop_at_end is true, then parsing will stop after nargs
    /// arguments have been parsed. In general, use stop_at_end if the
    /// argument is to be interpreted as a subcommand, that way only
    /// options before the subcommand are interpreted in this block. The
    /// subcommand can then be expected to parse its own arguments which
    /// occur after the subcommand.
    ArgBlock parse(int nargs, bool stop_at_end,
        const std::vector<ArgParse>& argdefs);

    /// Parse up to nargs.
    /// Will stop parsing block when either no arguments are left, nargs
    /// arguments have been parsed, or an option is met.
    ArgBlock parse_no_options(int nargs);

    /// Throw an exception if there are still unparsed arguments left.
    /// Call this function at the end of an agument parse chain, otherwise
    /// users will be confused when they have extra arguments at the end
    /// of a command.
    void assert_finished() const;
};

/// A simple struct that represents how an option can be parsed.
struct ArgParse {
    std::string m_name; /// Name of the option
    bool m_hasarg; /// Whether or not this option expects an argument after it
    char m_short; /// Shortened name of the option (\0 represents none)
};

/// Represents a single 'block' in an argument 'chain'.
/// A chain has its own list of arguments, as well as its own options.
class ArgBlock {
    friend class ArgChain;
    std::vector<std::string> m_args; /// Actual arguments
    std::map<std::string, std::string> m_values; /// Map of option values
    std::set<std::string> m_valid; /// Set of valid option names
    int m_nargs; /// Maximum number of arguments allowed in this block.
public:
    /// Make sure this block parsed all of its expected arguments.
    /// When calling ArgChain::parse, the returned ArgBlock stores the
    /// number of arguments that it was expected to parse. An exception
    /// will be thrown if the number of arguments that this block was
    /// expected to parse is not equal to the actual number of parsed
    /// arguments.
    void assert_all_args() const;

    /// Throw an exception if this block did not parse n arguments.
    void assert_num_args(int n) const;

    /// Throw an exception if at least n arguments were not parsed.
    void assert_least_num_args(int n) const;

    /// Assert that only the options defined in 'exceptions' were parsed.
    /// An exception will be thrown if this requirement is not met.
    void assert_options(const std::set<std::string>& exceptions) const;

    /// Number of arguments that were actually parsed.
    size_t size() const;

    /// Return if argument n is equal to value.
    /// This convenience function exists so that arguments can be tested
    /// without having to check the number of arguments parsed every time.
    bool match_arg(int n, const std::string& value) const;

    /// Get a direct reference to this block's arguments.
    const std::vector<std::string>& get_arguments() const;

    /// Check if this argument contains the given option.
    bool has_option(const std::string& name) const;

    /// Get the value of the given option. Note that options that do not
    /// take arguments, and options that are defined but not used will
    /// return an empty string. An option that was not defined for parsing
    /// will throw an exception.
    const std::string& get_option(const std::string& name) const;

    /// Get the argument at index i.
    const std::string& operator[](size_t i) const;

    /// Get the value of the given option.
    const std::string& operator[](const std::string& key) const;
};
