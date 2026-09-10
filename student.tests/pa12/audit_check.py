#!/usr/bin/env python3
"""Final audit: execution reducers plus qualifier and bounded-work properties."""
from pathlib import Path
import json
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
COMPILER = os.environ.get('CPPGM_AUDIT_COMPILER', ROOT/'dev/cppgm++')

def run(command):
    result = subprocess.run([str(x) for x in command], capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, (command, result.returncode, result.stderr)
    assert 'Sanitizer' not in result.stderr and 'runtime error:' not in result.stderr, result.stderr
    return result

with tempfile.TemporaryDirectory(prefix='pa12-final-check-') as directory:
    work = Path(directory)
    def compile(source):
        ir = work/'out.lowir'
        result = run([COMPILER, '--emit-lowir', '-O0', '--stats', '--validate-lowir', '-o', ir, source])
        stats = [json.loads(line) for line in result.stderr.splitlines()]
        return ir, ir.read_text(), {k: v for row in stats for k, v in row.items()}

    for source in sorted(HERE.glob('audit-*.cpp')):
        ir, text, stats = compile(source)
        exe = work/'out'
        run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); run([exe])
        if source.stem == 'audit-volatile-list':
            bodies = dict(re.findall(r'^function (@[^ (]+)[^\n]*\{\n(.*?)^}', text, re.M | re.S))
            def stores(name):
                body = bodies[name]
                return body.count('store volatile i32') + sum(stores(callee) for callee in
                    re.findall(r'call void (@[^ (]+)\(', body) if callee in bodies)
            assert stores('@main') == 3, text
        if source.stem == 'audit-heap-zero':
            assert 'store volatile i32 0,' in text, text
        print(source.name + ': validated, executed, properties pass')

    # A runtime extent must not create one IR fragment per heap element.
    counts = []
    for extent in (19, 1000000):
        source = work/'heap.cpp'
        source.write_text('struct V{int x;}; struct P{int V::*p;}; '
                          f'int main(){{P* p=new P[{extent}]();delete[] p;}}')
        counts.append(compile(source)[2]['instructions'])
    assert counts[0] == counts[1], counts
    print('heap typed-zero instruction counts:', counts)

    rows = []
    for count in (32, 128):
        source = work/'choices.cpp'
        cases = ''.join(f'n=={i}?V({i}).x:' for i in range(count)) + 'V(999).x'
        source.write_text('struct V{int x;V(int n):x(n){}~V(){}}; '
                          f'int run(int n){{const int& r={cases};return r;}}')
        rows.append(compile(source)[2])
    assert [r['semantic_reference_alternatives'] for r in rows] == [33, 129], rows
    for key in ('semantic_reference_binding_work', 'instructions'):
        assert rows[1][key] <= 4*rows[0][key], (key, rows)
    print('conditional reference work/instructions:',
          [(r['semantic_reference_binding_work'], r['instructions']) for r in rows])
