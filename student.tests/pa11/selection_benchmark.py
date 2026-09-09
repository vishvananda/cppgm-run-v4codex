#!/usr/bin/env python3
"""Frozen-input measurements of newly required access/ADL/operator work."""
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
VALUE = '''class Value { int n;
friend int read(const Value& v) { return v.n; }
public: Value(int x):n(x){} int operator()(int x)const{return n+x;}
friend int operator+(const Value&v,int x){return read(v)+x;} };
'''


def workloads():
    for n in (1000, 4000):
        source = ''.join(f'namespace N{i}{{{VALUE} int run(){{Value v(7);return v(3)+(v+2)+read(v);}}}}' for i in range(n))
        yield f'families-{n}', source + f'int main(){{return N{n-1}::run()!=26;}}'
    for depth in (8, 32):
        source = 'struct Base {int value;friend int read(const Base&v){return v.value;}};'
        for i in range(depth):
            base = f'D{i-1}' if i else 'Base'
            source += f'struct D{i}:{base}{{}};'
        source += ''.join(f'int run{i}(const D{depth-1}&v){{return read(v);}}' for i in range(1000))
        source += f'int main(){{D{depth-1} v;v.value=7;return run999(v)!=7;}}'
        yield f'base-depth-{depth}', source


def main():
    compiler, work, dest = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve(), Path(sys.argv[3])
    work.mkdir(parents=True, exist_ok=True)
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    data = dict(protocol='performance-protocol.md#selection-continuation-campaign', cpu=cpu,
        comparison='B-only AAAA: the baseline lacks these semantics; no speedup claim',
        compiler=dict(sha256=prior.sha(compiler), text_bytes=prior.text_size(compiler)),
        implementation=prior.run(['git', 'rev-parse', 'HEAD'], cwd=ROOT).stdout.strip(),
        harness_sha256=prior.sha(__file__), backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
        flags=['--emit-lowir', '-O0'], native_flags=['-O0'], inputs={}, observations=[], runtime={})
    def save(): dest.write_text(json.dumps(data, indent=2)+'\n')
    def observe(command):
        usage = work/'usage.txt'; start = time.perf_counter_ns()
        result = prior.run(['/usr/bin/time', '-f', '%M %U %S', '-o', usage, *command])
        wall = (time.perf_counter_ns()-start)/1e9
        rss, user, system = usage.read_text().split()
        return dict(wall_s=wall, rss_kib=int(rss), user_s=float(user), system_s=float(system), stderr=result.stderr)
    for name, source in workloads():
        src, ir, exe = work/(name+'.cpp'), work/(name+'.lowir'), work/name
        src.write_text(source)
        command = [compiler, '--emit-lowir', '-O0', '-o', ir, src]
        prior.run([*command, '--validate-lowir'])
        prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); prior.run([exe])
        data['inputs'][name] = dict(sha256=prior.sha(src), bytes=src.stat().st_size,
            lowir_sha256=prior.sha(ir), lowir_bytes=ir.stat().st_size, exit_status=0)
        for ordinal in range(4):
            row = observe(command); row.update(group=name, ordinal=ordinal)
            assert not row['stderr']; data['observations'].append(row)
        telemetry = observe([*command, '--stats'])
        telemetry['phases'] = [json.loads(line) for line in telemetry.pop('stderr').splitlines()]
        data['inputs'][name]['telemetry'] = telemetry
        save(); print('measured', name, flush=True)
    iterations = 48000000
    expected = (sum(range(1024))*(iterations//1024)+sum(range(iterations%1024))+3*iterations)&65535
    src, ir, exe = work/'runtime.cpp', work/'runtime.lowir', work/'runtime'
    src.write_text(VALUE+f'''int main(){{volatile int n={iterations};int sum=0;
for(int i=0;i<n;++i){{Value v(i&1023);sum=(sum+(v+3))&65535;}}return sum!={expected};}}''')
    prior.run([compiler, '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, src])
    prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); prior.run([exe])
    data['runtime'] = dict(source_sha256=prior.sha(src), lowir_sha256=prior.sha(ir), executable_sha256=prior.sha(exe),
        text_proxy_bytes=prior.text_size(exe), exit_status=0, volatile_iterations=iterations, observations=[])
    for ordinal in range(4):
        row = observe([exe]); row['ordinal'] = ordinal; data['runtime']['observations'].append(row)
    save(); print('measured selection runtime', flush=True)


if __name__ == '__main__': main()
