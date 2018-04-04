env = Environment()
env.Append(CXXFLAGS = '-std=c++14 -Wall')

import os
env['ENV']['TERM'] = os.environ['TERM']

sources = [
    "block.cpp",
    "compiler.cpp",
    "expression.cpp",
    "function.cpp",
    "main.cpp",
    "parse.cpp",
    "state.cpp",
    "statement.cpp",
    "symbol.cpp",
]

program = env.Program(target='build/nandlang', source=sources)
