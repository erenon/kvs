Import('env')

#
# KVS library
#

env.SharedLibrary('lib/KVS', Glob('src/lib/kvs/*.cpp'))

#
# KVS Server
#

serverLibs = env['LIBS'] + ['KVS']

env.Program('bin/kvsServer', Glob('src/bin/server/*.cpp'), LIBS = serverLibs)

# 
# Tests
#

testLibs = env['LIBS'] + ['KVS', 'boost_unit_test_framework']

testDefines = env['CPPDEFINES'] + ['BOOST_TEST_DYN_LINK', 'BOOST_TEST_MAIN']

testPrograms = [
  'IntegrationTest',
  'Value'
]

for test in testPrograms:
  env.Program(
    'test/' + test, 'src/test/' + test + '.cpp', 
    CPPDEFINES = testDefines,
    LIBS = testLibs,
  )
