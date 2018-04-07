env = Environment()
env.Append(CXXFLAGS = '-std=c++14 -Wall -g')
# env.Append(CXXFLAGS = '-g -rdynamic')
# env.Append(LINKFLAGS = ['-rdynamic'])

env.MergeFlags(['!pkg-config libunwind --cflags --libs', '-rdynamic', '-ggdb'])

import os
env['ENV']['TERM'] = os.environ['TERM']

sources = [
    "src/block.cpp",
    "src/compiler.cpp",
    "src/debug.cpp",
    "src/expression.cpp",
    "src/function.cpp",
    "src/main.cpp",
    "src/parse.cpp",
    "src/state.cpp",
    "src/statement.cpp",
    "src/symbol.cpp",
]

program = env.Program(target='build/nandlang', source=sources)
