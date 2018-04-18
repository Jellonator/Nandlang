import os

# Credit to Roald "swarminglogic" Fernandez for his SConstruct file from
# https://github.com/swarminglogic/scons-x-compile, which allows for cross
# compiling for Windows from Linux.

# Create variables
flags = ['-std=c++14', '-Wall']
linkflags = ['-static']
cxx = "g++"

# Create --crosswin64 option
AddOption('--crosswin64',
          action='store_true',
          help='cross-compiles for windows',
          default=False)

# Create --crosswin64 option
AddOption('--gdb',
          action='store_true',
          help='enables GDB debugging symbols',
          default=False)

# Determine compilation options
target = None;
vardir = 'build'

if GetOption('gdb'):
    flags.append('-ggdb')
    vardir += '/debug'
else:
    flags.append('-O3')
    linkflags.append('-s')
    vardir += '/release'
if GetOption('crosswin64'):
    cxx = 'x86_64-w64-mingw32-g++'
    target = '../../../nandlang.exe'
    vardir += '/win64'
else:
    target = '../../../nandlang'
    vardir += '/linux'

# Create environment
env = Environment(CXX=cxx, LINKFLAGS=linkflags, CXXFLAGS=flags)

# highlighting in terminal
env['ENV']['TERM'] = os.environ['TERM']
# Run src's SConstruct file
env.SConscript("src/SConstruct", {'env' : env, 'target': target},\
    variant_dir=vardir, duplicate=0)
