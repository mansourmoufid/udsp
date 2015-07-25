import os
from os.path import basename
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
}

debug_flags = {
    'CFLAGS': [
        '-g',
        '-O0',
    ],
}

conf = Configure(env)
cc = basename(os.environ.get('CC', ''))
if cc:
    conf.env.Replace(CC=os.environ['CC'])
if cc in ['gcc', 'clang', 'ccc-analyzer']:
    conf.env.MergeFlags(default_flags)
if cc in ['clang']:
    conf.env.MergeFlags({'CFLAGS': '-Weverything'})
for flags in ['CPPFLAGS', 'CFLAGS']:
    conf.env.MergeFlags({flags: os.environ.get(flags, '').split()})
if os.environ.get('FORTRAN'):
    conf.env.Replace(FORTRAN=os.environ['FORTRAN'])
if conf.env.get('FORTRAN') in ['gfortran']:
    conf.env.MergeFlags({'FORTRANFLAGS': ['-std=legacy']})
for flags in ['CFLAGS', 'FORTRANFLAGS', 'LINKFLAGS']:
    conf.env.MergeFlags({flags: os.environ.get('ARCHFLAGS', '').split()})
conf.env.MergeFlags({'LINKFLAGS': os.environ.get('LDFLAGS', '').split()})
for header in c_headers:
    if not conf.CheckCHeader(header):
        Exit(1)
if not conf.CheckLibWithHeader('m', 'math.h', 'c'):
    Exit(1)
libs = ['m']
if system() == 'Linux':
    libs += ['rt']
env = conf.Finish()
debug_env = env.Clone()
debug_env.MergeFlags(debug_flags)

fftpack = SConscript(['fftpack/SConscript'], exports='env')
fftpack_defines = env.SymDefines(None, fftpack, env)
env.Append(LIBS=libs)
env.Append(LIBPATH='#/fftpack')

udsp_src = ['fltop.c', 'udsp.c']
udsp = env.StaticLibrary('udsp', udsp_src)
test_udsp = debug_env.Program('test-udsp', ['test-udsp.c', 'nclock.c'],
                              LIBS=libs + [fftpack, udsp])
Depends('udsp.c', fftpack_defines)

Export('env')

Default([test_udsp])

env.Alias('test', [test_udsp])
