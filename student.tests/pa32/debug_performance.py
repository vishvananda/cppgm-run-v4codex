#!/usr/bin/env python3
"""Quantify final O3 source-snapshot costs: g0 versus line-table mode.
Both variants use the same frozen final compiler and checked executable input.
This is a required-metadata cost diagnostic, not an optimization speed claim.
"""
import hashlib
import json
import os
import pathlib
import statistics
import subprocess
import sys
import time

OUT = pathlib.Path(sys.argv[1]).resolve()
OUT.mkdir(parents=True, exist_ok=True)
BINARY = pathlib.Path(sys.argv[2]).resolve()
AFFINITY = ['taskset', '-c', os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(args):
    result = subprocess.run(list(map(str, args)), capture_output=True, text=True, timeout=60)
    assert result.returncode == 0, (args, result.returncode, result.stderr)
    return result


def text_size(path):
    return sum(int(line.split()[1]) for line in run(['size', '-A', path]).stdout.splitlines()
               if line.split() and line.split()[0].startswith('.text'))


source = OUT / 'debug.cpp'
source.write_text(''.join('extern "C" long kernel%d(long x) {\n'
                         ' for (int i=0; i<4; ++i) x=(x*17+i)%%1009;\n'
                         ' return x;\n}\n' % i for i in range(1200)))
expected = 8
for _ in range(3000000):
    for i in range(4):
        expected = (expected * 17 + i) % 1009
main = OUT / 'main.cpp'
main.write_text('extern "C" long kernel0(long);\n'
                'int main(int argc,char**argv){if(argc!=2)return 2;long x=argc+6;\n'
                'long n=(argv[1][0]-48)*1000000L;for(long j=0;j<n;++j)x=kernel0(x);\n'
                'return x==%d?0:1;}\n' % expected)
flags = {label: ['-c', '-O3', debug] for label, debug in [('A', '-g0'), ('B', '-gline-tables-only')]}
result = dict(binary=dict(path=str(BINARY), sha256=sha(BINARY)), flags=flags,
              affinity=AFFINITY, inputs={p.name: dict(sha256=sha(p), bytes=p.stat().st_size) for p in [source, main]},
              expected=expected, runtime_arguments=['3'], runs=[], images={}, summary={})


def save():
    (OUT / 'performance.json').write_text(json.dumps(result, indent=2) + '\n')


run([BINARY, '-c', '-O1', '-g0', main, '-o', OUT / 'main.o'])
for label in 'AB':
    obj, exe = OUT / (label + '.o'), OUT / label
    run([BINARY, *flags[label], source, '-o', obj])
    run(['g++', OUT / 'main.o', obj, '-o', exe])
    run([exe, '3'])
    result['images'][label] = dict(object_sha256=sha(obj), exe_sha256=sha(exe),
                                  object_text_bytes=text_size(obj), text_bytes=text_size(exe),
                                  telemetry=[json.loads(line) for line in run([BINARY, *flags[label], '--stats', source,
                                              '-o', OUT / 'stats.o']).stderr.splitlines() if line.startswith('{')])
    (OUT / (label + '.native')).write_text(run(['objdump', '-dr', obj]).stdout)
for mode in ['compile', 'runtime']:
    for block, order in enumerate(['AAAA'] + ['ABBA'] * 6):
        for label in order:
            args = [BINARY, *flags[label], source, '-o', OUT / 'measure.o'] if mode == 'compile' else [OUT / label, '3']
            start = time.perf_counter()
            sample = run(['/usr/bin/time', '-f', '%M', '-o', OUT / 'rss', *AFFINITY, *args])
            row = dict(mode=mode, block=block, label=label, wall_s=time.perf_counter()-start,
                       peak_rss_kib=int((OUT / 'rss').read_text()), status=sample.returncode)
            if mode == 'compile':
                row['object_sha256'] = sha(OUT / 'measure.o')
                assert row['object_sha256'] == result['images'][label]['object_sha256']
            result['runs'].append(row)
            save()
    rows = [row for row in result['runs'] if row['mode'] == mode]
    ratios = [statistics.mean(row['wall_s'] for row in rows if row['block'] == block and row['label'] == 'B') /
              statistics.mean(row['wall_s'] for row in rows if row['block'] == block and row['label'] == 'A') for block in range(1, 7)]
    aa = [row['wall_s'] for row in rows if not row['block']]
    summary = dict(AA_range_s=[min(aa), max(aa)], paired_ratios=ratios,
                   paired_ratio_median=statistics.median(ratios), paired_ratio_range=[min(ratios), max(ratios)])
    for label in 'AB':
        samples = [row for row in rows if row['block'] and row['label'] == label]
        summary[label] = dict(median_s=statistics.median(row['wall_s'] for row in samples),
                              range_s=[min(row['wall_s'] for row in samples), max(row['wall_s'] for row in samples)],
                              peak_rss_kib=max(row['peak_rss_kib'] for row in samples))
    result['summary'][mode] = summary
    save()
    print(mode, json.dumps(summary), flush=True)
assert sha(BINARY) == result['binary']['sha256']
