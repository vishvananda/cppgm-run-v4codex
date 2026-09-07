#!/usr/bin/env python3
"""Freeze PA1 inputs/binary; calibrate A/A and measure ABBA telemetry overhead.

PA1 produces tokens, so executable runtime/text size are not applicable.
No speedup over the incorrect starter or generated-code benefit is claimed.
All observations and frozen artifacts go under obj/student-pa1/performance-*.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def repeat(path, pattern, size):
    repetitions = max(1, size // len(pattern))
    with path.open('wb') as stream:
        for _ in range(repetitions):
            stream.write(pattern)


def output_hash(binary, source, args):
    h = hashlib.sha256()
    with source.open('rb') as stream:
        p = subprocess.Popen([str(binary), *args], stdin=stream,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        for chunk in iter(lambda: p.stdout.read(1024 * 1024), b''):
            h.update(chunk)
        error = p.stderr.read()
        assert p.wait() == 0, error
    return h.hexdigest(), json.loads(error) if args else None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=Path)
    args = parser.parse_args()
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%d-%H%M%S')
    out = args.out or ROOT / ('obj/student-pa1/performance-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    out = out.resolve()
    binary = out / 'pptoken-frozen'
    shutil.copy2(ROOT / 'dev/pptoken', binary)
    flags = (ROOT / 'obj/dev/.compile_config').read_text()
    sources = [ROOT / 'dev/pptoken.cpp', *sorted((ROOT / 'dev/src/preprocess').glob('*.cpp')),
               *sorted((ROOT / 'dev/src/preprocess').glob('*.h'))]
    manifest = dict(binary_sha256=digest(binary), flags=flags,
                    host=platform.platform(), cpu=next(line.strip() for line in Path('/proc/cpuinfo').read_text().splitlines()
                                                     if line.startswith('model name')),
                    compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                    source_sha256={str(p.relative_to(ROOT)): digest(p) for p in sources},
                    A='frozen binary, default flags', B='same frozen binary, --stats',
                    generated_runtime=None, generated_text_size=None,
                    budgets={'decoded_units': '2 * bytes + 64', 'peak_rss': '4 * bytes + 32 MiB',
                             'fourfold_input_latency_ratio': 6})
    pattern = (b'template<class T> struct Box { T value; T get() { return value; } };\n'
               b'for (int i=0; i<1024; ++i) { sum += values[i] * 1.25e-3; call(sum); }\n')
    mixed = b'alpha al\\\npha \\u03C0 \xcf\x80 1.e+2 /*comment\n*/ R"tag(??/\n\\UFFFFFFFF)tag"_s\n'
    workloads = []
    for name, data, mib in [('repeated-8', pattern, 8), ('repeated-32', pattern, 32),
                            ('translations-8', mixed, 8),
                            ('self-source-8', b'\n'.join(p.read_bytes() for p in sources), 8)]:
        path = out / (name + '.cpp')
        repeat(path, data, mib * 1024 * 1024)
        workloads.append(path)
    unique = out / 'unique-200000.cpp'
    with unique.open('w') as stream:
        for i in range(200000):
            stream.write('name_' + str(i) + ' ')
        stream.write('\n')
    workloads.append(unique)
    manifest['workloads'] = {p.stem: {'bytes': p.stat().st_size, 'sha256': digest(p)} for p in workloads}
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    observations = []
    summaries = {}
    print('Frozen evidence: ' + str(out), flush=True)
    for source in workloads:
        ha, _ = output_hash(binary, source, [])
        hb, counters = output_hash(binary, source, ['--stats'])
        assert ha == hb, source
        size = source.stat().st_size
        assert counters['decoded_units'] <= 2 * size + 64, counters
        local = []
        for index, mode in enumerate('AAAAABBAABBA'):
            telemetry = ['--stats'] if mode == 'B' else []
            rss_file = out / 'rss.tmp'
            with source.open('rb') as stream:
                start = time.perf_counter_ns()
                p = subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(rss_file), str(binary), *telemetry],
                                   stdin=stream, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, check=True)
                seconds = (time.perf_counter_ns() - start) / 1e9
            rss = int(rss_file.read_text())
            assert rss * 1024 <= 4 * size + 32 * 1024 * 1024, (source, rss)
            row = dict(workload=source.stem, index=index, block='AA' if index < 4 else 'ABBA',
                       mode=mode, seconds=seconds, peak_rss_kib=rss)
            if telemetry:
                measured = json.loads(p.stderr)
                assert measured['tokens'] == counters['tokens']
                row['telemetry'] = measured
            local.append(row)
            observations.append(row)
            (out / 'observations.json').write_text(json.dumps(observations, indent=2) + '\n')
        calibration = [(local[i + 1]['seconds'] / local[i]['seconds'] - 1) * 100 for i in (0, 2)]
        paired = []
        for begin in (4, 8):
            a = statistics.mean(local[i]['seconds'] for i in (begin, begin + 3))
            b = statistics.mean(local[i]['seconds'] for i in (begin + 1, begin + 2))
            paired.append((b / a - 1) * 100)
        timings = {mode: [r['seconds'] for r in local if r['mode'] == mode and r['block'] == 'ABBA']
                   for mode in ('A', 'B')}
        summary = dict(output_sha256=ha, counters=counters, aa_percent=calibration,
                       telemetry_overhead_percent=paired,
                       seconds={mode: {'median': statistics.median(values), 'min': min(values), 'max': max(values)}
                                for mode, values in timings.items()},
                       peak_rss_kib=max(r['peak_rss_kib'] for r in local))
        summaries[source.stem] = summary
        (out / 'summary.json').write_text(json.dumps(summaries, indent=2) + '\n')
        print(source.stem + ': ' + json.dumps(summary), flush=True)
    ratio = summaries['repeated-32']['seconds']['A']['median'] / summaries['repeated-8']['seconds']['A']['median']
    assert ratio <= 6, ratio
    assert summaries['repeated-8']['counters']['identifier_storage_bytes'] == summaries['repeated-32']['counters']['identifier_storage_bytes']
    print('All compiler work/memory/scaling envelopes passed; 4x latency ratio: ' + str(ratio), flush=True)
    print('Generated runtime/text size: N/A (PA1 token output only).', flush=True)


if __name__ == '__main__':
    main()
