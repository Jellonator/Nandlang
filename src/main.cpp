#include "parse.h"
#include "state.h"
#include "debug.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

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

void run(std::istream& stream, const DebugInfo& info)
{
    // create debugging information
    try {
        // parse characters into tokens
        TokenBlock block = parseTokens(stream, info);
        // create execution state
        State state;
        // load functions from token block
        state.parse(std::move(block));
        // check for integrity issues
        state.check();
        // call main function
        state.getFunction("main").call(state);
    } catch (DebugError& e) {
        handleError(e);
    } catch (std::exception& e) {
        // generic, unknown error
        std::cout << "Error: " << e.what() << std::endl;
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
"An esoteric programming language based on NAND completeness";

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cout << coolstuff << std::endl;
        std::cout << "Usage:\nnandlang <path/to/script>" << std::endl;
    } else {
        std::ifstream file(argv[1]);
        if (!file.is_open()) {
            std::cout << "Could not open file." << std::endl;
        } else {
            DebugInfo info;
            info.filename = std::make_shared<std::string>(argv[1]);
            info.line = 1;
            info.column = 1;
            info.position = 0;
            run(file, info);
        }
    }
    return 0;
}
