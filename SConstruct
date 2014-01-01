import os
from sconsutils import get_symbol_defines

env = Environment()
env['BUILDERS']['SymDefines'] = Builder(action=get_symbol_defines)
env['GETSYMBOLDEFINES'] = {
    'RFFTI': 'rffti',
    'RFFTF': 'rfftf',
    'RFFTB': 'rfftb',
}

AddOption('--prefix',
          dest='prefix', type='string', nargs=1, action='store',
          metavar='PREFIX', default='/usr/local',
          help='Install files into PREFIX')
AddOption('--destdir',
          dest='destdir', type='string', nargs=1, action='store',
          metavar='DESTDIR', default='/',
          help='Install files into DESTDIR/PREFIX')

c_headers = [
    'assert.h',
    'float.h',
    'limits.h',
    'math.h',
    'stddef.h',
    'stdint.h',
    'stdio.h',
    'stdlib.h',
]

conf = Configure(env)
if os.environ.get('DESTDIR'):
    conf.env.Replace(DESTDIR=os.environ['DESTDIR'])
else:
    conf.env.Replace(DESTDIR=GetOption('destdir'))
if os.environ.get('PREFIX'):
    conf.env.Replace(PREFIX=os.environ['PREFIX'])
else:
    conf.env.Replace(PREFIX=GetOption('prefix'))
if os.environ.get('CC'):
    conf.env.Replace(CC=os.environ['CC'])
if os.environ.get('CFLAGS'):
    conf.env.Replace(CFLAGS=os.environ['CFLAGS'])
if conf.env.get('CC') in ['gcc', 'clang']:
    for option in ['-Wall', '-Wextra', '-pedantic', '-std=c99']:
        if not option in conf.env.get('CFLAGS'):
            conf.env.Append(CFLAGS=' ' + option)
if os.environ.get('FORTRAN'):
    conf.env.Replace(FORTRAN=os.environ['FORTRAN'])
if os.environ.get('FORTRANFLAGS'):
    conf.env.Replace(FORTRANFLAGS=os.environ['FORTRANFLAGS'])
if conf.env.get('FORTRAN') in ['gfortran']:
    for option in ['-std=legacy']:
        if not option in conf.env.get('FORTRANFLAGS'):
            conf.env.Append(FORTRANFLAGS=' ' + option)
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
libudsp = env.SharedLibrary('udsp', udsp_src)
test_udsp = env.Program('test-udsp', udsp_src + ['test-udsp.c'])
Depends('udsp.c', fftpack_defines)

Export('env')

Default([udsp, libudsp])

basedir = os.path.join(env['DESTDIR'],
                       env['PREFIX'].lstrip(os.path.sep))
libdir = os.path.join(basedir, 'lib')
includedir = os.path.join(basedir, 'include')

lib = env.Install(dir=libdir, source=libudsp)
h = env.Install(dir=includedir, source=['udsp.h'])
env.AddPreAction(lib, Chmod(libudsp, 0755))
env.AddPreAction(h, Chmod('udsp.h', 0644))

env.Alias('install', [libdir, includedir])
env.Alias('test', [test_udsp])
