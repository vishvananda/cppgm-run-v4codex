#!/usr/bin/env python3
"""Frozen-input layout/initializer compiler and native execution evidence."""
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
    for n in (1000, 4000):
        record = 'struct alignas(16) Bits{unsigned a:3;int b:3;char c;};int run(){Bits b={5,-2,7};return b.a+b.b+b.c;}'
        yield f'layouts-{n}', ''.join(f'namespace N{i}{{{record}}}' for i in range(n))+f'int main(){{return N{n-1}::run()!=10;}}'
        record = 'struct Inner{int a[3];char text[4];};struct Outer{Inner inner;int last;};int run(){Outer o={1,2,3,"hi",7};return o.inner.a[2]+o.inner.text[1]+o.last;}'
        yield f'initializers-{n}', ''.join(f'namespace N{i}{{{record}}}' for i in range(n))+f'int main(){{return N{n-1}::run()!=115;}}'
    for n in (32, 1000000):
        yield f'zero-range-{n}', f'struct Large{{int values[{n}];}};int main(){{Large object={{}};return object.values[{n-1}];}}'
    for n in (32, 1000000):
        yield f'volatile-range-{n}', f'int main(){{volatile int values[{n}]={{7}};return values[0]!=7||values[{n-1}]!=0;}}'


def main():
    compiler, work, dest = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve(), Path(sys.argv[3])
    work.mkdir(parents=True, exist_ok=True)
    cpu=min(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
    data=dict(protocol='performance-protocol.md#layout-and-initializer-continuation-campaign',cpu=cpu,
        comparison='B-only AAAA: A lacks required semantics; no speedup claim',
        compiler=dict(sha256=prior.sha(compiler),text_bytes=prior.text_size(compiler)),
        implementation=prior.run(['git','rev-parse','HEAD'],cwd=ROOT).stdout.strip(),
        harness_sha256=prior.sha(__file__),backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
        flags=['--emit-lowir','-O0'],native_flags=['-O0'],inputs={},observations=[],runtime={})
    def save(): dest.write_text(json.dumps(data,indent=2)+'\n')
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
        data['inputs'][name]=dict(sha256=prior.sha(src),bytes=src.stat().st_size,lowir_sha256=prior.sha(ir),lowir_bytes=ir.stat().st_size,exit_status=0)
        for ordinal in range(4):
            row=observe(command); row.update(group=name,ordinal=ordinal)
            assert not row['stderr']; data['observations'].append(row)
        telemetry=observe([*command,'--stats']); telemetry['phases']=[json.loads(line) for line in telemetry.pop('stderr').splitlines()]
        data['inputs'][name]['telemetry']=telemetry; save(); print('measured',name,flush=True)
    n=48000000
    expected=((n//32)*sum(range(32))+sum(range(n%32))+2*n)&65535
    src,ir,exe=work/'runtime.cpp',work/'runtime.lowir',work/'runtime'
    src.write_text(f'''struct Bits{{unsigned a:5;unsigned b:3;int s:4;}};int main(){{
Bits bits={{0,5,-3}};volatile int n={n};int sum=0;
for(int i=0;i<n;++i){{bits.a=i;sum=(sum+bits.a+bits.b+bits.s)&65535;}}return sum!={expected};}}''')
    prior.run([compiler,'--emit-lowir','-O0','--validate-lowir','-o',ir,src]); prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]); prior.run([exe])
    data['runtime']=dict(source_sha256=prior.sha(src),lowir_sha256=prior.sha(ir),executable_sha256=prior.sha(exe),
        text_proxy_bytes=prior.text_size(exe),exit_status=0,volatile_iterations=n,observations=[])
    for ordinal in range(4):
        row=observe([exe]); row['ordinal']=ordinal; data['runtime']['observations'].append(row)
    save(); print('measured bit-field runtime',flush=True)


if __name__=='__main__': main()
