#include "parse.h"
#include "state.h"
#include "debug.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

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

int main(int argc, char **argv)
{
    if (argc == 1) {
        std::cout << "Expected 1 argument." << std::endl;
    } else if (argc == 2) {
        std::ifstream stream(argv[1]);
        if (!stream.is_open()) {
            std::cout << "Could not open file." << std::endl;
        } else {
            DebugInfo info;
            info.filename = std::make_shared<std::string>(argv[1]);
            info.line = 1;
            info.column = 1;
            info.position = 0;
            try {
                TokenBlock block = parseTokens(stream, info);
                // printBlock(block);
                State state;
                state.parse(std::move(block));
                state.check();
                state.getFunction("main").call(state);
                std::cout << "Execution successful!" << std::endl;
            } catch (DebugError& e) {
                std::cout << e.what() << std::endl;
                stream.seekg(e.getDebugInfo().position-1, std::ios_base::beg);
                while (stream.tellg() > 0 && stream.peek() != '\n') {
                    stream.unget();
                }
                stream.get();
                std::string line;
                std::getline(stream, line);
                std::string errline = line.substr(0, e.getDebugInfo().column-1);
                line = replaceTabs(line, 4);
                errline = replaceTabs(errline, 4);
                for (char& c : errline) {
                    c = '-';
                }
                errline.back() = '^';
                std::cout << line << std::endl;
                std::cout << errline << std::endl;
            } catch (std::exception& e) {
                std::cout << "Generic Error: " << e.what() << std::endl;
            }
        }
    }
    return 0;
}
