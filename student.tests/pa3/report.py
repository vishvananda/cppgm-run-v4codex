#!/usr/bin/env python3
"""Render retained PA3 benchmark observations and verify frozen provenance."""
import argparse
import hashlib
import json
from pathlib import Path
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT / 'student.tests/pa3/performance.md')
    parser.add_argument('--baseline-revision', help='git revision proving frozen A build-source hashes')
    parser.add_argument('campaigns', type=Path, nargs='+')
    args = parser.parse_args()
    campaigns = [path.resolve() for path in args.campaigns]
    assert campaigns, 'pass completed benchmark directories'
    lines = ['# PA3 performance evidence', '',
             'PA3 emits expression results; generated executable runtime and text size are **N/A**.',
             'No speedup over the nonfunctional entry stub or generated-code benefit is claimed.',
             'These measurements establish frontend resource bounds and compare the declared A/B modes.', '',
             'Protocol: frozen binaries, flags, source/harness hashes and fixed input hashes;',
             'independently checked complete outputs; two A/A pairs followed by two ABBA',
             'blocks per workload. A/B modes, host and affinity are recorded below. Wall time',
             'and peak RSS are measured separately from untimed hashing/warmup. All runs',
             'have a 60-second timeout. Campaigns run serially after correctness checks,',
             'with no compiler builds or test processes running during measurement.',
             'No samples are discarded. Small positive and negative changes are disclosed;',
             'interpret changes against A/A noise and the predeclared budgets.', '',
             'Budgets and workload meanings are in [README.md](README.md). Every campaign',
             'passes the compiler work, memory and scaling envelopes. Flat chains retain',
             'two values/one operator (35 bytes allocated capacity); repeated 4/16 MiB',
             'inputs retain identical 140-byte expression scratch and 126-byte name storage.',
             'Deep conditional/parenthesis storage tracks nesting, without host recursion.',
             'RSS below is `/usr/bin/time` whole-child peak; telemetry samples RSS before',
             'reporting and teardown, and can be lower than the whole-child peak.',
             'Untimed output-hash launch RSS is retained in raw summaries but is excluded',
             'from the measurement table and resource conclusions.', '']
    total = 0
    frozen_sources = None
    confirmed = {'floating-rejection-4': 0, 'suffix-rejection-200000': 0}
    comparison_seen = False
    for number, path in enumerate(campaigns, 1):
        manifest = json.loads((path / 'manifest.json').read_text())
        summary = json.loads((path / 'summary.json').read_text())
        rows = json.loads((path / 'observations.json').read_text())
        assert set(summary) == set(manifest['workloads']) and len(rows) == 12 * len(summary)
        assert digest(path / 'ppexpr-frozen') == manifest['binary_sha256'] == digest(ROOT / 'dev/ppexpr')
        assert digest(path / 'benchmark-frozen.py') == manifest['harness_sha256']
        for name, sha in manifest['source_sha256'].items():
            assert digest(ROOT / name) == sha, name
        if frozen_sources is None:
            frozen_sources = manifest['source_sha256']
        assert frozen_sources == manifest['source_sha256']
        comparison = manifest['baseline_sha256'] != manifest['binary_sha256']
        comparison_seen |= comparison
        if comparison:
            assert args.baseline_revision, 'supply the reviewed baseline revision'
            assert digest(path / 'ppexpr-baseline') == manifest['baseline_sha256']
            baseline = json.loads((path / 'baseline-manifest.json').read_text())
            assert baseline['binary_sha256'] == manifest['baseline_sha256']
            assert baseline['flags'] == manifest['flags']
            for name, sha in baseline['source_sha256'].items():
                content = subprocess.check_output(['git', 'show', args.baseline_revision + ':' + name], cwd=ROOT)
                assert hashlib.sha256(content).hexdigest() == sha, name
            old_text = int(manifest['baseline_host_size'].splitlines()[1].split()[0])
            new_text = int(manifest['host_size'].splitlines()[1].split()[0])
            assert new_text <= old_text * 1.01
        for name, info in manifest['workloads'].items():
            assert digest(path / (name + '.cpp')) == info['sha256']
        if number == len(campaigns):
            assert digest(ROOT / 'student.tests/pa3/benchmark.py') == manifest['harness_sha256']
        for mode in ('A', 'B'):
            assert summary['repeated-16']['seconds'][mode]['median'] / summary['repeated-4']['seconds'][mode]['median'] <= 6
        total += len(rows)
        lines += [f'## Campaign {number}: {path.name}', '',
                  f'Frozen artifacts: `{path.relative_to(ROOT)}`. All {len(rows)} samples retained.',
                  f'Compiler: `{manifest["compiler"]}`; CPU: `{manifest["cpu"]}`.',
                  f'Host: `{manifest["host"]}`; affinity: `{manifest["affinity"]}`.',
                  f'Binary SHA-256: `{manifest["binary_sha256"]}`.',
                  f'A: {manifest["A"]}; B: {manifest["B"]}.',
                  f'Baseline SHA-256: `{manifest["baseline_sha256"]}`; source revision: `{args.baseline_revision or "current"}`.',
                  f'Harness SHA-256: `{manifest["harness_sha256"]}`.', '',
                  'Build flags and host-tool size (not generated text):', '', '```text',
                  '\n'.join(line.rstrip() for line in manifest['flags'].splitlines()),
                  manifest.get('baseline_host_size', '').rstrip(), manifest['host_size'].rstrip(), '```', '',
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
            calibration = [(local[i+1]['seconds'] / local[i]['seconds'] - 1)*100 for i in (0, 2)]
            assert calibration == info['aa_percent']
            for mode in ('A', 'B'):
                assert max(r['peak_rss_kib'] for r in local if r['mode'] == mode) == info['peak_rss_kib'][mode]
            if comparison:
                noise = max(map(abs, calibration))
                if name in ('floating-rejection-4', 'suffix-rejection-200000'):
                    confirmed[name] += max(paired) < -noise
                    assert info['counters']['identifiers'] == 0
                else:
                    assert statistics.mean(paired) <= 5 + noise, (name, paired, noise)
                assert info['peak_rss_kib']['B'] <= info['peak_rss_kib']['A'] + 1024
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
        if comparison:
            lines += ['', '| Rejection workload | Identifiers A/B | Name storage A/B bytes |',
                      '| --- | --- | --- |']
            for name in ('floating-rejection-4', 'suffix-rejection-200000'):
                a, b = summary[name]['baseline_counters'], summary[name]['counters']
                lines.append(f'| {name} | {a["identifiers"]}/{b["identifiers"]} | {a["identifier_storage_bytes"]}/{b["identifier_storage_bytes"]} |')
            for name in confirmed:
                info = summary[name]
                proven = max(info['paired_change_percent']) < -max(map(abs, info['aa_percent']))
                lines.append(f'{name}: ' + ('both ABBA blocks improve beyond this campaign\'s A/A noise.'
                                           if proven else 'A/A noise exceeds the measured gain; this campaign alone is inconclusive.'))
            lines += ['', 'Other workloads pass the ≤5% plus calibrated noise budget on the campaign mean',
                      'of paired changes. Individual block excursions remain visible in the table.',
                      'Host-tool text stays within 1% growth; peak RSS B stays within A + 1 MiB.', '']
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
            lines += ['', 'Frozen build-source hashes (identical across campaigns):', '',
                      '| Source | SHA-256 |', '| --- | --- |']
            for name, sha in manifest['source_sha256'].items():
                lines.append(f'| {name} | `{sha}` |')
    if comparison_seen:
        assert min(confirmed.values()) >= 2, ('need two calibrated confirmations per affected workload', confirmed)
        lines += ['', f'Calibrated confirmations across campaigns: {confirmed}.',
                  'Each affected workload has at least two campaigns with both ABBA gains beyond A/A noise.',
                  'Inconclusive campaigns remain in the evidence and are not used alone to claim a gain.', '']
    lines += ['', f'Total: {total} retained timed observations. Compiler outputs and final source/binary',
              'provenance agree. Generated-program runtime/text size are N/A; frontend changes',
              'do not establish generated-code optimization profitability.', '']
    args.output.write_text('\n'.join(lines))
    print(f'Recomputed and retained {total} observations; all frozen source, binary, harness and input hashes match.')


if __name__ == '__main__':
    main()
