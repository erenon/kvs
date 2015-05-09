Import('env')

#
# KVS library
#

env.SharedLibrary('lib/KVS', Glob('src/lib/kvs/*.cpp'))

#
# KVS Server
#

serverLibs = env['LIBS'] + ['KVS', 'boost_system']

env.Program('bin/kvsServer', Glob('src/bin/server/*.cpp'), LIBS = serverLibs)

# 
# Tests
#

testLibs = env['LIBS'] + ['KVS', 'boost_unit_test_framework', 'boost_thread', 'boost_system']

testDefines = env['CPPDEFINES'] + ['BOOST_TEST_DYN_LINK', 'BOOST_TEST_MAIN']

testPrograms = [
  'IntegrationTest',
  'StoreTest',
  'ValueTest'
]

for test in testPrograms:
  env.Program(
    'test/' + test, 'src/test/' + test + '.cpp', 
    CPPDEFINES = testDefines,
    LIBS = testLibs,
  )

env.SharedLibrary('test/BackupProcedure', 'src/test/BackupProcedure.cpp', LIBS = testLibs)
