env = Environment()
env.MergeFlags(['-std=c++14', '-Wall', '-O2']);

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
