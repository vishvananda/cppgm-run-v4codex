#!/usr/bin/env python3
"""Trace the same declaration/template from both compiler generations to ELF."""
import hashlib
import json
import pathlib
import subprocess
import sys

out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
compilers = dict(zip(('seed', 'self'), (pathlib.Path(p).resolve() for p in sys.argv[2:4])))
source = pathlib.Path('student.tests/pa33/audit-trace.cpp').resolve()
rows = []
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def run(args):
    args = list(map(str, args))
    result = subprocess.run(args, capture_output=True, text=True, timeout=90)
    rows.append(dict(command=args, status=result.returncode, stdout=result.stdout, stderr=result.stderr))
    (out/'commands.json').write_text(json.dumps(rows, indent=2)+'\n')
    assert result.returncode == 0, rows[-1]
    return result
def counters(result):
    return [{k:v for k,v in json.loads(line).items() if not k.endswith('_ms') and not k.endswith('_rss_kib')}
            for line in result.stderr.splitlines() if line.startswith('{')]
binding = dict(compilers={name:dict(path=str(p), sha256=sha(p)) for name,p in compilers.items()},
               source=dict(path=str(source), sha256=sha(source)), levels={})
for level in range(4):
    metrics = {}
    for name, compiler in compilers.items():
        flags = [f'-O{level}', '-gline-tables-only', '--stats']
        ir = out/f'{name}{level}.lowir'
        obj = out/f'{name}{level}.o'
        run([compiler, *flags, '--emit-lowir', '--validate-lowir', source, '-o', ir])
        metrics[name] = counters(run([compiler, *flags, '-c', source, '-o', obj]))
        replay = out/f'{name}{level}-replay.o'
        run([compiler, f'-O{level}', '-c', out/f'{name}0.lowir', '-o', replay])
        assert obj.read_bytes() == replay.read_bytes(), (name, level, 'replay')
        exe = out/f'{name}{level}'
        run(['g++', obj, '-o', exe])
        for args in ([], ['a'], ['a','b','c']):
            run([exe, *args])
        run(['objdump', '-dr', obj])
        run(['readelf', '-wf', obj])
    for suffix in ('.lowir', '.o'):
        assert (out/f'seed{level}{suffix}').read_bytes() == (out/f'self{level}{suffix}').read_bytes(), (level, suffix)
    assert metrics['seed'] == metrics['self'], (level, 'work counters', metrics)
    binding['levels'][level] = dict(counters=metrics['seed'], object_sha256=sha(out/f'seed{level}.o'))
for name, compiler in compilers.items():
    assert sha(compiler) == binding['compilers'][name]['sha256']
(out/'binding.json').write_text(json.dumps(binding, indent=2)+'\n')
print('source/template trace: O0–O3, identical seed/self LowIR, ELF, work counters; replay and 24 executions PASS')
