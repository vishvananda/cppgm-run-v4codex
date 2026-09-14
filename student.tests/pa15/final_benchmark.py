#!/usr/bin/env python3
"""Remeasure frozen whole-stage owner corpora against the final-audit entry.

All selected inputs are correct on both compilers. Preserve historical artifacts;
copy their sources and check exact LowIR/native parity before separate timing.
"""
from pathlib import Path
import json
import os
import platform
import statistics
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'student.tests/pa10'))
import benchmark as shared

A, B, WORK, OUT = [Path(p).resolve() for p in sys.argv[1:5]]
selected_names = set(sys.argv[5:])
ART = Path(os.environ['RALPH_ARTIFACT_DIR'])
WORK.mkdir(parents=True, exist_ok=True)
cpu = max(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
SELECTION = [
    (ROOT / 'student.tests/pa15/execution-performance.json',
     ['calls-1000', 'calls-4000', 'receivers-4000', 'dormant-4000', 'storage-4000', 'runtime-constant']),
    (ART / 'pa15-audit/checkpoint-repeat-performance.json',
     ['nested-packs-1000', 'nested-packs-4000', 'pack-targets-4000', 'pack-selections-4000', 'defaults-4000', 'values-4000']),
    (ART / 'pa15-loop33/performance-final.json',
     ['class-patterns-4000', 'ordered-patterns-4000', 'alias-overloads-4000', 'runtime-class-pattern']),
    (ROOT / 'student.tests/pa15/initialization-performance.json',
     ['arrays-4000', 'derived-4000', 'updates-4000', 'runtime-array-4', 'runtime-array-8', 'runtime-array-16', 'runtime-array-64']),
]
result = dict(
    protocol='one warmup each; four A/A observations; two ABBA blocks; separate compiler and native timing; usage parsing and hashing outside timer',
    flags=['--emit-lowir', '-O0'], build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
    cpu=cpu, platform=platform.platform(),
    implementation_commit=shared.run(['git', 'rev-parse', 'HEAD']).stdout.strip(),
    source_diff=shared.run(['git', 'diff', 'HEAD', '--', 'dev']).stdout,
    harness_sha256=shared.sha(__file__), shared_harness_sha256=shared.sha(ROOT / 'student.tests/pa10/benchmark.py'),
    binaries=[dict(path=str(p), sha256=shared.sha(p), text_bytes=shared.text_size(p)) for p in (A, B)],
    backend=dict(path=str(ROOT / 'reference-binaries/lowir2native'),
                 sha256=shared.sha(ROOT / 'reference-binaries/lowir2native'), flags=['-O0']),
    acceptance='PA15/O0 correctness; no numerical latency/RSS/text exit gate; fixed 32-byte backing policy; no added optional transform',
    workloads={}, started_utc=time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime()))

def save():
    OUT.write_text(json.dumps(result, indent=2) + '\n')

def observe(command):
    usage = WORK / 'usage.txt'
    begin = time.perf_counter_ns()
    shared.run(['/usr/bin/time', '-f', '%M %U %S %c %w', '-o', usage, *command])
    elapsed = (time.perf_counter_ns() - begin) / 1e9
    rss, user, system, involuntary, voluntary = usage.read_text().split()
    return dict(wall_s=elapsed, rss_kib=int(rss), user_s=float(user), system_s=float(system),
                involuntary=int(involuntary), voluntary=int(voluntary), checked_exit=0)

def measure(commands):
    warmups = [dict(binary=i, **observe(c)) for i, c in enumerate(commands)]
    rows = [dict(binary=i, **observe(commands[i])) for i in shared.ORDER]
    return dict(warmups=warmups, observations=rows,
                aa_range_s=[min(r['wall_s'] for r in rows[:4]), max(r['wall_s'] for r in rows[:4])],
                paired_b_over_a=[statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 1) /
                                 statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 0) for k in (4, 8)])

for campaign_path, names in SELECTION:
    campaign = json.loads(campaign_path.read_text())
    for name in names:
        if selected_names and name not in selected_names:
            continue
        original = campaign['workloads'][name]
        source = Path(original['source_path'])
        assert shared.sha(source) == original['source_sha256']
        src = WORK / (name + '.cpp')
        src.write_bytes(source.read_bytes())
        item = dict(source_path=str(src), source_sha256=shared.sha(src), common_correct=True,
                    input_campaign=dict(path=str(campaign_path), sha256=shared.sha(campaign_path)), outputs=[])
        result['workloads'][name] = item
        commands, executables = [], []
        for i, compiler in enumerate((A, B)):
            ir = WORK / (name + f'-{i}.lowir')
            command = [compiler, '--emit-lowir', '-O0', '-o', ir, src]
            commands.append(command)
            checked = shared.run([*command, '--stats', '--validate-lowir'])
            out = dict(binary=i, path=str(ir), sha256=shared.sha(ir), bytes=ir.stat().st_size,
                       telemetry=[json.loads(s) for s in checked.stderr.splitlines()])
            item['outputs'].append(out)
            if 'runtime' in original:
                exe = WORK / (name + f'-{i}')
                shared.run([ROOT / 'dev/lowir2native-ref', '-O0', '-o', exe, ir])
                shared.run([exe])
                previous = original['outputs'][-1]['native']
                assert shared.sha(exe) == previous['sha256'], (name, 'changed native bytes')
                # The identical frozen ELF carries a previously verified text/
                # data split (arrays have data tails); never count data as text.
                out['native'] = dict(previous, path=str(exe))
                executables.append([exe])
        assert item['outputs'][0]['sha256'] == item['outputs'][1]['sha256'], name
        print(name, 'preflight', flush=True)
        save()
        item['compiler'] = measure(commands)
        if executables:
            item['runtime'] = measure(executables)
        for out in item['outputs']:
            assert shared.sha(out['path']) == out['sha256']
        save()
        print(name, 'measured', flush=True)
for binary in result['binaries']:
    assert shared.sha(binary['path']) == binary['sha256']
result['finished_utc'] = time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime())
save()
