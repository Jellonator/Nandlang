env = Environment()
env.Append(CXXFLAGS = '-std=c++14 -Wall -g')
# env.Append(CXXFLAGS = '-g -rdynamic')
# env.Append(LINKFLAGS = ['-rdynamic'])

env.MergeFlags(['!pkg-config libunwind --cflags --libs', '-rdynamic', '-ggdb'])

import os
env['ENV']['TERM'] = os.environ['TERM']

sources = [
    "block.cpp",
    "compiler.cpp",
    "debug.cpp",
    "expression.cpp",
    "function.cpp",
    "main.cpp",
    "parse.cpp",
    "state.cpp",
    "statement.cpp",
    "symbol.cpp",
]

program = env.Program(target='build/nandlang', source=sources)
