import os

# build with `scons --debug-build` for debug.
AddOption(
  '--debug-build',
  action='store_true',
  help='debug build',
  default=False
)

env = Environment(
  CPPPATH = ['src/lib/'],
  CPPDEFINES=[
    'BOOST_LOG_DYN_LINK'
  ],
  CXXFLAGS=Split('-std=c++11 -Wall -Wextra -Werror'),
  LIBS=['pthread', 'boost_log', 'boost_program_options']
)

if 'CXX' in os.environ:
  env['CXX'] = os.environ['CXX']

#env.Append( LINKFLAGS = Split('-z origin') )
env.Append( RPATH = env.Literal(os.path.join('\\$$ORIGIN', os.pardir, 'lib')))

build_dir = 'build/' + env['CXX'] + '/'

if GetOption('debug_build'):
  variant_dir = build_dir + 'debug'
  env['CXXFLAGS'] += ['-Og']
else:
  variant_dir = build_dir + 'release'
  env['CXXFLAGS'] += ['-O2']

env['CCFLAGS'] += ['-g']
env['LIBPATH'] = os.path.join(variant_dir, 'lib')

SConscript(
  dirs='.',
  variant_dir=variant_dir,
  duplicate=False,
  exports="env"
)

