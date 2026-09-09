#!/usr/bin/env python3
"""B-only construction family scaling and checked native runtime observations."""
from pathlib import Path
import importlib.util
import json
import os
import sys
import time
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec); spec.loader.exec_module(prior)


def workloads():
    families = {
        'inherited': ('struct Base{int x;Base(int n):x(n){}};struct D:Base{using Base::Base;};int run(){D d(7);return d.x-7;}', 'run()'),
        'converting': ('struct Box{int x;Box(int n):x(n+2){}};int read(const Box& b){return b.x;}int run(){return read(5)-7;}', 'run()'),
        'constant-arrays': ('struct Entry{const char*name;int value;Entry(const char*n,int v):name(n),value(v){}};Entry a[]={{"a",3},{"b",4}};int run(){return a[1].value-4;}', 'run()'),
        'tls': ('struct Item{int x;Item():x(7){}};thread_local Item value;int run(){return value.x-7;}', 'run()'),
    }
    for name, (record, call) in families.items():
        for n in (1000, 4000):
            yield f'{name}-{n}', ''.join(f'namespace N{i}{{{record}}}' for i in range(n))+f'int main(){{return N{n-1}::{call};}}'


def main():
    compiler, work, destination = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve(), Path(sys.argv[3])
    work.mkdir(parents=True, exist_ok=True)
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    data = dict(protocol='performance-protocol.md#construction-completion-campaign', cpu=cpu,
        comparison='B-only AAAA; baseline lacks these behaviors, no speedup claim',
        compiler=dict(path=str(compiler), sha256=prior.sha(compiler), text_bytes=prior.text_size(compiler)),
        implementation=prior.run(['git','rev-parse','HEAD'],cwd=ROOT).stdout.strip(),
        harness_sha256=prior.sha(__file__), backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
        flags=['--emit-lowir','-O0'], native_flags=['-O0'], inputs={}, observations=[], runtime={})
    def save(): destination.write_text(json.dumps(data,indent=2)+'\n')
    def observe(command):
        usage=work/'usage.txt'; start=time.perf_counter_ns()
        result=prior.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
        wall=(time.perf_counter_ns()-start)/1e9
        rss,user,system,involuntary,voluntary=usage.read_text().split()
        return dict(wall_s=wall,rss_kib=int(rss),user_s=float(user),system_s=float(system),
            involuntary=int(involuntary),voluntary=int(voluntary),stderr=result.stderr)
    for name,source in workloads():
        src,ir,exe=work/(name+'.cpp'),work/(name+'.lowir'),work/name
        src.write_text(source); command=[compiler,'--emit-lowir','-O0','-o',ir,src]
        prior.run([*command,'--validate-lowir']); prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]); prior.run([exe])
        data['inputs'][name]=dict(path=str(src),sha256=prior.sha(src),bytes=src.stat().st_size,lowir_sha256=prior.sha(ir),lowir_bytes=ir.stat().st_size,executable_sha256=prior.sha(exe),exit_status=0)
        for ordinal in range(4):
            row=observe(command); row.update(group=name,ordinal=ordinal)
            assert not row['stderr']; data['observations'].append(row)
        telemetry=observe([*command,'--stats']); telemetry['phases']=[json.loads(line) for line in telemetry.pop('stderr').splitlines()]
        data['inputs'][name]['telemetry']=telemetry; save(); print('measured',name,flush=True)
    n=8000000
    expected=((n//32)*sum((i&15)+i+2 for i in range(32)))&65535
    src,ir,exe=work/'runtime.cpp',work/'runtime.lowir',work/'runtime'
    src.write_text(f'''struct Base{{int x;Base(int n):x(n){{}}}};struct D:Base{{using Base::Base;}};
struct Box{{int x;Box(int n):x(n+2){{}}}};int read(const Box&b){{return b.x;}}
int main(){{volatile int n={n};int sum=0;for(int i=0;i<n;++i){{D d(i&15);sum=(sum+d.x+read(i&31))&65535;}}return sum!={expected};}}''')
    prior.run([compiler,'--emit-lowir','-O0','--validate-lowir','-o',ir,src]); prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]); prior.run([exe])
    data['runtime']=dict(source_path=str(src),source_sha256=prior.sha(src),lowir_sha256=prior.sha(ir),executable_sha256=prior.sha(exe),
        text_proxy_bytes=prior.text_size(exe),exit_status=0,volatile_iterations=n,observations=[])
    for ordinal in range(4):
        row=observe([exe]); row['ordinal']=ordinal; data['runtime']['observations'].append(row)
    save(); print('measured construction runtime',flush=True)


if __name__=='__main__': main()
