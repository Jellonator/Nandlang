#include "parse.h"
#include "state.h"
#include "debug.h"
#include "arg.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <chrono>

/// Replace every tab character with the given number of spaces
std::string replaceTabs(const std::string& str, size_t spaces)
{
    std::string ret;
    for (char c : str) {
        if (c == '\t') {
            for (size_t i = 0; i < spaces; ++i) {
                ret += ' ';
            }
        } else {
            ret += c;
        }
    }
    return ret;
}

/// handle the given error
void handleError(const DebugError& e)
{
    // Output error message
    std::cout << e.what() << std::endl;
    // Re-open file
    std::ifstream stream(*e.getDebugInfo().filename);
    // Seek to the given position, then go one past the last newline
    // character, so that the entire line can be loaded.
    stream.seekg(e.getDebugInfo().position, std::ios_base::beg);
    while (stream.tellg() > 0) {
        stream.unget();
        if (stream.peek() == '\n') {
            stream.get();
            break;
        }
    }
    // read the line in question
    std::string line;
    std::getline(stream, line);
    // the error line is every character up to and including the
    // point of error
    std::string errline = line.substr(0, e.getDebugInfo().column);
    // format line so that tabs are predictable
    line = replaceTabs(line, 4);
    errline = replaceTabs(errline, 4);
    // Every character in the error line is now a -
    for (char& c : errline) {
        c = '-';
    }
    // The error line points to the error
    errline.back() = '^';
    // output error line
    std::cout << line << std::endl;
    std::cout << errline << std::endl;
}

void printTime(std::ostream& stream, const std::string& s, std::chrono::duration<double> t)
{
    double usec = std::chrono::duration_cast<std::chrono::microseconds>(t).count();
    stream << s << std::fixed << std::setprecision(2) << std::setw(8)
           << std::right;
    if (usec >= 1000000.0) {
        stream << usec/1000000.0 << " s" << std::endl;
    } else if (usec >= 1000.0) {
        stream << usec/1000.0 << " ms" << std::endl;
    } else {
        stream << usec << " us" << std::endl;
    }
}

void run(std::istream& stream, const DebugInfo& info, bool benchmark)
{
    // Get time start
    auto time_start = std::chrono::system_clock::now();
    // parse characters into tokens
    TokenBlock block = parseTokens(stream, info);
    auto time_parse = std::chrono::system_clock::now();
    // create execution state
    State state;
    // load functions from token block
    state.parse(std::move(block));
    auto time_compile = std::chrono::system_clock::now();
    // check for integrity issues
    state.check();
    auto time_check = std::chrono::system_clock::now();
    // call main function
    state.getFunction("main").call(state);
    auto time_run = std::chrono::system_clock::now();
    if (benchmark) {
        std::cout << "Step      | Duration" << std::endl;
        printTime(std::cout, "Parsing   | ", time_parse-time_start);
        printTime(std::cout, "Compiling | ", time_compile-time_parse);
        printTime(std::cout, "Checking  | ", time_check-time_compile);
        printTime(std::cout, "Running   | ", time_run-time_check);
    }
}

const char* coolstuff =
"!!  !!   !!    !!!    !!   !!  !!!!!    !!         !!!    !!   !!   !!!!!   !!\n"
"!!  !!!  !!   !!!!!   !!!  !!  !!  !!   !!        !!!!!   !!!  !!  !!  !!!  !!\n"
"!!  !!!! !!  !!   !!  !!!! !!  !!   !!  !!       !!   !!  !!!! !!  !!       !!\n"
"!!  !!!!!!!  !!!!!!!  !!!!!!!  !!   !!  !!       !!!!!!!  !!!!!!!  !! !!!!  !!\n"
"!!  !! !!!!  !!   !!  !! !!!!  !!   !!  !!       !!   !!  !! !!!!  !!   !!  !!\n"
"    !!  !!!  !!   !!  !!  !!!  !!  !!   !!       !!   !!  !!  !!!  !!!!!!!    \n"
"!!  !!   !!  !!   !!  !!   !!  !!!!!    !!!!!!!  !!   !!  !!   !!   !!! !!  !!\n"
"Nandlang v1.2, An esoteric programming language based on NAND completeness";

int main(int argc, char **argv)
{
    try {
        std::vector<std::string> arguments;
        for (int i = 1; i < argc; ++i) {
            arguments.push_back(argv[i]);
        }
        ArgChain argchain(arguments);
        ArgBlock argblock = argchain.parse(1, false, {
            {"bench", false, 'b'}
        });
        if (argblock.size() == 0) {
            argchain.assert_finished();
            std::cout << coolstuff << std::endl;
        } else {
            argchain.assert_finished();
            bool do_benchmark = argblock.has_option("bench");
            std::ifstream file(argblock[0]);
            if (!file.is_open()) {
                std::cout << "Could not open file." << std::endl;
            } else {
                DebugInfo info;
                info.filename = std::make_shared<std::string>(argv[1]);
                info.line = 1;
                info.column = 1;
                info.position = 0;
                run(file, info, do_benchmark);
            }
        }
    } catch (DebugError& e) {
        handleError(e);
    } catch (std::exception& e) {
        // generic, unknown error
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}
