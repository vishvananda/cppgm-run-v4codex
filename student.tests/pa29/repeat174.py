#!/usr/bin/env python3
"""Repeat the pruning signal with the exact frozen inputs/flags and all samples."""
import hashlib, json, pathlib, statistics, subprocess, sys, time
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
prior = pathlib.Path(sys.argv[2]).resolve()
data = json.loads((prior/'performance.json').read_text())
source = prior/'pruning.cpp'
assert hashlib.sha256(source.read_bytes()).hexdigest() == data['inputs']['pruning']
result = {k: data[k] for k in ['binaries', 'flags', 'affinity']}
result.update(inputs={'pruning': data['inputs']['pruning']},
              images={'pruning': data['images']['pruning']}, runs=[], summary={})
for mode in ['compile', 'runtime']:
    for block, order in enumerate(['AAAA']+['ABBA']*6):
        for label in order:
            args = [data['binaries'][label]['path'], *data['flags'], str(source), '-o', str(out/'measure.o')] if mode == 'compile' else [str(prior/('pruning'+label))]
            start = time.perf_counter()
            p = subprocess.run(['/usr/bin/time', '-f', '%M', '-o', str(out/'rss'),
                                *data['affinity'], *args], capture_output=True, timeout=90)
            wall = time.perf_counter()-start
            assert p.returncode == 0, p.stderr.decode()
            result['runs'].append(dict(workload='pruning', mode=mode, block=block,
                label=label, wall_s=wall, peak_rss_kib=int((out/'rss').read_text()), status=p.returncode,
                phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]))
            (out/'performance.json').write_text(json.dumps(result, indent=2)+'\n')
    rows = [r for r in result['runs'] if r['mode'] == mode]
    ratios = [statistics.mean(r['wall_s'] for r in rows if r['block'] == b and r['label'] == 'B') /
              statistics.mean(r['wall_s'] for r in rows if r['block'] == b and r['label'] == 'A') for b in range(1,7)]
    aa = [r['wall_s'] for r in rows if not r['block']]
    summary = dict(paired_ratios=ratios, paired_ratio_median=statistics.median(ratios),
                   paired_ratio_range=[min(ratios),max(ratios)], AA_range_s=[min(aa),max(aa)])
    for label in 'AB':
        samples = [r for r in rows if r['label'] == label and r['block']]
        summary[label] = dict(median_s=statistics.median(r['wall_s'] for r in samples),
            range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],
            peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
    result['summary'].setdefault('pruning',{})[mode] = summary
    (out/'performance.json').write_text(json.dumps(result, indent=2)+'\n')
for record in result['binaries'].values():
    assert hashlib.sha256(pathlib.Path(record['path']).read_bytes()).hexdigest() == record['sha256']
print(json.dumps(result['summary']), flush=True)
