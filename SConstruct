from __future__ import print_function
import os
from platform import system
import subprocess

from sconsutils import get_symbol_defines

def run(target, source, env):
    tgt = str(target.pop().abspath)
    for src in source:
        src = str(src.abspath)
        print(os.path.basename(src), end=': ')
        with open(tgt, 'w+') as log:
            p = subprocess.Popen(src, stdout=log, stderr=log, shell=True)
            p.wait()
        if p.returncode == 0:
            print('pass')
        else:
            print('fail')
            Exit(1)

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
        conf.env.Replace(**{var: os.environ[var]})
cc = os.path.basename(conf.env['CC'])
if cc in ['gcc', 'clang', 'ccc-analyzer']:
    conf.env.MergeFlags(default_flags)
if cc in ['clang']:
    conf.env.MergeFlags({'CFLAGS': '-Weverything'})
for flags in ['CPPFLAGS', 'CFLAGS', 'FORTRANFLAGS']:
    conf.env.MergeFlags({flags: os.environ.get(flags, '').split()})
fortran = os.path.basename(conf.env['FORTRAN'])
if fortran in ['gfortran']:
    conf.env.MergeFlags({'FORTRANFLAGS': ['-std=legacy']})
for flags in ['CFLAGS', 'FORTRANFLAGS', 'LINKFLAGS']:
    conf.env.MergeFlags({flags: os.environ.get('ARCHFLAGS', '').split()})
conf.env.MergeFlags({'LINKFLAGS': os.environ.get('LDFLAGS', '').split()})
if not all(map(conf.CheckCHeader, c_headers)):
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
Depends(udsp, fftpack_defines)
test_udsp = debug_env.Program(
    'test-udsp',
    ['test-udsp.c', 'nclock.c'],
    LIBS=[udsp, fftpack] + ['m'] + librt,
)

Export('env')

tests = [test_udsp]
all = [udsp] + tests

Default(all)
env.Alias('all', all)

check = Command('check.log', tests, run)
AlwaysBuild(check)
env.Alias('check', check)
