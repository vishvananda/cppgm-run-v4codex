#!/usr/bin/env python3
"""Freeze PA3 inputs/binaries; calibrate A/A and measure ABBA changes.

PA3 produces expression values, so executable runtime/text size are not applicable.
No speedup over the incorrect starter or generated-code benefit is claimed.
All observations and frozen artifacts go under obj/student-pa3/performance-*.
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
    out = args.out or ROOT / ('obj/student-pa3/performance-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    out = out.resolve()
    shutil.copy2(Path(__file__), out / 'benchmark-frozen.py')
    binary = out / 'ppexpr-frozen'
    shutil.copy2(ROOT / 'dev/ppexpr', binary)
    baseline = out / 'ppexpr-baseline' if args.baseline else binary
    if args.baseline:
        shutil.copy2(args.baseline, baseline)
        provenance = args.baseline.parent / 'manifest.json'
        if provenance.exists():
            shutil.copy2(provenance, out / 'baseline-manifest.json')
    flags = (ROOT / 'obj/dev/.compile_config').read_text()
    sources = [ROOT / 'dev/ppexpr.cpp', *sorted((ROOT / 'dev/src/preprocess').glob('*.cpp')),
               *sorted((ROOT / 'dev/src/preprocess').glob('*.h'))]
    sources += sorted((ROOT / 'dev/src/posttoken').glob('*.cpp')) + sorted((ROOT / 'dev/src/posttoken').glob('*.h'))
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
                    budgets={'decoded_units': '2 * bytes + 64', 'peak_rss': '40 * bytes + 32 MiB',
                             'expression_storage': '32 * max_values + 6 * max_operators + 64',
                             'number_and_literal_bytes': 'bytes',
                             'decoded_elements': 'bytes',
                             'reductions': 'expression tokens',
                             'expression_storage_growths': '128',
                             'dense_unique_peak_rss': '2 * retained source/name/spelling capacities + 32 MiB',
                             'identifier_storage': '64 * unique identifiers + 2 * bytes + 64',
                             'rehash_probes': '8 * unique identifiers + 64',
                             'fourfold_input_latency_ratio': 6})
    pattern = (b"defined alpha ? -7 : u'a'\n(1u<<63)/-1\n0 && (7/0) || 1\n"
               b"0 ? 0 : 1 ? 5 : 6u\n'\\u03d0' + L'a'\n")
    workloads, expected = [], {}

    def add(name, data, result):
        path = out / (name + '.cpp')
        path.write_bytes(data)
        workloads.append(path)
        expected[name] = hashlib.sha256(result + b'eof\n').hexdigest()

    for mib in (4, 16):
        count = mib * 1024 * 1024 // len(pattern)
        add('repeated-' + str(mib), pattern * count,
            b'18446744073709551609u\n0u\n1\n5u\n1073\n' * count)
    count = 2 * 1024 * 1024
    add('chain-4', b'1' + b'+1' * count + b'\n', str(count+1).encode() + b'\n')
    add('nested-4', b'(' * count + b'7' + b')' * count + b'\n', b'7\n')
    add('conditional-4', b'0?1u:' * (4 * 1024 * 1024 // 5) + b'-7\n', b'18446744073709551609u\n')
    add('unique-names-200000', b''.join(('defined name_' + str(i) + ' ? 1 : 7u\n').encode()
                                     for i in range(200000)), b'7u\n' * 200000)
    invalid = b'0 ? 1.0 : 2\n7\ndefined(a+b)\n9\n'
    count = 4 * 1024 * 1024 // len(invalid)
    add('recovery-4', invalid * count, b'error\n7\nerror\n9\n' * count)
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
    manifest['host_size'] = subprocess.check_output(['size', str(binary)], text=True)
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    observations = []
    summaries = {}
    print('Frozen evidence: ' + str(out), flush=True)
    for source in workloads:
        ha, _ = output_hash(baseline, source, [])
        hb, _ = output_hash(binary, source, [])
        hs, counters = output_hash(binary, source, ['--stats'])
        assert ha == hb == hs == expected[source.stem], source
        size = source.stat().st_size
        assert counters['decoded_units'] <= 2 * size + 64, counters
        assert counters['identifier_storage_bytes'] <= 64 * counters['identifiers'] + 2 * size + 64
        assert counters['rehash_probes'] <= 8 * counters['identifiers'] + 64
        assert counters['source_storage_bytes'] <= 2 * size + 65536
        assert counters['spelling_storage_bytes'] <= 2 * size + 15
        assert counters['expression_storage_bytes'] <= 32 * counters['max_values'] + 6 * counters['max_operators'] + 64
        assert counters['number_bytes'] + counters['literal_bytes'] <= size
        assert counters['decoded_elements'] <= size
        assert counters['expression_tokens'] <= size
        assert counters['reductions'] <= counters['expression_tokens']
        assert counters['expression_storage_growths'] <= 128
        if source.stem == 'chain-4':
            assert counters['max_values'] == 2 and counters['max_operators'] == 1
        retained = sum(counters[key] for key in
                       ('source_storage_bytes', 'identifier_storage_bytes', 'spelling_storage_bytes', 'expression_storage_bytes'))
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
            rss_budget = 40 * size + 32 * 1024 * 1024
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
    assert summaries['repeated-4']['counters']['identifier_storage_bytes'] == summaries['repeated-16']['counters']['identifier_storage_bytes']
    assert summaries['repeated-4']['counters']['expression_storage_bytes'] == summaries['repeated-16']['counters']['expression_storage_bytes']
    for mode in ('A', 'B'):
        for small, large in (('repeated-4', 'repeated-16'),):
            growth = summaries[large]['seconds'][mode]['median'] / summaries[small]['seconds'][mode]['median']
            assert growth <= 6, (mode, small, large, growth)
            print(f'{mode} {small} -> {large} latency ratio: {growth:.6f}', flush=True)
    print('All compiler work/memory/scaling envelopes passed.', flush=True)
    print('Generated runtime/text size: N/A (PA3 expression output only).', flush=True)


if __name__ == '__main__':
    main()
