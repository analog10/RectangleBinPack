Import('*')

libs = Split('benejson m')
bin_env = bin_env.Clone() 
srcs = Split('GuillotineBinPack.cpp Rect.cpp binjack.cpp')
prog = bin_env.Program('binjack', srcs, LIBS=libs)

bin_env.Install(bin_env.BinDest, prog)
