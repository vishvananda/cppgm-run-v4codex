#!/usr/bin/env python3
"""Verify retained PA12 evidence and final frozen compiler/output campaigns."""
from pathlib import Path
import hashlib
import json
import math
import os
import statistics
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
ORDER = [0,0,0,0,0,1,1,0,0,1,1,0]
hashes = {}
counts = dict(campaigns=0, observations=0, file_hash_pairs=0, historical_outputs=0, reproduced_outputs=0)
recovery = Path(os.environ.get('RALPH_ARTIFACT_DIR', '/tmp'))/'pa12-final-audit'/'reproduced-evidence'

def sha(path):
    path = Path(path)
    if path not in hashes: hashes[path] = hashlib.sha256(path.read_bytes()).hexdigest()
    return hashes[path]

def check(path, digest):
    assert sha(path) == digest, (str(path), 'hash mismatch')
    counts['file_hash_pairs'] += 1

def walk(value):
    if isinstance(value, list):
        for child in value: walk(child)
    elif isinstance(value, dict):
        if 'wall_s' in value:
            assert value['wall_s'] > 0 and value.get('rss_kib', 1) > 0, value
            counts['observations'] += 1
        for path, digest in [('path','sha256'), ('source_path','source_sha256')]:
            if path in value and digest in value: check(value[path], value[digest])
        if 'paired_b_over_a' in value:
            rows = value['observations']
            label = 'binary' if 'binary' in rows[0] else 'compiler'
            assert [r[label] for r in rows] == ORDER, value
            actual = [statistics.mean(r['wall_s'] for r in rows[k:k+4] if r[label]==1)/
                      statistics.mean(r['wall_s'] for r in rows[k:k+4] if r[label]==0) for k in (4,8)]
            assert all(math.isclose(a,b,rel_tol=1e-12) for a,b in zip(actual,value['paired_b_over_a'])), value
        for child in value.values(): walk(child)

historical = [p for p in sorted(HERE.glob('*.json')) if not p.name.startswith('final-')]
for path in historical:
    data = json.loads(path.read_text()); counts['campaigns'] += 1; walk(data)
    if 'backend_sha256' in data: check(ROOT/'reference-binaries/lowir2native', data['backend_sha256'])
    for name, workload in data.get('workloads', {}).items():
        if 'source_path' not in workload: continue
        source = Path(workload['source_path'])
        candidates = [p for p in source.parent.glob(source.stem+'*') if p.is_file()]
        for record in workload.get('outputs', [workload]):
            missing = [field for field in ('lowir_sha256', 'executable_sha256')
                       if field in record and not any(sha(p) == record[field] for p in candidates)]
            if missing:
                # Historical scratch was reused after measurement. Reproduce in
                # a separate directory from the still-frozen compiler and input;
                # never replace the observation or silently accept a new digest.
                destination = recovery/path.stem/name/str(record['binary'])
                destination.mkdir(parents=True, exist_ok=True)
                ir, exe = destination/'out.lowir', destination/'out'
                compiler = data['binaries'][record['binary']]
                check(compiler['path'], compiler['sha256'])
                subprocess.run([compiler['path'], '--emit-lowir', '-O0', '--validate-lowir',
                                '-o', str(ir), str(source)], check=True, timeout=60)
                check(ir, record['lowir_sha256'])
                subprocess.run([str(ROOT/'dev/lowir2native-ref'), '-O0', '-o', str(exe), str(ir)],
                               check=True, timeout=60)
                check(exe, record['executable_sha256'])
                subprocess.run([str(exe)], check=True, timeout=60)
                candidates += [ir, exe]
                counts['reproduced_outputs'] += len(missing)
                print('reproduced original hashes:', path.name, name, record['binary'], str(destination))
            for field in ('lowir_sha256', 'executable_sha256'):
                if field not in record: continue
                assert any(sha(p) == record[field] for p in candidates), (path.name, source, field)
                counts['historical_outputs'] += 1
print('historical:', json.dumps(counts, sort_keys=True))

# Verify the native-layout causal control without running or changing its
# benchmark: only the recorded four addresses, segment sizes and data padding
# may differ. This is diagnostic evidence, never a production rewrite.
layout = json.loads((HERE/'return-layout-performance.json').read_text())
consumption = json.loads((HERE/'consumption-performance.json').read_text())
layout_dir = Path(consumption['workloads']['loop-runtime']['source_path']).parent
for record in layout['images']:
    original = layout_dir/('loop-runtime-'+str(record['compiler']))
    relocated = original.with_name(original.name+'-separated')
    check(original, record['original_sha256'])
    check(relocated, record['relocated_sha256'])
    check(original.with_suffix('.lowir'), record['lowir_sha256'])
    data = original.read_bytes(); expected = bytearray(data)
    for fixup in record['fixups']:
        at, size = fixup['file_offset'], fixup['bytes']
        fmt = '<i' if fixup['kind'] == 'rip-relative' else '<Q'
        assert struct.calcsize(fmt) == size
        struct.pack_into(fmt, expected, at, struct.unpack_from(fmt, data, at)[0]+64)
    phoff = struct.unpack_from('<Q', data, 32)[0]
    for at in (phoff+32, phoff+40):
        struct.pack_into('<Q', expected, at, struct.unpack_from('<Q', data, at)[0]+64)
    expected[len(data)-4:len(data)-4] = bytes(64)
    assert bytes(expected) == relocated.read_bytes(), original
print('native layout control: only recorded addresses, segment sizes and data padding differ')

final = json.loads((HERE/'final-audit-performance.json').read_text())
walk(final)
check(ROOT/'dev/cppgm++', final['binaries'][1]['sha256'])
check(HERE/'audit_benchmark.py', final['harness_sha256'])
check(ROOT/'student.tests/pa10/benchmark.py', final['shared_harness_sha256'])
check(ROOT/'reference-binaries/lowir2native', final['backend_sha256'])
executed = 0
for name, workload in final['workloads'].items():
    if len(workload['outputs']) == 2:
        assert workload['outputs'][0]['sha256'] == workload['outputs'][1]['sha256'], name
    for record in workload['outputs']:
        if 'native' not in record: continue
        native = record['native']
        assert native['checked_exit'] == 0
        subprocess.run([native['path']], check=True, timeout=60); executed += 1
    for kind in ('compiler', 'runtime'):
        if kind not in workload: continue
        rows = workload[kind]['observations']
        assert [r['binary'] for r in rows] == (ORDER if len(workload['outputs']) == 2 else [1]*6), name
print('final audit:', len(final['workloads']), 'workloads;', executed, 'executables checked again')
noise = json.loads((HERE/'final-noise-performance.json').read_text())
walk(noise)
check(HERE/'audit_noise.py', noise['harness_sha256'])
check(HERE/'final-audit-performance.json', noise['original_campaign_sha256'])
assert noise['binaries'] == final['binaries']
for name, workload in noise['workloads'].items():
    assert workload['source_sha256'] == final['workloads'][name]['source_sha256']
    assert [r['sha256'] for r in workload['outputs']] == [r['sha256'] for r in final['workloads'][name]['outputs']]
print('focused noise repeat: same frozen inputs, compilers and outputs; all observations retained')

sys.path.insert(0, str(ROOT/'student.tests/pa10'))
import benchmark as shared
initial = json.loads((HERE/'final-initial-audit-performance.json').read_text())
walk(initial)
initial_stage = json.loads((HERE/'final-initial-stage-performance.json').read_text())
walk(initial_stage)
shared.verify(initial_stage)
assert initial['binaries'][1]['sha256'] == initial_stage['binaries'][1]['sha256']
print('initial audit campaigns retained and verified after final projection follow-up')
stage = json.loads((HERE/'final-stage-performance.json').read_text())
walk(stage)
check(ROOT/'reference-binaries/lowir2native', stage['backend']['sha256'])
check(ROOT/'dev/lowir2native-ref', stage['backend']['wrapper_sha256'])
for path, digest in stage['source_hashes'].items(): check(ROOT/path, digest)
for entry in stage['inputs'].values(): assert entry['output_hashes'][0] == entry['output_hashes'][1]
for entry in stage['runtime']: assert entry['executables'][0]['sha256'] == entry['executables'][1]['sha256']
shared.verify(stage)
assert stage['binaries'][1]['sha256'] == final['binaries'][1]['sha256']
print('PASS: final compiler matches both frozen campaigns; all retained observations and checked hashes verified')
