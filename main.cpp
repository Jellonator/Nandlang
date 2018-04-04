#include "parse.h"
#include "state.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>

/// Print out the given token block
void printBlock(const TokenBlock& block)
{
    for (const auto& t : block) {
        switch (t.getSymbol()) {
        case Symbol::BLOCK:
            std::cout << "{" << std::endl;
            printBlock(t.getBlock());
            std::cout << "}" << std::endl;
            break;
        case Symbol::PARENTHESIS:
            std::cout << "(";
            printBlock(t.getBlock());
            std::cout << ")";
            break;
        case Symbol::IDENTIFIER: std::cout << t.getIdentifier() << " "; break;
        case Symbol::LINESEP:    std::cout << ";" << std::endl; break;
        case Symbol::LITERAL:    std::cout << t.getValue();     break;
        case Symbol::FUNCTION:   std::cout << "function "; break;
        case Symbol::WHILE:      std::cout << "while ";    break;
        case Symbol::IF:         std::cout << "if ";       break;
        case Symbol::VAR:        std::cout << "var ";      break;
        case Symbol::COMMA:      std::cout << ","; break;
        case Symbol::IOSEP:      std::cout << ":"; break;
        case Symbol::ASSIGN:     std::cout << "="; break;
        case Symbol::NAND:       std::cout << "!"; break;
        }
    }
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
            try {
                TokenBlock block = parseTokens(stream);
                printBlock(block);
                State state;
                state.parse(std::move(block));
                state.getFunction("main").call(state);
                std::cout << "Execution successful!" << std::endl;
            } catch (std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
    }
    return 0;
}
