import os

# Use current directory as root staging point.
rootStage = os.getcwd()

#Set default C++ building flags for both libraries and executables
commonEnv = Environment(ENV = os.environ)

# Files installed to VaultGard:
commonEnv.RootDest = rootStage + '/target'
commonEnv.BinDest = commonEnv.RootDest + '/bin'

# Add Include Destination to general include directories
# Compile with all warnings, no opti, debug
commonEnv.Append(CCFLAGS = ' -Wall -fpermissive')
#commonEnv.Append(CCFLAGS = '-O0 -g')
#commonEnv.Append(CCFLAGS = '-O2')
#commonEnv.Replace(LINK = 'ld')


# Set linking flags for executables
bin_env = commonEnv.Clone()
#bin_env.Append(LINKFLAGS = ['-Wl,--as-needed'])
#bin_env.Append(LINKFLAGS = ['-Wl,-z,lazy'])
#bin_env.Append(LINKFLAGS = ['-Wl,--unresolved-symbols=ignore-in-shared-libs'])

SConscript(Split('SConscript'),\
	exports='bin_env', build_dir='tmp', duplicate=0)
