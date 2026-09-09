#!/usr/bin/env python3
"""PA11-only necessary semantic costs; see performance-protocol.md."""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import statistics
import subprocess
import sys
import time
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prior)


def source(count):
    return ''.join(f'''namespace N{i} {{
struct Field {{ int value; Field(int v) : value(v) {{}} }};
struct Object {{ Field field; int bias = 3; Object(int v) : field(v) {{}}
int get() const {{ return field.value + bias; }} }};
int run(int v) {{ Object object(v); return object.get(); }}
}}
''' for i in range(count)) + f'int main(){{return N{count-1}::run(39)-42;}}\n'


def main():
    compiler, work, dest = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve(), Path(sys.argv[3])
    work.mkdir(parents=True, exist_ok=True)
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    data = dict(protocol='performance-protocol.md', comparison='correct-candidate A/A only; no stage-base performance ratio',
                compiler=dict(path=str(compiler), sha256=prior.sha(compiler), text_bytes=prior.text_size(compiler)),
                implementation=prior.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT).stdout.strip(),
                harness_sha256=prior.sha(__file__), cpu=cpu,
                backend=dict(path=str(ROOT/'reference-binaries/lowir2native'), sha256=prior.sha(ROOT/'reference-binaries/lowir2native')),
                flags=['--emit-lowir', '-O0'], native_flags=['-O0'], inputs={}, observations=[], runtime={})
    def save(): dest.write_text(json.dumps(data, indent=2)+'\n')
    def observe(command):
        usage = work/'usage.txt'
        start = time.perf_counter_ns()
        result = prior.run(['/usr/bin/time', '-f', '%M %U %S', '-o', usage, *command])
        wall = (time.perf_counter_ns()-start)/1e9
        rss, user, system = usage.read_text().split()
        return dict(wall_s=wall, rss_kib=int(rss), user_s=float(user), system_s=float(system), stderr=result.stderr)
    for count in (1000, 4000):
        name = f'constructors-{count}'
        src, ir, exe = work/(name+'.cpp'), work/(name+'.lowir'), work/name
        src.write_text(source(count))
        command = [compiler, '--emit-lowir', '-O0', '-o', ir, src]
        prior.run([*command, '--validate-lowir'])
        prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); prior.run([exe])
        data['inputs'][name] = dict(path=str(src), sha256=prior.sha(src), lowir_sha256=prior.sha(ir), exit_status=0)
        for ordinal in range(4):
            row = observe(command); row.update(group=name, ordinal=ordinal)
            assert not row['stderr']; data['observations'].append(row)
        telemetry = observe([*command, '--stats'])
        telemetry['phases'] = [json.loads(line) for line in telemetry.pop('stderr').splitlines()]
        data['inputs'][name]['telemetry'] = telemetry
        save(); print('measured', name, flush=True)
    n = 96000000
    cycle = sum((i*17+3)&1023 for i in range(1024))
    expected = ((n//1024)*cycle + sum((i*17+3)&1023 for i in range(n%1024))) & 65535
    src, ir, exe = work/'constructor-runtime.cpp', work/'constructor-runtime.lowir', work/'constructor-runtime'
    src.write_text(f'''struct Field {{ int value; Field(int v) : value(v) {{}}
int next() const {{return (value*17+3)&1023;}} }};
int main(){{volatile int n={n};int sum=0;for(int i=0;i<n;++i){{Field f(i);sum=(sum+f.next())&65535;}}
return sum=={expected}?0:1;}}\n''')
    prior.run([compiler, '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, src])
    prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); prior.run([exe])
    data['runtime'] = dict(source=str(src), source_sha256=prior.sha(src), executable=str(exe),
                           executable_sha256=prior.sha(exe), text_proxy_bytes=prior.text_size(exe),
                           lowir_sha256=prior.sha(ir), exit_status=0, volatile_iterations=n, observations=[])
    for ordinal in range(4):
        row = observe([exe]); row['ordinal'] = ordinal; data['runtime']['observations'].append(row)
    save(); print('measured constructor runtime', flush=True)

if __name__ == '__main__': main()
