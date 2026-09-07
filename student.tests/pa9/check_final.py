#!/usr/bin/env python3
"""Reduced final-audit defects, ABI grammar boundaries and controlled rejection."""
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser()
p.add_argument('--tool', default=str(ROOT / 'dev/abimangle'))
p.add_argument('--api', default='/tmp/pa9-evidence/check-api')
a = p.parse_args()
valid = [
    ('let-expr A value uint -1\nlet-expr B value uint 4294967295\n'
     'let-type X decltype A\nlet-type Y decltype B\nfunction f X Y\n',
     '_Z1fDTLj4294967295EES_\n'),
    ('let-expr A value bool 2\nlet-expr B value bool 1\n'
     'let-type X decltype A\nlet-type Y decltype B\nfunction f X Y\n',
     '_Z1fDTLb1EES_\n'),
    ('let-context C function host\ntype local-type C L 10\n', 'Z4hostvE1L_9\n'),
    ('let-context C function host\ntype local-type C L 11\n', 'Z4hostvE1L__10_\n'),
    ('let-context C function host\ntype local-type C L 18446744073709551615\n',
     'Z4hostvE1L__18446744073709551614_\n'),
    ('function f\ncomponent-abi-tag a\n', '_Z1fB1av\n'),
    ('let-type F function-type int\nlet-type C const F\nfunction f C F C F\n',
     '_Z1fKFivEFivES_S0_\n'),
    ('type function-type-qualified 5 false int\n', 'KFivRE\n'),
]
invalid = ['type ' + constructor * 1500 + tail + '\n' for constructor, tail in (
    ('vendor v ', 'int'), ('function-type ', 'int'), ('array 1 ', 'int'),
    ('builtin-transform T ', 'int'), ('member ', 'C' + ' m' * 1500),
    ('ptr ', 'int'))]
# Compact modifiers are deliberately iterative and remain supported at 20k.
invalid += ['type tagged int tag\n', 'type function-type-qualified 12 false int\n']
ctx = 'let-context c0 function host\n'
for n in range(1, 1500):
    ctx += f'let-context c{n} function local c{n-1} L f 0\n'
invalid.append(ctx + 'type local-type c1499 L 0\n')
ext = 'let-entity e0 function host\n'
for n in range(1, 1500):
    ext += f'let-arg a{n} entity-reference e{n-1}\nlet-entity e{n} function use a{n}\n'
invalid.append(ext + 'function use a1499\n')
with tempfile.TemporaryDirectory(prefix='pa9-final-probes-') as tmp:
    source, output = Path(tmp) / 'input.facts', Path(tmp) / 'names'
    for text, expected in valid:
        source.write_text(text)
        subprocess.run([a.tool, '-o', str(output), str(source)], check=True, capture_output=True, timeout=15)
        assert output.read_text() == expected, (text, output.read_text(), expected)
        result = subprocess.run([a.api, str(source)], check=True, capture_output=True, text=True, timeout=15)
        assert result.stdout == expected
    for text in invalid:
        source.write_text(text)
        for command in ([a.tool, '-o', str(output), str(source)], [a.api, str(source)]):
            result = subprocess.run(command, capture_output=True, timeout=15)
            assert result.returncode == 1, (command, result.returncode, result.stderr[:200])
print(f'final audit: {len(valid)} exact/roundtrip probes, {len(invalid)} controlled rejections pass')
