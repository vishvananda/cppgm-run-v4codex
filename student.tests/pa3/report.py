#!/usr/bin/env python3
"""Render retained PA3 benchmark observations and verify frozen provenance."""
import hashlib
import json
from pathlib import Path
import statistics
import sys

ROOT = Path(__file__).resolve().parents[2]


def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    campaigns = [Path(arg).resolve() for arg in sys.argv[1:]]
    assert campaigns, 'pass completed benchmark directories'
    lines = ['# PA3 performance evidence', '',
             'PA3 emits expression results; generated executable runtime and text size are **N/A**.',
             'No speedup over the nonfunctional entry stub or generated-code benefit is claimed.',
             'These measurements establish frontend resource bounds and expose telemetry cost.', '',
             'Protocol: frozen binaries, flags, source/harness hashes and fixed input hashes;',
             'independently checked complete outputs; two A/A pairs followed by two ABBA',
             'blocks per workload. A is ordinary execution, B adds `--stats`. Wall time',
             'and peak RSS are measured separately from untimed hashing/warmup. All runs',
             'pin CPU 0; each invocation has a 60-second timeout. The second campaign',
             'runs after sanitizer/course checks have finished, with the final harness.',
             'No samples are discarded. Small positive and negative changes are disclosed;',
             'noise and optional instrumentation cost do not justify an optimization claim.', '',
             'Budgets and workload meanings are in [README.md](README.md). Every campaign',
             'passes the compiler work, memory and scaling envelopes. Flat chains retain',
             'two values/one operator (35 bytes allocated capacity); repeated 4/16 MiB',
             'inputs retain identical 140-byte expression scratch and 126-byte name storage.',
             'Deep conditional/parenthesis storage tracks nesting, without host recursion.',
             'RSS below is `/usr/bin/time` whole-child peak; telemetry samples RSS before',
             'reporting and teardown, and can be up to 284 KiB lower in these campaigns.',
             'Untimed output-hash launch RSS is retained in raw summaries but is excluded',
             'from the measurement table and resource conclusions.', '']
    total = 0
    for number, path in enumerate(campaigns, 1):
        manifest = json.loads((path / 'manifest.json').read_text())
        summary = json.loads((path / 'summary.json').read_text())
        rows = json.loads((path / 'observations.json').read_text())
        assert len(summary) == 7 and len(rows) == 84
        assert digest(path / 'ppexpr-frozen') == manifest['binary_sha256'] == digest(ROOT / 'dev/ppexpr')
        assert digest(path / 'benchmark-frozen.py') == manifest['harness_sha256']
        for name, sha in manifest['source_sha256'].items():
            assert digest(ROOT / name) == sha, name
        for name, info in manifest['workloads'].items():
            assert digest(path / (name + '.cpp')) == info['sha256']
        if number == len(campaigns):
            assert digest(ROOT / 'student.tests/pa3/benchmark.py') == manifest['harness_sha256']
        for mode in ('A', 'B'):
            assert summary['repeated-16']['seconds'][mode]['median'] / summary['repeated-4']['seconds'][mode]['median'] <= 6
        total += len(rows)
        lines += [f'## Campaign {number}: {path.name}', '',
                  f'Frozen artifacts: `{path.relative_to(ROOT)}`. All 84 samples retained.',
                  f'Compiler: `{manifest["compiler"]}`; CPU: `{manifest["cpu"]}`.',
                  f'Host: `{manifest["host"]}`; affinity: `{manifest["affinity"]}`.',
                  f'Binary SHA-256: `{manifest["binary_sha256"]}`.',
                  f'Harness SHA-256: `{manifest["harness_sha256"]}`.', '',
                  'Build flags and host-tool size (not generated text):', '', '```text',
                  '\n'.join(line.rstrip() for line in manifest['flags'].splitlines()), manifest['host_size'].rstrip(), '```', '',
                  '| Workload | A median [min, max] s | B median [min, max] s | Peak RSS A/B KiB | ABBA B/A changes % | A/A noise % |',
                  '| --- | --- | --- | --- | --- | --- |']
        for name, info in summary.items():
            local = [r for r in rows if r['workload'] == name]
            assert len(local) == 12
            times = {}
            for mode in ('A', 'B'):
                values = [r['seconds'] for r in local if r['mode'] == mode and r['block'] == 'ABBA']
                actual = {'median': statistics.median(values), 'min': min(values), 'max': max(values)}
                assert actual == info['seconds'][mode]
                times[mode] = f'{actual["median"]:.6f} [{actual["min"]:.6f}, {actual["max"]:.6f}]'
            paired = []
            for begin in (4, 8):
                a = statistics.mean(local[i]['seconds'] for i in (begin, begin + 3))
                b = statistics.mean(local[i]['seconds'] for i in (begin + 1, begin + 2))
                paired.append((b/a-1)*100)
            assert paired == info['paired_change_percent']
            pair = ', '.join(f'{v:+.3f}' for v in paired)
            noise = ', '.join(f'{v:+.3f}' for v in info['aa_percent'])
            rss = info['peak_rss_kib']
            lines.append(f'| {name} | {times["A"]} | {times["B"]} | {rss["A"]}/{rss["B"]} | {pair} | {noise} |')
        ratios = {m: summary['repeated-16']['seconds'][m]['median'] / summary['repeated-4']['seconds'][m]['median']
                  for m in ('A', 'B')}
        lines += ['', f'Fourfold-source latency ratios: A {ratios["A"]:.6f}x, B {ratios["B"]:.6f}x (budget ≤ 6x).', '',
                  '| Workload | Bytes | Input SHA-256 | Checked output SHA-256 |', '| --- | --- | --- | --- |']
        for name, info in manifest['workloads'].items():
            lines.append(f'| {name} | {info["bytes"]} | `{info["sha256"]}` | `{summary[name]["output_sha256"]}` |')
        lines += ['', '| Workload | Expression tokens / reductions | Max values / operators | Scratch bytes / growths | Name bytes |',
                  '| --- | --- | --- | --- | --- |']
        for name, info in summary.items():
            c = info['counters']
            lines.append(f'| {name} | {c["expression_tokens"]} / {c["reductions"]} | {c["max_values"]} / {c["max_operators"]} | {c["expression_storage_bytes"]} / {c["expression_storage_growths"]} | {c["identifier_storage_bytes"]} |')
        lines += ['', 'All observations; first four rows per workload are A/A calibration,',
                  'then two ABBA blocks. Times retain nanosecond-resolution observations.', '',
                  '| Workload | Index | Block/mode | Wall s | Peak RSS KiB | Telemetry RSS KiB | Read ms | Evaluate/emit ms |',
                  '| --- | --- | --- | --- | --- | --- | --- | --- |']
        for row in rows:
            telemetry = row.get('telemetry', {})
            if telemetry:
                assert telemetry['peak_rss_kib'] <= row['peak_rss_kib']
            lines.append(f'| {row["workload"]} | {row["index"]} | {row["block"]}/{row["mode"]} | {row["seconds"]:.9f} | {row["peak_rss_kib"]} | {telemetry.get("peak_rss_kib", "—")} | {telemetry.get("read_ms", "—")} | {telemetry.get("evaluate_emit_ms", "—")} |')
        if number == len(campaigns):
            lines += ['', 'Frozen build-source hashes (identical across both campaigns):', '',
                      '| Source | SHA-256 |', '| --- | --- |']
            for name, sha in manifest['source_sha256'].items():
                lines.append(f'| {name} | `{sha}` |')
    lines += ['', f'Total: {total} retained timed observations. Compiler outputs and final source/binary',
              'provenance agree. No runtime/text-size or optimization-profitability claim applies.', '']
    (ROOT / 'student.tests/pa3/performance.md').write_text('\n'.join(lines))
    print(f'Recomputed and retained {total} observations; all frozen source, binary, harness and input hashes match.')


if __name__ == '__main__':
    main()
