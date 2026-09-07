#!/usr/bin/env python3
"""Freeze PA1 inputs/binaries; calibrate A/A and measure ABBA changes.

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
import signal
import statistics
import subprocess
import threading
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
        watchdog = threading.Timer(60, p.kill)
        watchdog.start()
        try:
            for chunk in iter(lambda: p.stdout.read(1024 * 1024), b''):
                h.update(chunk)
            error = p.stderr.read()
            assert p.wait() == 0, error
        finally:
            watchdog.cancel()
    return h.hexdigest(), json.loads(error) if args else None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--out', type=Path)
    parser.add_argument('--baseline', type=Path, help='frozen A binary; B is current, both without telemetry')
    parser.add_argument('--inputs', type=Path, help='reuse every input from a frozen manifest')
    args = parser.parse_args()
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%d-%H%M%S')
    out = args.out or ROOT / ('obj/student-pa1/performance-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    out = out.resolve()
    shutil.copy2(Path(__file__), out / 'benchmark-frozen.py')
    binary = out / 'pptoken-frozen'
    shutil.copy2(ROOT / 'dev/pptoken', binary)
    baseline = out / 'pptoken-baseline' if args.baseline else binary
    if args.baseline:
        shutil.copy2(args.baseline, baseline)
        provenance = args.baseline.parent / 'manifest.json'
        if provenance.exists():
            shutil.copy2(provenance, out / 'baseline-manifest.json')
    flags = (ROOT / 'obj/dev/.compile_config').read_text()
    sources = [ROOT / 'dev/pptoken.cpp', *sorted((ROOT / 'dev/src/preprocess').glob('*.cpp')),
               *sorted((ROOT / 'dev/src/preprocess').glob('*.h'))]
    build_sources = sources + [ROOT / 'dev/src/support/testing/test_runner.cpp',
                               ROOT / 'dev/Makefile', ROOT / 'dev/frontend_source_sets.mk']
    manifest = dict(binary_sha256=digest(binary), baseline_sha256=digest(baseline), flags=flags,
                    host=platform.platform(), cpu=next(line.strip() for line in Path('/proc/cpuinfo').read_text().splitlines()
                                                     if line.startswith('model name')),
                    compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                    source_sha256={str(p.relative_to(ROOT)): digest(p) for p in build_sources},
                    harness_sha256=digest(Path(__file__)), affinity=sorted(os.sched_getaffinity(0)),
                    A='baseline, default flags' if args.baseline else 'current, default flags',
                    B='current, default flags' if args.baseline else 'current, --stats',
                    generated_runtime=None, generated_text_size=None,
                    budgets={'decoded_units': '2 * bytes + 64', 'peak_rss': '4 * bytes + 32 MiB',
                             'dense_unique_peak_rss': '2 * retained source/name/spelling capacities + 32 MiB',
                             'identifier_storage': '64 * unique identifiers + 2 * bytes + 64',
                             'rehash_probes': '8 * unique identifiers + 64',
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
    for count in (800000, 3200000):
        path = out / ('unique-' + str(count) + '.cpp')
        with path.open('w') as stream:
            for i in range(count):
                stream.write('name_' + str(i) + ' ')
            stream.write('\n')
        workloads.append(path)
    boundary = out / 'table-hit-boundary.cpp'
    with boundary.open('w') as stream:
        for i in range(262144):
            stream.write('name_' + str(i) + ' ')
        for i in range(1500000):
            stream.write('name_262143 ')
        stream.write('\n')
    workloads.append(boundary)
    raw = out / 'raw-near-8.cpp'
    raw.write_bytes(b'R"abcdefghijklmnop(' + b')abcdefghijklmnoQ' * 524288 + b')abcdefghijklmnop"\n')
    workloads.append(raw)
    long_token = out / 'long-token-8.cpp'
    long_token.write_bytes(b'a' * (8 * 1024 * 1024) + b'\\\nx\n')
    workloads.append(long_token)
    if args.inputs:
        for path in workloads:
            path.unlink()
        workloads = []
        original = json.loads((args.inputs / 'manifest.json').read_text())
        for name, info in original['workloads'].items():
            path = out / (name + '.cpp')
            shutil.copy2(args.inputs / path.name, path)
            assert digest(path) == info['sha256']
            workloads.append(path)
    manifest['workloads'] = {p.stem: {'bytes': p.stat().st_size, 'sha256': digest(p)} for p in workloads}
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    observations = []
    summaries = {}
    print('Frozen evidence: ' + str(out), flush=True)
    for source in workloads:
        ha, _ = output_hash(baseline, source, [])
        hb, _ = output_hash(binary, source, [])
        hs, counters = output_hash(binary, source, ['--stats'])
        assert ha == hb == hs, source
        size = source.stat().st_size
        assert counters['decoded_units'] <= 2 * size + 64, counters
        assert counters['identifier_storage_bytes'] <= 64 * counters['identifiers'] + 2 * size + 64
        assert counters['rehash_probes'] <= 8 * counters['identifiers'] + 64
        assert counters['source_storage_bytes'] <= 2 * size + 65536
        assert counters['spelling_storage_bytes'] <= 2 * size + 15
        retained = sum(counters[key] for key in
                       ('source_storage_bytes', 'identifier_storage_bytes', 'spelling_storage_bytes'))
        local = []
        for index, mode in enumerate('AAAAABBAABBA'):
            telemetry = ['--stats'] if mode == 'B' and not args.baseline else []
            selected_binary = baseline if mode == 'A' else binary
            rss_file = out / 'rss.tmp'
            with source.open('rb') as stream:
                start = time.perf_counter_ns()
                with subprocess.Popen(['/usr/bin/time', '-f', '%M', '-o', str(rss_file), str(selected_binary), *telemetry],
                                      stdin=stream, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                                      start_new_session=True) as p:
                    try:
                        _, error = p.communicate(timeout=60)
                    except subprocess.TimeoutExpired:
                        os.killpg(p.pid, signal.SIGKILL)
                        p.communicate()
                        raise
                    assert p.returncode == 0, error
                seconds = (time.perf_counter_ns() - start) / 1e9
            rss = int(rss_file.read_text())
            dense = source.stem in ('unique-800000', 'unique-3200000')
            rss_budget = (2 * retained if dense else 4 * size) + 32 * 1024 * 1024
            assert rss * 1024 <= 2 * retained + 32 * 1024 * 1024, (source, rss)
            assert rss * 1024 <= rss_budget, (source, rss, rss_budget)
            row = dict(workload=source.stem, index=index, block='AA' if index < 4 else 'ABBA',
                       mode=mode, seconds=seconds, peak_rss_kib=rss)
            if telemetry:
                measured = json.loads(error)
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
                       paired_change_percent=paired,
                       seconds={mode: {'median': statistics.median(values), 'min': min(values), 'max': max(values)}
                                for mode, values in timings.items()},
                       peak_rss_kib={mode: max(r['peak_rss_kib'] for r in local if r['mode'] == mode)
                                     for mode in ('A', 'B')})
        summaries[source.stem] = summary
        (out / 'summary.json').write_text(json.dumps(summaries, indent=2) + '\n')
        print(source.stem + ': ' + json.dumps(summary), flush=True)
    assert summaries['repeated-8']['counters']['identifier_storage_bytes'] == summaries['repeated-32']['counters']['identifier_storage_bytes']
    for mode in ('A', 'B'):
        for small, large in (('repeated-8', 'repeated-32'), ('unique-200000', 'unique-800000'),
                             ('unique-800000', 'unique-3200000')):
            growth = summaries[large]['seconds'][mode]['median'] / summaries[small]['seconds'][mode]['median']
            assert growth <= 6, (mode, small, large, growth)
            print(f'{mode} {small} -> {large} latency ratio: {growth:.6f}', flush=True)
    print('All compiler work/memory/scaling envelopes passed.', flush=True)
    print('Generated runtime/text size: N/A (PA1 token output only).', flush=True)


if __name__ == '__main__':
    main()
