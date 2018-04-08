import os

# Credit to Roald "swarminglogic" Fernandez for his SConstruct file from
# https://github.com/swarminglogic/scons-x-compile, which allows for cross
# compiling for Windows from Linux.

# Create environment
env = Environment()
env.MergeFlags(['-std=c++14', '-Wall', '-O2']);

# Create --crosswin64 option
AddOption('--crosswin64',
          action='store_true',
          help='cross-compiles for windows',
          default=False)

# Determine compilation options
target = None;
vardir = None
if GetOption('crosswin64'):
    env['CXX'] = 'x86_64-w64-mingw32-g++'
    target = '../../nandlang.exe'
    vardir = 'build/win64'
else:
    env['CXX']='g++'
    target = '../../nandlang'
    vardir = 'build/linux'
print(target);

# highlighting in terminal
env['ENV']['TERM'] = os.environ['TERM']
# Run src's SConstruct file
env.SConscript("src/SConstruct", {'env' : env, 'target': target},\
    variant_dir=vardir, duplicate=0)
