#!/usr/bin/env python3
"""Frozen-binary A/A, B/B and ABBA compiler measurements with exact outputs."""
import hashlib
import json
import pathlib
import platform
import subprocess
import sys
import time

base, final, inputs, output = map(pathlib.Path, sys.argv[1:5])
variants = {'A': base.resolve(), 'B': final.resolve()}
work = output.parent / 'measure-work'; work.mkdir(parents=True, exist_ok=True)

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def text_size(path):
    lines = subprocess.check_output(['size', '-A', path], text=True).splitlines()
    return sum(int(line.split()[1]) for line in lines if line.startswith('.text '))

result = {
    'protocol': 'two A/A pairs; B/B; two ABBA blocks; wall includes startup, excludes hashing; /usr/bin/time RSS',
    'flags': ['--emit-ast', '--stats', '-o', '<same-output>', '<fixed-input>'],
    'host_build': '-std=gnu++11 -Wall -O3 (dev/Makefile); TEST_RUNNER_ENABLE',
    'host_cxx': subprocess.check_output(['g++','--version'],text=True).splitlines()[0],
    'machine': platform.platform(),
    'binaries': {key: {'sha256': sha(path), 'text_bytes': text_size(path)} for key,path in variants.items()},
    'inputs': {p.name: {'sha256': sha(p), 'bytes': p.stat().st_size} for p in sorted(inputs.glob('*.cpp'))},
    'observations': [], 'startup': [],
    'generated_program_runtime': None, 'generated_program_text_bytes': None,
    'budgets': {'paired_regression_percent': 10, 'rss_percent': 15, 'rss_allowance_kib': 1024,
                'host_text_growth_percent': 15, 'fourfold_wall_ratio': 6, 'fourfold_rss_ratio': 5},
}
# A direct empty TU measures compiler startup with the same flags and time wrapper.
empty = work/'empty.cpp'; empty.write_text('')
art, rss = work/'tree.ast', work/'rss.txt'

def observe(label, source):
    start = time.perf_counter_ns()
    proc = subprocess.run(['/usr/bin/time','-f','%M','-o',rss,
                           variants[label],'--emit-ast','--stats','-o',art,source],
                          stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    wall = (time.perf_counter_ns()-start)/1e9
    assert proc.returncode == 0, (label, source, proc.stderr)
    stats = json.loads(proc.stderr)
    return {'variant': label, 'wall_s': wall, 'rss_kib': int(rss.read_text()),
            'output_sha256': sha(art), 'output_bytes': art.stat().st_size, 'stats': stats}

for label in ['A','B']*4:
    result['startup'].append(observe(label,empty))
for source in sorted(inputs.glob('*.cpp')):
    expected = None
    for block, order in [('AA1','AA'),('AA2','AA'),('BB','BB'),('ABBA1','ABBA'),('ABBA2','ABBA')]:
        for index,label in enumerate(order):
            row = observe(label,source)
            if expected is None: expected = row['output_sha256']
            assert row['output_sha256'] == expected, (source.name, label, 'output mismatch')
            row.update(input=source.name,block=block,ordinal=index)
            result['observations'].append(row)
        print(source.name,block,'complete',flush=True)
        output.write_text(json.dumps(result,indent=2)+'\n')
print('Saved',len(result['observations']),'observations to',output,flush=True)
