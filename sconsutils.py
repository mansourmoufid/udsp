#!/usr/bin/env python2

from os.path import basename
from re import match
from subprocess import Popen, PIPE as _PIPE


NM_MATCH = '(.+)\[(.*)\]: ([a-zA-Z0-9_]+) ([abdtABDTU]{1}) '
NM_MATCH += '([0-9a-fA-F]+) ([0-9a-fA-f]+)'


def symbols(file):
    nm_command = ['nm', '-A', '-P', basename(file)]
    p = Popen(nm_command, stdin=None, stdout=_PIPE, shell=False)
    out, err = p.communicate()
    for line in out.split('\n'):
        symbol = {}
        m = match(NM_MATCH, line)
        if m:
            symbol['file'] = m.group(1)
            symbol['object file'] = m.group(2)
            symbol['name'] = m.group(3)
            symbol['type'] = m.group(4)
            symbol['value'] = m.group(5)
            symbol['size'] = m.group(6)
            yield symbol


def global_text_symbols(file):
    global_text = lambda symbol: symbol['type'] == 'T'
    return filter(global_text, symbols(file))


def get_symbol_defines(target, source, env):
    source_file = str(source[0])
    defined_symbols = global_text_symbols(source_file)
    nameof = lambda sym: sym['name']
    defined_symbol_names = map(nameof, defined_symbols)
    desired_symbols = env.get('GETSYMBOLDEFINES', [])
    defines = {}
    for sym in defined_symbol_names:
        for name in desired_symbols:
            if sym.strip('_') == desired_symbols[name]:
                defines[name] = sym.lstrip('_')
    env.Append(CPPDEFINES=defines)
    return None


if __name__ == '__main__':
    pass
