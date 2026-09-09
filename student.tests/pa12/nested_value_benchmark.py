#!/usr/bin/env python3
"""Equivalent-output A/A and ABBA evidence for shared conditional transfer slices."""
from pathlib import Path
import importlib.util
import json
import os
import statistics
import sys
import time
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('prior',ROOT/'student.tests/pa10/benchmark.py')
prior=importlib.util.module_from_spec(spec);spec.loader.exec_module(prior)
a,b,work,output=map(Path,sys.argv[1:5]);work.mkdir(parents=True,exist_ok=True)
binaries=[a.resolve(),b.resolve()]
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
data=dict(protocol='four A/A observations, two ABBA blocks; frozen equivalent outputs',cpu=cpu,
    binaries=[dict(path=str(x),sha256=prior.sha(x),text_bytes=prior.text_size(x)) for x in binaries],
    harness_sha256=prior.sha(__file__),backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
    compile_flags=['--emit-lowir','-O0'],native_flags=['-O0'],workloads={})
def observe(command):
    usage=work/'usage.txt';start=time.perf_counter_ns()
    prior.run(['/usr/bin/time','-f','%M','-o',usage,*command])
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()))
def campaign(commands):
    rows=[]
    for ordinal,label in enumerate(prior.ORDER):
        row=observe(commands[label]);row.update(ordinal=ordinal,binary=label);rows.append(row)
    ratios=[]
    for start in (4,8):
        block=rows[start:start+4]
        ratios.append(statistics.mean(x['wall_s'] for x in block if x['binary']==1)/statistics.mean(x['wall_s'] for x in block if x['binary']==0))
    return dict(observations=rows,paired_b_over_a=ratios,aa_range_s=[min(x['wall_s'] for x in rows[:4]),max(x['wall_s'] for x in rows[:4])])
for depth in (64,256):
    expression='S(n)'
    for j in range(depth): expression=f'b?S(n+{j}):({expression})'
    body='struct S{int a;S(int n):a(n){}};S f(bool b,int n){return '+expression+';}'
    count=100000;expected=sum((i&7)+(depth-1 if i&1 else 0) for i in range(count))&65535
    contents=''.join(f'namespace N{j}{{{body}}}' for j in range(16))
    contents+=f'int main(){{volatile int n={count};int sum=0;for(int i=0;i<n;++i)sum=(sum+N0::f((i&1)!=0,i&7).a)&65535;return sum!={expected};}}'
    name=f'nested-{depth}';source=work/(name+'.cpp');source.write_text(contents)
    irs=[work/(name+f'-{j}.lowir') for j in (0,1)];exes=[work/(name+f'-{j}') for j in (0,1)]
    commands=[[binary,'--emit-lowir','-O0','-o',ir,source] for binary,ir in zip(binaries,irs)]
    entry=dict(source_path=str(source),source_sha256=prior.sha(source),outputs=[])
    for label in (0,1):
        stats=prior.run([*commands[label],'--validate-lowir','--stats'])
        prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exes[label],irs[label]]);prior.run([exes[label]])
        entry['outputs'].append(dict(lowir_sha256=prior.sha(irs[label]),lowir_bytes=irs[label].stat().st_size,
            executable_sha256=prior.sha(exes[label]),text_bytes=prior.text_size(exes[label]),checked_exit=0,
            telemetry=[json.loads(line) for line in stats.stderr.splitlines()]))
    assert irs[0].read_bytes()==irs[1].read_bytes(), 'shared slices changed LowIR'
    assert exes[0].read_bytes()==exes[1].read_bytes(), 'shared slices changed native output'
    entry['compiler']=campaign(commands);entry['runtime']=campaign([[x] for x in exes]);data['workloads'][name]=entry
    output.write_text(json.dumps(data,indent=2)+'\n');print(name,entry['compiler']['paired_b_over_a'],flush=True)
