#!/usr/bin/env python3
"""Retain noisy-group repeats and a longer frozen initializer runtime."""
from pathlib import Path
import json
import os
import sys
import time
import audit_benchmark as audit

prior = audit.prior
ROOT = audit.ROOT


def main():
    common_path, affected_path, output, work = map(Path, sys.argv[1:5])
    common = json.loads(common_path.read_text()); affected = json.loads(affected_path.read_text())
    assert common['binaries'] == affected['binaries']
    for b in common['binaries']:
        assert prior.sha(b['path']) == b['sha256']
    cpu = common['cpu']; os.sched_setaffinity(0, {cpu})
    work = work.resolve(); work.mkdir(parents=True, exist_ok=True)
    data = dict(protocol='performance-protocol.md#independent-final-audit', cpu=cpu,
        binaries=common['binaries'], harness_sha256=prior.sha(__file__),
        sources={str(p): prior.sha(p) for p in (common_path, affected_path)},
        inputs={}, observations=[], runtime=[], native_startup=[])

    def save():
        output.write_text(json.dumps(data, indent=2)+'\n')

    def observe(command):
        usage = work/'usage.txt'; start = time.perf_counter_ns()
        result = prior.run(['/usr/bin/time', '-f', '%M %U %S %c %w', '-o', usage, *command])
        rss, user, system, involuntary, voluntary = usage.read_text().split()
        assert not result.stderr
        return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(rss),
                    user_s=float(user), system_s=float(system), involuntary=int(involuntary), voluntary=int(voluntary))

    for campaign, groups in ((common, ('memory-float-1', 'calls-4', 'references-8000')),
                             (affected, ('nested-initializers-4000', 'joined-literals-500', 'joined-literals-2000'))):
        for group in groups:
            entry = campaign['inputs'][group]
            assert prior.sha(entry['path']) == entry['sha256']
            data['inputs'][group] = dict(path=entry['path'], sha256=entry['sha256'])
            for ordinal, label in enumerate(prior.ORDER):
                row = observe([data['binaries'][label]['path'], '--emit-lowir', '-O0', '-o', work/'repeat.lowir', entry['path']])
                expected = entry['outputs'][label]['sha256'] if 'outputs' in entry else entry['output_hashes'][label]
                assert prior.sha(work/'repeat.lowir') == expected
                row.update(group=group, ordinal=ordinal, binary=label); data['observations'].append(row)
            save(); print('repeated', group, flush=True)

    source = work/'empty.cpp'; source.write_text('int main(){return 0;}')
    ir, exe = work/'empty.lowir', work/'empty'
    prior.run([data['binaries'][1]['path'], '--emit-lowir', '-O0', '-o', ir, source])
    prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir])
    for ordinal in range(4):
        row = observe([exe]); row['ordinal'] = ordinal; data['native_startup'].append(row)

    iterations = 2400000
    expected = ((iterations//256)*sum(range(256))+sum(range(iterations % 256))) & 65535
    source = work/'runtime-long.cpp'
    source.write_text(f'''struct A{{int a[1024];}};
int main(){{volatile int n={iterations};int sum=0;
for(int i=0;i<n;++i){{A x[2]={{}};int j=i&1023;x[0].a[j]=i&255;
sum=(sum+x[0].a[j]+x[1].a[(j+1)&1023])&65535;}}return sum!={expected};}}''')
    entry = dict(group='nested-zero-runtime-long', source_path=str(source), source_sha256=prior.sha(source),
        volatile_iterations=iterations, executables=[], observations=[])
    for label in (0, 1):
        ir, exe = work/f'runtime-{label}.lowir', work/f'runtime-{label}'
        prior.run([data['binaries'][label]['path'], '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, source])
        prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); prior.run([exe])
        entry['executables'].append(dict(path=str(exe), sha256=prior.sha(exe), text_bytes=prior.text_size(exe),
            lowir_path=str(ir), lowir_sha256=prior.sha(ir), exit_status=0))
    for ordinal, label in enumerate(prior.ORDER):
        row = observe([entry['executables'][label]['path']]); row.update(binary=label, ordinal=ordinal)
        entry['observations'].append(row)
    data['runtime'].append(entry); save(); prior.report(data)


if __name__ == '__main__':
    main()
