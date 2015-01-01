env = Environment()

# compile coffeescript
import glob
from subprocess import call
for file in glob.glob('src/coffee/*.coffee'):
    print "compiling " + file + "..."
    call(['coffee', '-o', '.', '-c', file])

# output object files to 'bin' folder
env.binary_dir = 'bin'
env.source_dir = 'src/c++'
env.executable_dir = "."
env.executable_name = "createStream"
env.VariantDir(env.binary_dir,env.source_dir,duplicate = 0)  # output to binary
# folder without copying files
env.output_executable = \
  env.Program(env.executable_name,Glob(env.binary_dir + '/*.cpp'))

# defines
# debug/release
debug = ARGUMENTS.get('debug',0) # defaults to debug configuration
release = ARGUMENTS.get('release',0)
if (int(debug) or not int(release)): # default to debug
    env.build_mode = 'DEBUG'
else:
    env.build_mode = 'RELEASE'
env.Append(CPPDEFINES = env.build_mode)

# extra warnings for cleanup before release
cleanup = ARGUMENTS.get('cleanup',0)
if int(cleanup):
    env.cleanup_mode = 'CLEAN'
else:
    env.cleanup_mode = 'UNCLEAN'
env.Append(CPPDEFINES = env.cleanup_mode)

# flags
env.Append(CXXFLAGS = ['-std=c++14'])
env.Append(LINKFLAGS = ['-pthread'])
# optimization for debug vs release
if (env.build_mode == 'DEBUG'):
    env.Append(CXXFLAGS = ['-O0','-ggdb','-g3','-Wall','-Wextra','-Werror'])
    if (env.cleanup_mode == 'CLEAN'):
        env.Replace(CXX = 'g++') # because most of these only work with gcc
        env.Append(CXXFLAGS = ['-Wundef','-Wshadow','-Wformat=2',
                              '-Wpointer-arith','-Wcast-align',
                              '-Wswitch-default',
                              '-Wswitch-enum','-Wunused','-Wstrict-overflow=5',
                              '-Wmissing-format-attribute',
                              '-Wsuggest-attribute=pure',
                              '-Wsuggest-attribute=const',
                              '-Wsuggest-attribute=noreturn','-Wtrampolines',
                              '-Wtype-limits',
                              '-Wcast-qual','-Wcast-align','-Wconversion',
                              '-Wlogical-op',
                              '-Wmissing-declarations',
                              '-Wmissing-field-initializers','-Wpacked',
                              '-Wredundant-decls',
                              '-Winline','-Wvector-operation-performance',
                              '-Wdisabled-optimization',
                              '-Wunused-parameter','-Wuninitialized'])
    else:
        env.Replace(CXX = 'clang++')
elif (env.build_mode == 'RELEASE'):
    env.Replace(CXX = 'g++')
    env.Append(CXXFLAGS = ['-Ofast','-finline-functions',
                          '-funsafe-loop-optimizations',
                          '-Wunsafe-loop-optimizations',
                          '-funroll-loops']) # TODO: ADD MARCH OPTIONS
    env.Append(LINKFLAGS = ['-s'])           # remove symbol infos
    if (env.cleanup_mode == 'CLEAN'):
        env.Append(CXXFLAGS = ['-Wdisabled-optimization'])

# add gmp
# env.Append(LIBS = ['gmp'])
