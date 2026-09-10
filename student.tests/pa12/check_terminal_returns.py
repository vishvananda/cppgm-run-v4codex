#!/usr/bin/env python3
"""Validate branch cleanup placement and the conservative unknown-call boundary."""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='pa12-terminal-rule-') as directory:
    ir = Path(directory)/'out.lowir'
    subprocess.run([str(ROOT/'dev/cppgm++'), '--emit-lowir', '-O0', '--validate-lowir',
                    '-o', str(ir), str(ROOT/'student.tests/pa12/terminal-return-lifetimes.cpp')], check=True)
    functions = {name: (header, body) for name, header, body in re.findall(
        r'^function @([^ (]+)([^\n]*\{)\n(.*?)^\}', ir.read_text(), re.M | re.S)}
    def copy_block(function, mangled):
        callee = next(name for name, (header, _) in functions.items() if 'object='+mangled+',' in header)
        blocks = re.split(r'^  block ', functions[function][1], flags=re.M)
        return next(body for body in blocks if 'call void @'+callee+'(' in body)
    assert 'eh_try' not in copy_block('choose_plain','_ZN4ItemC1ERKS_'), 'scalar-only terminal transfer still installs a handler'
    assert 'eh_try' in copy_block('choose_checked','_ZN7CheckedC1ERKS_'), 'unknown call lost its conservative handler'
    # A nested conditional still feeds an outer transfer; it is not itself the
    # terminal return destination and retains its private lifetime selection.
    assert re.search(r'cmp ne u8 .*\n\s+store i64 ', functions['choose'][1]), 'nested conditional lost its lifetime selector'
print('terminal return and unknown-call properties: pass')
