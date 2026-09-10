#!/usr/bin/env python3
"""Check the defaulted-late initialization rule without reading indeterminate values."""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
source = ROOT/'student.tests/pa12/zero-defaulted-late.cpp'
with tempfile.TemporaryDirectory(prefix='pa12-zero-rule-') as directory:
    ir = Path(directory)/'out.lowir'
    subprocess.run([str(ROOT/'dev/cppgm++'), '--emit-lowir', '-O0', '--validate-lowir',
                    '-o', str(ir), str(source)], check=True)
    functions = dict(re.findall(r'^function @([^ (]+)[^\n]*\{\n(.*?)^\}', ir.read_text(), re.M | re.S))
    assert 'zeroinit' not in functions['make_late'], 'user-provided constructor acquired a zeroing prelude'
    assert 'zeroinit 8x8' in functions['make_early'], 'first-declaration defaulting lost value-initialization zeroing'
print('defaulted-late zero-initialization property: pass')
