#!/usr/bin/env python3
"""Absolute cost of newly correct PA12 rules; no comparison to a rejecting baseline."""
from pathlib import Path
import importlib.util
import json
import os
import sys
import time
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec); spec.loader.exec_module(prior)
compiler, work, destination = map(Path, sys.argv[1:4])
compiler=compiler.resolve(); work.mkdir(parents=True, exist_ok=True)
cpu=min(os.sched_getaffinity(0)); os.sched_setaffinity(0,{cpu})
data=dict(protocol='newly supported semantics, absolute measurements only', cpu=cpu,
    compiler_sha256=prior.sha(compiler), compiler_text_bytes=prior.text_size(compiler),
    harness_sha256=prior.sha(__file__), backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
    compile_flags=['--emit-lowir','-O0'], native_flags=['-O0'], workloads={})
for count in (1000,4000):
    name=f'qualified-delegation-{count}'; source=work/(name+'.cpp'); ir=work/(name+'.lowir'); exe=work/name
    body='struct A{int x;A():A(7){}A(int v):x(v){}int get()&{return x;}int get()&&{return x+1;}};int run(){A a;return a.get()+A().get();}'
    source.write_text(''.join(f'namespace N{i}{{{body}}}' for i in range(count))+f'int main(){{return N{count-1}::run()-15;}}')
    command=[compiler,'--emit-lowir','-O0','-o',ir,source]
    rows=[]
    for ordinal in range(4):
        start=time.perf_counter_ns(); usage=work/'usage.txt'
        prior.run(['/usr/bin/time','-f','%M','-o',usage,*command])
        rows.append(dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(usage.read_text()),ordinal=ordinal))
    stats=prior.run([*command,'--stats','--validate-lowir'])
    prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]); prior.run([exe])
    runtimes=[]
    for ordinal in range(4):
        start=time.perf_counter_ns(); prior.run([exe]);runtimes.append((time.perf_counter_ns()-start)/1e9)
    data['workloads'][name]=dict(source_path=str(source),source_sha256=prior.sha(source),lowir_sha256=prior.sha(ir),
        lowir_bytes=ir.stat().st_size,executable_sha256=prior.sha(exe),text_bytes=prior.text_size(exe),checked_exit=0,
        compiler_observations=rows,runtime_s=runtimes,runtime_note='startup-sized correctness probe; no runtime profit claim',
        telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
    destination.write_text(json.dumps(data,indent=2)+'\n');print(name,rows,flush=True)
