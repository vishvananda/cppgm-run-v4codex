#!/usr/bin/env python3
"""Itanium 5.1.8 closure signatures/discriminators, including fact roundtrips."""
from pathlib import Path
import json, subprocess, sys

ROOT = Path(__file__).resolve().parents[2]
cc, api, work = map(Path, sys.argv[1:4])
cc, api = cc.resolve(), api.resolve()
work.mkdir(parents=True, exist_ok=True)
cases = [
    ('type lambda-closure C first', 'Z4hostvEUlvE_'),
    ('type lambda-closure C 0', 'Z4hostvEUlvE0_'),
    ('type lambda-closure C first int', 'Z4hostvEUliE_'),
    ('type lambda-closure C 0 int', 'Z4hostvEUliE0_'),
    ('type lambda-closure C first int ...', 'Z4hostvEUlizE_'),
    ('function lambda C first call int\nparam int\nqualifier const', '_ZZ4hostvENKUliE_clEi'),
    ('function lambda C 0 call int\nparam int\nqualifier const', '_ZZ4hostvENKUliE0_clEi'),
]
rows = []
for i, (fact, expected) in enumerate(cases):
    source = work / f'case-{i}.facts'
    source.write_text('let-context C function host\n' + fact + '\n')
    r = subprocess.run([api, source], capture_output=True, text=True, timeout=15)
    assert r.returncode == 0 and r.stdout == expected + '\n', (fact, r.stdout, r.stderr)
    rows.append(dict(fact=source.read_text(), expected=expected, passed=True))
source = work / 'source.cpp'
source.write_text('int host(){auto a=[]{return 2;};auto b=[](int n){return n+1;};'
                  'auto c=[](int n){return n+2;};return a()+b(1)+c(1);}'
                  'int main(){return host()!=7;}')
ir = source.with_suffix('.lowir')
subprocess.run([cc, '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, source], check=True)
required = ['_ZZ4hostvENKUlvE_clEv', '_ZZ4hostvENKUliE_clEi', '_ZZ4hostvENKUliE0_clEi']
for symbol in required:
    assert 'object=' + symbol + ',' in ir.read_text(), (symbol, ir.read_text())
exe = source.with_suffix('.exe')
subprocess.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir], check=True)
subprocess.run([exe], check=True)
rows.append(dict(source=source.read_text(), symbols=required, passed=True))
(work/'results.json').write_text(json.dumps(rows, indent=2)+'\n')
print('7 exact ABI fact roundtrips and source closure numbering/native check pass')
