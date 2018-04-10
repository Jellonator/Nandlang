import os

# Credit to Roald "swarminglogic" Fernandez for his SConstruct file from
# https://github.com/swarminglogic/scons-x-compile, which allows for cross
# compiling for Windows from Linux.

# Create environment
env = Environment()
flags = ['-std=c++14', '-Wall']

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
    flags.append('-O2')
    vardir += '/release'

if GetOption('crosswin64'):
    env['CXX'] = 'x86_64-w64-mingw32-g++'
    target = '../../../nandlang.exe'
    vardir += '/win64'
else:
    target = '../../../nandlang'
    vardir += '/linux'

env.MergeFlags(flags);
print(target);

# highlighting in terminal
env['ENV']['TERM'] = os.environ['TERM']
# Run src's SConstruct file
env.SConscript("src/SConstruct", {'env' : env, 'target': target},\
    variant_dir=vardir, duplicate=0)
