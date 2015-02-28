Import('env')

env.SharedLibrary('lib/KVS', Glob('src/lib/kvs/*.cpp'))

#env['CPPPATH'].append('src/bin/')

serverLibs = env['LIBS'] + ['KVS']

env.Program('bin/kvsServer', Glob('src/bin/server/*.cpp'), LIBS = serverLibs)
