#!/usr/bin/env python3
"""Serial frozen PA4 A/A + ABBA compiler campaign; all observations in Markdown."""
import argparse
import hashlib
import json
import platform
from pathlib import Path
import statistics
import subprocess
import tempfile
import time

parser = argparse.ArgumentParser()
parser.add_argument('baseline', type=Path)
parser.add_argument('candidate', type=Path)
parser.add_argument('report', type=Path)
parser.add_argument('--candidate-commit', default='c90cf1e62')
args = parser.parse_args()
a, b = args.baseline.resolve(), args.candidate.resolve()
sha = lambda data: hashlib.sha256(data).hexdigest()
median = statistics.median

with tempfile.TemporaryDirectory(prefix='pa4-performance-') as temporary:
    root = Path(temporary)
    ordinary = ('template<class T> struct Box { T value; };\n'
                'double sum(double *p, int n) { double s=0; for(int i=0;i<n;++i) s+=p[i]; return s; }\n'
                'int caller(int x) { return x+7; }\n')
    nested = '#define I(x) x\n' + ('I('*600+'42'+')'*600+'\n')*128
    chain = '#define F0() done\n' + ''.join('#define F%d() F%d()\n' % (i,i-1) for i in range(1,8001))
    chain += '#define D(x) ' + 'x '*64 + '\n' + 'D(F8000())\n'*64
    workloads = {
        'flat-4MiB': ('name + 123;\n' * (4*1024*1024//12)),
        'flat-16MiB': ('name + 123;\n' * (16*1024*1024//12)),
        'ordinary-C++': ordinary * 16000,
        'macro-reuse': '#define D(x) x+x+x+x\n' + 'D(7) '*150000,
        'nested-arguments': nested,
        'long-chain-reuse': chain,
        'counter-spellings': '__COUNTER__ '*300000,
    }
    frozen = {'A': sha(a.read_bytes()), 'B': sha(b.read_bytes())}
    rows, manifests, summaries = [], [], []

    def measure(label, binary, path, mode='ordinary'):
        rss = root / 'rss'
        out = root / 'output'
        command = ['/usr/bin/time', '-f', '%M', '-o', str(rss), str(binary)]
        if mode == 'stats': command.append('--stats')
        command += ['-o', str(out), str(path)]
        start = time.perf_counter()
        run = subprocess.run(command, capture_output=True, check=True)
        elapsed = time.perf_counter() - start
        digest = sha(out.read_bytes())
        sample = dict(workload=label, mode=mode, binary='A' if binary == a else 'B',
                      seconds=elapsed, rss_kib=int(rss.read_text()), output_sha256=digest)
        rows.append(sample)
        return sample, json.loads(run.stderr) if mode == 'stats' else None

    for label, text in workloads.items():
        path = root / (label + '.cc')
        path.write_text(text)
        group = []
        for kind in 'AAAABB':
            sample, _ = measure(label, a if kind == 'A' else b, path)
            sample['block'] = 'calibration'
            group.append(sample)
        for block in range(2):
            for kind in 'ABBA':
                sample, _ = measure(label, a if kind == 'A' else b, path)
                sample['block'] = str(block + 1)
                group.append(sample)
        sample, stats = measure(label, b, path, 'stats')
        sample['block'] = 'telemetry'
        group.append(sample)
        assert len({r['output_sha256'] for r in group}) == 1, label
        cal = group[:6]
        noise = max(abs(cal[i+1]['seconds']/cal[i]['seconds']-1) for i in (0,2,4))
        paired = []
        for k in (6,10):
            x = group[k:k+4]
            paired.append((x[1]['seconds']+x[2]['seconds'])/(x[0]['seconds']+x[3]['seconds'])-1)
        ar = [r for r in group if r['binary']=='A' and r['mode']=='ordinary']
        br = [r for r in group if r['binary']=='B' and r['mode']=='ordinary']
        summary = dict(workload=label, noise=noise, paired=paired,
                       a_seconds=median(r['seconds'] for r in ar),
                       b_seconds=median(r['seconds'] for r in br),
                       a_rss=median(r['rss_kib'] for r in ar), b_rss=median(r['rss_kib'] for r in br))
        summaries.append(summary)
        manifests.append(dict(workload=label, bytes=path.stat().st_size, input_sha256=sha(path.read_bytes()),
                              output_sha256=group[0]['output_sha256'], stats=stats))
        print(label, json.dumps(summary), flush=True)

    texts = [int(subprocess.check_output(['size', str(binary)], text=True).splitlines()[1].split()[0])
             for binary in (a,b)]
    assert frozen == {'A':sha(a.read_bytes()), 'B':sha(b.read_bytes())}
    lines = ['# PA4 frozen compiler performance evidence', '',
        'A: first complete implementation (`28279a9d0`); B: `' + args.candidate_commit + '` (indexed argument slices, explicit prescan tasks and reusable spelling/scratch storage).', '',
        'Both use the ordinary `g++ -std=gnu++11 -Wall -O3` build with the same course runner. '
        'Fresh processes, serial measurements, output to a temporary file, wall time including process startup and peak RSS from GNU time. '
        'Each workload has two A/A pairs, one B/B pair, two ABBA blocks and a separate B telemetry sample. All observations are below. '
        'Every output agrees within its workload. Temporary source paths affect output hashes, but are identical across each A/B group.', '',
        'Platform: `' + platform.platform() + '`. Host: `' + subprocess.check_output(['g++','--version'],text=True).splitlines()[0] + '`.', '',
        'Frozen SHA256 A: `' + frozen['A'] + '`; B: `' + frozen['B'] + '`.', '',
        'Generated executable runtime / generated text size: **N/A at PA4**. Host-tool text (GNU size, including read-only data): '
        + str(texts[0]) + ' → ' + str(texts[1]) + ' bytes (' + format((texts[1]/texts[0]-1)*100,'.2f') + '%).', '',
        'Budgets fixed in pa4/plan.md: unaffected paired latency <=10% plus measured noise; RSS <=15% plus 1 MiB; '
        'host text growth <=15%; 4x flat input <6x wall time and <5x RSS. Indexed nesting captures <=3n tokens; '
        'generated spelling arena <=128 KiB. No runtime optimization or generated-code benefit is claimed.', '',
        '| Workload | A median s | B median s | ABBA deltas % | A/A or B/B noise % | A RSS KiB | B RSS KiB |',
        '| --- | ---: | ---: | --- | ---: | ---: | ---: |']
    for s in summaries:
        lines.append('| {workload} | {a_seconds:.6f} | {b_seconds:.6f} | '.format(**s)
                     + ', '.join(format(x*100,'.2f') for x in s['paired'])
                     + ' | {noise:.2%} | {a_rss:.0f} | {b_rss:.0f} |'.format(**s))
    lines += ['', '## Manifests and telemetry', '', '```json', json.dumps(manifests, indent=2), '```', '',
              '## Every observation', '',
              '| Workload | Block | Binary | Mode | Wall seconds | Peak RSS KiB |',
              '| --- | --- | --- | --- | ---: | ---: |']
    for r in rows:
        lines.append('| {workload} | {block} | {binary} | {mode} | {seconds:.6f} | {rss_kib} |'.format(**r))
    lines += ['', 'Output SHA256 is recorded once per workload above; every sample was checked against it.', '']
    args.report.write_text('\n'.join(lines))
