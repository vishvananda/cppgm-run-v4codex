#!/usr/bin/env python3
"""Frozen A/B evidence for the final PA11 initialization and spelling audit."""
from pathlib import Path
import importlib.util
import json
import os
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prior)


def workloads():
    for count in (1000, 4000):
        body = 'struct A{int values[32];};int run(){A a[2]={};return a[1].values[31];}'
        yield f'nested-initializers-{count}', ''.join(f'namespace N{i}{{{body}}}' for i in range(count))+f'int main(){{return N{count-1}::run();}}'
    for count in (500, 2000):
        text = 'ab'*2048
        source = ''.join(f'namespace L{i}{{const char*p="{i:08d}" "{text}";}}' for i in range(count))
        yield f'joined-literals-{count}', source+f'int main(){{return L{count-1}::p[4103]!=98;}}'


def main():
    a, b, work, destination = map(Path, sys.argv[1:5])
    work = work.resolve(); work.mkdir(parents=True, exist_ok=True)
    binaries = [a.resolve(), b.resolve()]
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    data = dict(protocol='performance-protocol.md#independent-final-audit', cpu=cpu,
        binaries=[dict(path=str(p), sha256=prior.sha(p), text_bytes=prior.text_size(p)) for p in binaries],
        implementation=prior.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT).stdout.strip(),
        harness_sha256=prior.sha(__file__), backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
        flags=['--emit-lowir', '-O0'], native_flags=['-O0'], inputs={}, observations=[], runtime=[], startup=[])

    def save():
        destination.write_text(json.dumps(data, indent=2)+'\n')

    def observe(command):
        usage = work/'usage.txt'; start = time.perf_counter_ns()
        result = prior.run(['/usr/bin/time', '-f', '%M %U %S %c %w', '-o', usage, *command])
        wall = (time.perf_counter_ns()-start)/1e9
        rss, user, system, involuntary, voluntary = usage.read_text().split()
        return dict(wall_s=wall, rss_kib=int(rss), user_s=float(user), system_s=float(system),
                    involuntary=int(involuntary), voluntary=int(voluntary), stderr=result.stderr)

    source = work/'empty.cpp'; source.write_text('int main(){return 0;}')
    for label in (0, 1):
        for ordinal in range(4):
            row = observe([binaries[label], '--emit-lowir', '-O0', '-o', work/'empty.lowir', source])
            row.update(binary=label, ordinal=ordinal); data['startup'].append(row)
    for name, text in workloads():
        source = work/(name+'.cpp'); source.write_text(text)
        entry = dict(path=str(source), sha256=prior.sha(source), outputs=[], telemetry=[])
        outputs = [work/(name+f'-{label}.lowir') for label in (0, 1)]
        for label in (0, 1):
            command = [binaries[label], '--emit-lowir', '-O0', '-o', outputs[label], source]
            prior.run([*command, '--validate-lowir'])
            exe = work/(name+f'-{label}')
            prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, outputs[label]])
            prior.run([exe])
            entry['outputs'].append(dict(path=str(outputs[label]), sha256=prior.sha(outputs[label]),
                bytes=outputs[label].stat().st_size, executable_path=str(exe), executable_sha256=prior.sha(exe), exit_status=0))
            telemetry = observe([*command, '--stats'])
            telemetry['phases'] = [json.loads(line) for line in telemetry.pop('stderr').splitlines()]
            entry['telemetry'].append(telemetry)
        for ordinal, label in enumerate(prior.ORDER):
            row = observe([binaries[label], '--emit-lowir', '-O0', '-o', outputs[label], source])
            assert not row['stderr']; row.update(binary=label, ordinal=ordinal, group=name)
            data['observations'].append(row)
        data['inputs'][name] = entry; save(); print('measured', name, flush=True)

    iterations = 120000
    expected = sum(i & 255 for i in range(iterations)) & 65535
    source = work/'runtime.cpp'
    source.write_text(f'''struct A{{int a[1024];}};
int main(){{volatile int n={iterations};int sum=0;
for(int i=0;i<n;++i){{A x[2]={{}};int j=i&1023;x[0].a[j]=i&255;
sum=(sum+x[0].a[j]+x[1].a[(j+1)&1023])&65535;}}return sum!={expected};}}''')
    entry = dict(group='nested-zero-runtime', source_path=str(source), source_sha256=prior.sha(source),
        volatile_iterations=iterations, executables=[], observations=[])
    for label in (0, 1):
        ir, exe = work/f'runtime-{label}.lowir', work/f'runtime-{label}'
        prior.run([binaries[label], '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, source])
        prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); prior.run([exe])
        entry['executables'].append(dict(path=str(exe), sha256=prior.sha(exe), text_bytes=prior.text_size(exe),
            lowir_path=str(ir), lowir_sha256=prior.sha(ir), exit_status=0))
    for ordinal, label in enumerate(prior.ORDER):
        row = observe([entry['executables'][label]['path']]); row.update(binary=label, ordinal=ordinal)
        entry['observations'].append(row)
    data['runtime'].append(entry); save(); prior.report(data)


if __name__ == '__main__':
    main()
