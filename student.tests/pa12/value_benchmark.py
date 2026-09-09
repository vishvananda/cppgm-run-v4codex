#!/usr/bin/env python3
"""Absolute and scaling evidence for new value paths; no unsupported baseline claim."""
from pathlib import Path
import importlib.util
import json
import os
import sys
import time
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior',ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec); spec.loader.exec_module(prior)
binary,work,output = map(Path,sys.argv[1:4]); binary=binary.resolve(); work.mkdir(parents=True,exist_ok=True)
cpu=min(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
data=dict(protocol='frozen final compiler; one warmup then six observations per workload; absolute/scaling only',
    cpu=cpu,binary=dict(path=str(binary),sha256=prior.sha(binary),text_bytes=prior.text_size(binary)),
    harness_sha256=prior.sha(__file__),backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
    compile_flags=['--emit-lowir','-O0'],native_flags=['-O0'],workloads={})
def observe(command):
    usage=work/'usage.txt'; start=time.perf_counter_ns()
    prior.run(['/usr/bin/time','-f','%M','-o',usage,*command])
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()))
def campaign(command):
    warmup=observe(command)
    return dict(warmup=warmup,observations=[observe(command) for _ in range(6)])
body = """struct Small{int a,b;Small(int n):a(n),b(n+1){}};
Small small(int n){return Small(n);} int read(Small x){return x.a+x.b;}
Small choose(bool b,int n){return b?small(n):small(n+1);}
struct Large{long a,b,c;Large(long n):a(n),b(n+1),c(n+2){}};
Large large(long n){return Large(n);} long read(Large x){return x.a+x.b+x.c;}
int run(int n){return read(choose((n&1)!=0,n))+read(large(n));}
"""
sources=[]
for count in (1000,4000):
    sources.append((f'values-{count}',''.join(f'namespace N{i}{{{body}}}' for i in range(count))+f'int main(){{return N{count-1}::run(7)-39;}}'))
count=2000000
expected=((count//1024)*sum(5*i+4+(0 if i&1 else 2) for i in range(1024))+sum(5*i+4+(0 if i&1 else 2) for i in range(count%1024)))&65535
sources.append(('value-runtime',body+f'int main(){{volatile int n={count};int sum=0;for(int i=0;i<n;++i)sum=(sum+run(i&1023))&65535;return sum!={expected};}}'))
for name,contents in sources:
    source=work/(name+'.cpp');source.write_text(contents)
    ir=work/(name+'.lowir');exe=work/name
    command=[binary,'--emit-lowir','-O0','-o',ir,source]
    stats=prior.run([*command,'--validate-lowir','--stats'])
    prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);prior.run([exe])
    entry=dict(source_path=str(source),source_sha256=prior.sha(source),lowir_sha256=prior.sha(ir),lowir_bytes=ir.stat().st_size,
        executable_sha256=prior.sha(exe),text_bytes=prior.text_size(exe),checked_exit=0,
        telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
    entry['compiler']=campaign(command);entry['runtime']=campaign([exe]);data['workloads'][name]=entry
    output.write_text(json.dumps(data,indent=2)+'\n'); print(name,flush=True)
