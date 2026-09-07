#!/usr/bin/env python3
"""Frozen ABI compiler measurements: AAAA calibration, two ABBA blocks.
No reference compiler, demangler or host object output is an ABI oracle.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser()
p.add_argument('--baseline', required=True)
p.add_argument('--baseline-commit', default='df7dbb00a')
p.add_argument('--candidate', default=str(ROOT / 'dev/abimangle'))
p.add_argument('--directory', default='/tmp/pa9-evidence/performance')
p.add_argument('--report', default=str(ROOT / 'student.tests/pa9/performance.json'))
a = p.parse_args()
work = Path(a.directory); work.mkdir(parents=True, exist_ok=True)
if (work / 'protocol.json').exists():
    raise SystemExit('use a fresh directory to preserve frozen observations')
run_state = subprocess.run(['git', 'diff', '--quiet', 'HEAD', '--', 'dev'], cwd=ROOT)
assert run_state.returncode == 0, 'commit implementation before freezing B'
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def run(command, **kw): return subprocess.run(command, check=True, **kw)
def text_size(path):
    # This measures the compiler executable, never a live ABI-name oracle.
    return int(subprocess.check_output(['size', str(path)], text=True).splitlines()[1].split()[0])

binaries = {}
for label, path in [('A', a.baseline), ('B', a.candidate)]:
    frozen = work / ('abimangle-' + label)
    shutil.copy2(path, frozen)
    binaries[label] = {'path': str(frozen), 'sha256': sha(frozen), 'compiler_text_bytes': text_size(frozen)}
cpu = min(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
# Budgets are fixed before any timed observations. These cover required
# serializer/telemetry growth as well as the modifier/substitution changes.
budgets = dict(wall_ratio=1.25, rss_ratio=1.20, rss_slack_kib=16384,
               compiler_text_growth_bytes=102400, scale_wall_ratio=5.5,
               scale_work_ratio=4.5, scale_rss_ratio=5.0)
report = dict(protocol='AAAA, ABBA, ABBA; paired block means and min/max spread',
              cpu=cpu, platform=platform.platform(), flags='-std=gnu++11 -Wall -O3',
              compile_configuration=(ROOT / 'obj/dev/.compile_config').read_text(),
              test_runner_configuration=(ROOT / 'obj/dev/.test_runner_mode').read_text(),
              host_compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
              baseline_commit=a.baseline_commit, candidate_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
              source_trees={label: subprocess.check_output(['git', 'rev-parse', rev + ':dev'], cwd=ROOT, text=True).strip()
                            for label, rev in [('A', a.baseline_commit), ('B', 'HEAD')]},
              binaries=binaries, budgets=budgets, generated_program_runtime=None,
              generated_program_text_size=None, executable_boundary='PA9 emits ABI names only', workloads={}, observations=[])
(work / 'protocol.json').write_text(json.dumps(report, indent=2) + '\n')

def corpus(kind, n):
    if kind == 'modifiers':
        return ''.join(f'case c{i}\ntype ' + 'ptr:const:' * 128 + 'int\n' for i in range(n))
    if kind == 'batch':
        case = ('let-arg I type int\nlet-type B template ns::Box I\n'
                'let-context C function path ns::host int\nlet-type L local-type C Local 0\n'
                'function use B B L L\n')
        return ''.join(f'case c{i}\n' + case for i in range(n))
    lines = []
    for i in range(n):
        if kind == 'templates':
            lines += [f'let-arg a{i} value ulong {i}', f'let-type t{i} template bench::Box a{i}']
        else:
            lines += [f'let-expr v{i} literal {i}', f'let-expr x{i} binary pl p v{i}', f'let-type t{i} decltype x{i}']
    prefix = 'let-expr p function-param 0\n' if kind == 'expressions' else ''
    return prefix + '\n'.join(lines) + '\nfunction bench::use\n' + ''.join(f'param t{i}\n' for i in range(n))

# Enough work to make process startup a small fraction even at the small size.
sizes = {'templates': 96000, 'expressions': 72000, 'modifiers': 9600, 'batch': 24000}
for kind, n in sizes.items():
    for scale in (1, 4):
        name = f'{kind}-{scale}'
        path = work / (name + '.facts'); path.write_text(corpus(kind, n * scale))
        outputs = {}
        for label in binaries:
            out = work / (name + '.' + label + '.names')
            run([binaries[label]['path'], '-o', str(out), str(path)], capture_output=True)
            outputs[label] = sha(out)
        assert outputs['A'] == outputs['B'], (name, 'A/B output mismatch')
        stats_run = run([binaries['B']['path'], '--stats', '-o', str(work / 'stats.names'), str(path)], capture_output=True, text=True)
        assert sha(work / 'stats.names') == outputs['B']
        counters = {k: int(v) for k, v in re.findall(r'(\w+)=(\d+)', stats_run.stderr)}
        report['workloads'][name] = dict(path=str(path), sha256=sha(path), input_bytes=path.stat().st_size,
                                       output_sha256=outputs['B'], output_bytes=(work / 'stats.names').stat().st_size, counters=counters)
        for block, sequence in enumerate(('AAAA', 'ABBA', 'ABBA')):
            for position, label in enumerate(sequence):
                rss = work / 'rss'; output = work / 'timed.names'
                start = time.perf_counter_ns()
                result = run(['/usr/bin/time', '-f', '%M', '-o', str(rss), binaries[label]['path'],
                              '-o', str(output), str(path)], capture_output=True)
                wall = (time.perf_counter_ns() - start) / 1e9
                output_digest = sha(output)
                assert output_digest == outputs[label]
                report['observations'].append(dict(workload=name, block=block, position=position,
                    binary=label, wall_seconds=wall, peak_rss_kib=int(rss.read_text()), exit_code=result.returncode,
                    output_sha256=output_digest))
        print('measured', name, flush=True)
        Path(a.report).write_text(json.dumps(report, indent=2) + '\n')

summary = {}
for name, workload in report['workloads'].items():
    rows = [r for r in report['observations'] if r['workload'] == name]
    noise = [r['wall_seconds'] for r in rows if r['block'] == 0]
    paired = []
    for block in (1, 2):
        aa = [r['wall_seconds'] for r in rows if r['block'] == block and r['binary'] == 'A']
        bb = [r['wall_seconds'] for r in rows if r['block'] == block and r['binary'] == 'B']
        paired.append(statistics.mean(bb) / statistics.mean(aa))
    selected = {k: [r for r in rows if r['block'] > 0 and r['binary'] == k] for k in 'AB'}
    summary[name] = dict(paired_wall_ratios=paired, mean_paired_wall_ratio=statistics.mean(paired),
        aa_noise_range_seconds=[min(noise), max(noise)],
        wall_median_seconds={k: statistics.median(r['wall_seconds'] for r in v) for k,v in selected.items()},
        wall_range_seconds={k: [min(r['wall_seconds'] for r in v), max(r['wall_seconds'] for r in v)] for k,v in selected.items()},
        peak_rss_median_kib={k: statistics.median(r['peak_rss_kib'] for r in v) for k,v in selected.items()})
report['summary'] = summary
report['budget_checks'] = {}
for name, s in summary.items():
    report['budget_checks'][name + '-latency'] = s['mean_paired_wall_ratio'] <= budgets['wall_ratio']
    report['budget_checks'][name + '-rss'] = s['peak_rss_median_kib']['B'] <= s['peak_rss_median_kib']['A'] * budgets['rss_ratio'] + budgets['rss_slack_kib']
for kind in sizes:
    small, large = summary[kind + '-1'], summary[kind + '-4']
    report['budget_checks'][kind + '-wall-scale'] = large['wall_median_seconds']['B'] / small['wall_median_seconds']['B'] <= budgets['scale_wall_ratio']
    report['budget_checks'][kind + '-rss-scale'] = large['peak_rss_median_kib']['B'] / small['peak_rss_median_kib']['B'] <= budgets['scale_rss_ratio']
    for counter in ('intern_requests', 'intern_probes', 'substitution_lookups', 'emitted_nodes'):
        c1, c4 = (report['workloads'][kind + '-' + str(scale)]['counters'][counter] for scale in (1, 4))
        report['budget_checks'][kind + '-' + counter + '-scale'] = c4 <= c1 * budgets['scale_work_ratio']
report['budget_checks']['compiler-text-growth'] = binaries['B']['compiler_text_bytes'] - binaries['A']['compiler_text_bytes'] <= budgets['compiler_text_growth_bytes']
report['budget_checks']['total-stage-text-growth'] = binaries['B']['compiler_text_bytes'] - 172418 <= budgets['compiler_text_growth_bytes']
Path(a.report).write_text(json.dumps(report, indent=2) + '\n')
failed = [k for k,v in report['budget_checks'].items() if not v]
print('budget failures:', failed)
if failed: raise SystemExit(1)
