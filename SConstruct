import os
from os.path import basename

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

default_cflags = [
    '-Wall',
    '-Wextra',
    '-pedantic',
    '-std=c99',
]

conf = Configure(env)
cc = basename(os.environ.get('CC', ''))
if cc:
    conf.env.Replace(CC=os.environ['CC'])
if os.environ.get('CFLAGS'):
    conf.env.MergeFlags({'CFLAGS': os.environ['CFLAGS'].split()})
if cc in ['gcc', 'clang', 'ccc-analyzer']:
    conf.env.MergeFlags({'CFLAGS': default_cflags})
if os.environ.get('FORTRAN'):
    conf.env.Replace(FORTRAN=os.environ['FORTRAN'])
if os.environ.get('FORTRANFLAGS'):
    conf.env.Replace(FORTRANFLAGS=os.environ['FORTRANFLAGS'])
if conf.env.get('FORTRAN') in ['gfortran']:
    for option in ['-std=legacy']:
        if not option in conf.env.get('FORTRANFLAGS'):
            conf.env.Append(FORTRANFLAGS=' ' + option)
if os.environ.get('ARCHFLAGS'):
    conf.env.MergeFlags({'CFLAGS': os.environ['ARCHFLAGS'].split()})
    conf.env.MergeFlags({'FORTRANFLAGS': os.environ['ARCHFLAGS'].split()})
    conf.env.MergeFlags({'LINKFLAGS': os.environ['ARCHFLAGS'].split()})
for header in c_headers:
    if not conf.CheckCHeader(header):
        Exit(1)
if not conf.CheckLibWithHeader('m', 'math.h', 'c'):
    Exit(1)
env = conf.Finish()

fftpack = SConscript(['fftpack/SConscript'], exports='env')
fftpack_defines = env.SymDefines(None, fftpack, env)
env.Append(LIBS=[fftpack, 'm'])
env.Append(LIBPATH='#/fftpack')

udsp_src = ['fltop.c', 'udsp.c']
udsp = env.StaticLibrary('udsp', udsp_src)
test_udsp = env.Program('test-udsp', udsp_src + ['test-udsp.c', 'nclock.c'])
Depends('udsp.c', fftpack_defines)

Export('env')

Default([test_udsp])

env.Alias('test', [test_udsp])
