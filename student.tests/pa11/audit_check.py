#!/usr/bin/env python3
"""Explicit PA11 audit reducers, executable results and initialization budgets."""
from pathlib import Path
import json
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
COMPILER = Path(os.environ.get('CPPGM_AUDIT_COMPILER', str(ROOT/'dev/cppgm++')))


def run(command, success=True):
    result = subprocess.run([str(x) for x in command], capture_output=True, text=True, timeout=60)
    assert 'Sanitizer' not in result.stderr and 'runtime error:' not in result.stderr, result.stderr
    assert (result.returncode == 0) == success, (command, result.returncode, result.stderr)
    return result


def main():
    with tempfile.TemporaryDirectory(prefix='audit-', dir=HERE) as directory:
        scratch = Path(directory)

        def compile(source, execute=True):
            ir, exe = scratch/'out.lowir', scratch/'out'
            result = run([COMPILER, '--emit-lowir', '-O0', '--validate-lowir', '--stats', '-o', ir, source])
            text = ir.read_text()
            if execute:
                run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir])
                run([exe])
            return text, [json.loads(line) for line in result.stderr.splitlines()]

        for source in sorted(HERE.glob('audit-*.cpp')):
            if source.stem.endswith('-bad') or source.stem == 'audit-bad-initializer':
                run([COMPILER, '--emit-lowir', '-O0', '-o', scratch/'bad.lowir', source], success=False)
            else:
                ir, stats = compile(source)
                if source.stem == 'audit-volatile-object':
                    assert ir.count('store volatile i32') == 3, ir
                if source.stem == 'audit-union-default':
                    assert 'store f64' not in ir, ir
                if source.stem == 'audit-nested-range':
                    assert stats[-1]['instructions'] < 30, stats
                if source.stem == 'audit-temporary-zero':
                    assert ir.count('zeroinit') == 1, ir
            print(source.name + ': PASS', flush=True)

        for volatile in ('', 'volatile '):
            observations = []
            for count in (32, 1000000):
                source = scratch/'range.cpp'
                source.write_text(f'struct A{{{volatile}int a[{count}];}};int main(){{A x[2]={{}};return x[1].a[{count-1}];}}')
                ir, stats = compile(source)
                assert ('zeroinit' not in ir) if volatile else ('zeroinit' in ir)
                observations.append(stats[-1]['instructions'])
            assert observations[0] == observations[1], observations
            print(('volatile' if volatile else 'ordinary') + ' nested ranges: ' + str(observations), flush=True)

        # The expansion budget applies to the product of nested repetitions.
        counts = []
        for depth in (2, 4, 6):
            source = scratch/'dimensions.cpp'
            source.write_text('int main(){volatile int a'+'[8]'*depth+'={};return a'+'[7]'*depth+';}')
            ir, stats = compile(source)
            counts.append(stats[-1]['instructions'])
            assert 'zeroinit' not in ir
        assert counts[-1] < 1000 and counts[2]-counts[1] <= counts[1]-counts[0], counts
        print('nested expansion budget: ' + str(counts), flush=True)


if __name__ == '__main__':
    main()
