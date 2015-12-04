import os
from platform import system

from sconsutils import get_symbol_defines

env = Environment(ENV=os.environ)
env['BUILDERS']['SymDefines'] = Builder(action=get_symbol_defines)
env['GETSYMBOLDEFINES'] = {
    'RFFTI': 'rffti',
    'RFFTF': 'rfftf',
    'RFFTB': 'rfftb',
}

c_headers = [
    'assert.h',
    'float.h',
    'inttypes.h',
    'limits.h',
    'math.h',
    'stddef.h',
    'stdint.h',
    'stdio.h',
    'stdlib.h',
]

default_flags = {
    'CFLAGS': [
        '-Wall',
        '-Wextra',
        '-pedantic',
        '-std=c99',
    ],
    'CPPDEFINES': [
        ('_POSIX_C_SOURCE', '200809L'),
    ]
}

darwin_flags = {
    'CPPDEFINES': [
        ('_DARWIN_C_SOURCE', '1'),
    ],
}

debug_flags = {
    'CFLAGS': [
        '-g',
        '-O0',
    ],
}

conf = Configure(env)
for var in ['AR', 'CC', 'FORTRAN', 'NM', 'RANLIB']:
    if os.environ.get(var, None):
        name = os.path.basename(os.environ[var])
        conf.env.Replace(**{var: name})
if conf.env['CC'] in ['gcc', 'clang', 'ccc-analyzer']:
    conf.env.MergeFlags(default_flags)
if conf.env['CC'] in ['clang']:
    conf.env.MergeFlags({'CFLAGS': '-Weverything'})
for flags in ['CPPFLAGS', 'CFLAGS', 'FORTRANFLAGS']:
    conf.env.MergeFlags({flags: os.environ.get(flags, '').split()})
if os.environ.get('FORTRAN'):
    conf.env.Replace(FORTRAN=os.environ['FORTRAN'])
if conf.env['FORTRAN'] in ['gfortran']:
    conf.env.MergeFlags({'FORTRANFLAGS': ['-std=legacy']})
for flags in ['CFLAGS', 'FORTRANFLAGS', 'LINKFLAGS']:
    conf.env.MergeFlags({flags: os.environ.get('ARCHFLAGS', '').split()})
conf.env.MergeFlags({'LINKFLAGS': os.environ.get('LDFLAGS', '').split()})
for header in c_headers:
    if not conf.CheckCHeader(header):
        Exit(1)
if not conf.CheckLibWithHeader('m', 'math.h', 'c'):
    Exit(1)
if system() == 'Darwin':
    conf.env.MergeFlags(darwin_flags)
    if not conf.CheckCHeader('mach/mach_time.h'):
        Exit(1)
if system() == 'Linux':
    if not conf.CheckCHeader('time.h'):
        Exit(1)
librt = ['rt'] if system() == 'Linux' else []
env = conf.Finish()
debug_env = env.Clone()
debug_env.MergeFlags(debug_flags)

fftpack = SConscript(['fftpack/SConscript'], exports='env')
fftpack_defines = env.SymDefines(None, fftpack, env)
env.Append(LIBPATH='#/fftpack')

udsp = env.StaticLibrary('udsp', ['fltop.c', 'udsp.c'])
nclock = env.Object('nclock.c')
test_udsp = debug_env.Program('test-udsp', ['test-udsp.c', nclock],
                              LIBS=[udsp, fftpack] + ['m'] + librt)
Depends('udsp.c', fftpack_defines)

Export('env')

Default([test_udsp])

env.Alias('test', [test_udsp])
