#!/usr/bin/env python3
"""Final audit: fixed binaries/inputs, ordinary compilation, isolated telemetry.

measure A B output.json; verify output.json A B; report output.json
"""
import hashlib
import json
import os
import pathlib
import platform
import statistics as st
import subprocess
import sys
import tempfile
import time
from bench_inputs import workloads

ORDERS = [('AA1', 'AA'), ('AA2', 'AA'), ('BB', 'BB'),
          ('ABBA1', 'ABBA'), ('ABBA2', 'ABBA')]
BUDGETS = dict(wall_percent=10, rss_percent=15, rss_allowance_kib=1024,
               host_text_percent=15, scaling_wall=6, scaling_rss=5)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def text_size(path):
    return sum(int(line.split()[1]) for line in
               subprocess.check_output(['size', '-A', path], text=True).splitlines()
               if line.startswith('.text '))


def fixed_inputs():
    return {f'{name}-{scale}.cpp': text for scale in (1, 4)
            for name, text in workloads(scale).items()}


def measure(a, b, output):
    binaries = dict(A=a.resolve(), B=b.resolve())
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    data = dict(protocol=ORDERS, budgets=BUDGETS, affinity_cpu=cpu,
                host_build='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
                host_cxx=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                platform=platform.platform(),
                command=['/usr/bin/time', '-f', '%M', '-o', '<rss>', '<binary>',
                         '--emit-ast', '-o', '<same-output>', '<source> repeated'],
                binaries={k: dict(sha256=sha(v), text_bytes=text_size(v)) for k, v in binaries.items()},
                inputs={}, observations=[], startup=[], telemetry=[], work=[],
                generated_runtime=None, generated_text=None)
    with tempfile.TemporaryDirectory(prefix='pa5-audit-timing-') as tmp:
        root = pathlib.Path(tmp)
        art, rss = root/'tree.ast', root/'rss'
        empty = root/'empty.cpp'
        empty.write_text('')

        def observe(label, source, repeats, stats=False):
            cmd = ['/usr/bin/time', '-f', '%M', '-o', rss, binaries[label], '--emit-ast']
            if stats:
                cmd.append('--stats')
            cmd += ['-o', art, *([source]*repeats)]
            begin = time.perf_counter_ns()
            run = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
            wall = (time.perf_counter_ns()-begin)/1e9
            assert run.returncode == 0, (label, source, run.stderr)
            facts = [json.loads(line) for line in run.stderr.splitlines()] if stats else []
            assert stats or not run.stderr, run.stderr
            return dict(variant=label, stats_enabled=stats, wall_s=wall,
                        rss_kib=int(rss.read_text()), output_sha256=sha(art),
                        output_bytes=art.stat().st_size, phases=facts)

        for label in 'ABABABAB':
            data['startup'].append(observe(label, empty, 4))
        for name, text in sorted(fixed_inputs().items()):
            source = root/name
            source.write_text(text)
            repeats = 8 if name.startswith('nested') else 4
            data['inputs'][name] = dict(sha256=sha(source), bytes=source.stat().st_size, repeats=repeats)
            expected = None
            for block, order in ORDERS:
                for ordinal, label in enumerate(order):
                    row = observe(label, source, repeats)
                    expected = expected or row['output_sha256']
                    assert row['output_sha256'] == expected, (name, 'output differs')
                    row.update(input=name, block=block, ordinal=ordinal)
                    data['observations'].append(row)
                output.write_text(json.dumps(data, indent=2)+'\n')
                print(name, block, 'complete', flush=True)
            for label in 'AB':
                row = observe(label, source, repeats, True)
                assert row['output_sha256'] == expected, (name, 'telemetry changes output')
                row.update(input=name)
                data['work'].append(row)
            # Same final binary: A=ordinary, B=telemetry. All other inputs/flags fixed.
            if name in ('declarations-4.cpp', 'templates-4.cpp'):
                for block, order in ORDERS:
                    for ordinal, mode in enumerate(order):
                        row = observe('B', source, repeats, mode == 'B')
                        assert row['output_sha256'] == expected
                        row.update(input=name, block=block, ordinal=ordinal, mode=mode)
                        data['telemetry'].append(row)
            output.write_text(json.dumps(data, indent=2)+'\n')
    assert all(sha(path) == data['binaries'][key]['sha256'] for key, path in binaries.items())
    print('Frozen campaign complete:', output, flush=True)


def summarize(rows, label_key='variant'):
    blocks = {name: [r for r in rows if r['block'] == name] for name, _ in ORDERS}
    for block, order in ORDERS:
        assert ''.join(r[label_key] for r in blocks[block]) == order
        assert [r['ordinal'] for r in blocks[block]] == list(range(len(order)))
    noise = max(abs(v[0]['wall_s']-v[1]['wall_s'])/st.mean(r['wall_s'] for r in v)
                for key, v in blocks.items() if key.startswith('AA'))
    paired = []
    for block in ('ABBA1', 'ABBA2'):
        a, b = ([r for r in blocks[block] if r[label_key] == label] for label in 'AB')
        paired.append(dict(gain_percent=100*(1-st.mean(r['wall_s'] for r in b)/st.mean(r['wall_s'] for r in a)),
                           A_rss=st.mean(r['rss_kib'] for r in a), B_rss=st.mean(r['rss_kib'] for r in b)))
    result = dict(noise_percent=noise*100, paired=paired)
    for label in 'AB':
        values = [r for r in rows if r[label_key] == label and r['block'].startswith('ABBA')]
        result[label] = {key: st.median(r[key] for r in values) for key in ('wall_s', 'rss_kib')}
        result[label]['spread_s'] = [min(r['wall_s'] for r in values), max(r['wall_s'] for r in values)]
    return result


def verify(data, a, b):
    assert data['budgets'] == BUDGETS
    assert data['protocol'] == [list(v) for v in ORDERS]
    for label, path in zip('AB', (a, b)):
        assert sha(path) == data['binaries'][label]['sha256']
        assert text_size(path) == data['binaries'][label]['text_bytes']
    assert data['binaries']['B']['text_bytes'] <= data['binaries']['A']['text_bytes']*1.15
    assert set(data['inputs']) == set(fixed_inputs())
    assert len(data['observations']) == 168 and len(data['startup']) == 8
    assert len(data['work']) == 24 and len(data['telemetry']) == 28
    summary = {}
    for name, text in fixed_inputs().items():
        meta = data['inputs'][name]
        assert meta['sha256'] == hashlib.sha256(text.encode()).hexdigest()
        assert meta['bytes'] == len(text.encode())
        assert meta['repeats'] == (8 if name.startswith('nested') else 4)
        rows = [r for r in data['observations'] if r['input'] == name]
        work = [r for r in data['work'] if r['input'] == name]
        assert len({r['output_sha256'] for r in rows+work}) == 1
        assert all(not r['stats_enabled'] and not r['phases'] for r in rows)
        item = summary[name] = summarize(rows)
        for pair in item['paired']:
            assert pair['gain_percent'] >= -10-item['noise_percent'], (name, 'wall budget', pair)
            assert pair['B_rss'] <= pair['A_rss']*1.15+1024, (name, 'RSS budget')
        for run in work:
            assert len(run['phases']) == meta['repeats']
            facts = run['phases'][0]
            # Repeated primary files must have fresh, equivalent TU work/state.
            keys = ['tokens', 'nodes', 'delimiter_work', 'angle_work', 'hint_bytes']
            if run['variant'] == 'B':
                keys += ['scopes', 'lookup_scopes', 'name_probes']
            assert all(all(p[k] == facts[k] for k in keys) for p in run['phases'])
            assert facts['delimiter_work'] == facts['tokens']
            assert facts['node_growths'] <= facts['node_capacity'].bit_length()
            if name.startswith('nested'):
                assert facts['angle_work'] < facts['tokens']*2
            if run['variant'] == 'B':
                item['work'] = {k: facts[k] for k in keys}
        startup = st.median(r['wall_s'] for r in data['startup'] if r['variant'] == 'B')
        assert item['B']['wall_s'] >= startup*25, (name, 'startup fraction')
    for family in workloads():
        small, large = (summary[f'{family}-{scale}.cpp'] for scale in (1, 4))
        assert large['B']['wall_s'] < small['B']['wall_s']*6
        assert large['B']['rss_kib'] < small['B']['rss_kib']*5+1024
        for key in ('tokens', 'nodes', 'scopes', 'lookup_scopes', 'name_probes', 'angle_work'):
            # Class depth has output indentation growth; work still bounded.
            assert large['work'][key] < small['work'][key]*6+1024, (family, key, 'work scaling')
    for name in ('declarations-4.cpp', 'templates-4.cpp'):
        rows = [r for r in data['telemetry'] if r['input'] == name]
        assert len({r['output_sha256'] for r in rows}) == 1
        assert all(r['variant'] == 'B' and r['stats_enabled'] == (r['mode'] == 'B') for r in rows)
        summarize(rows, 'mode')
    print('PASS: frozen identities, exact outputs, protocol, startup, budgets, TU isolation and work scaling')


if __name__ == '__main__':
    mode = sys.argv[1]
    if mode == 'measure':
        measure(*map(pathlib.Path, sys.argv[2:5]))
    elif mode == 'verify':
        verify(json.loads(pathlib.Path(sys.argv[2]).read_text()), *map(pathlib.Path, sys.argv[3:5]))
    elif mode == 'report':
        data = json.loads(pathlib.Path(sys.argv[2]).read_text())
        print(json.dumps({name: summarize([r for r in data['observations'] if r['input'] == name])
                          for name in data['inputs']}, indent=2))
    else:
        raise SystemExit('measure A B output.json | verify output.json A B | report output.json')
