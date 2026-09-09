#!/usr/bin/env python3
"""Necessary PA11 lifetime costs, with fixed source generators and raw evidence."""
from pathlib import Path
import importlib.util
import json
import os
import statistics
import sys
import time
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prior)
GUARD = '''struct Guard { int *sum; int value;
Guard(int *p,int n):sum(p),value(n){}
~Guard(){*sum=(*sum+value)&65535;} };
'''


def workloads():
    for n in (1000, 4000):
        objects = ''.join(f'Guard value{i}(p,1);' for i in range(n))
        yield f'lexical-{n}', GUARD + f'''int run(bool early,int*p){{{objects} if(early)return 7;return 9;}}
int main(){{int trace=0;int result=run(false,&trace);return result!=9||trace!={n};}}'''
        bodies = ''.join(f'namespace N{i}{{{GUARD} int run(int*p){{Guard a(p,1);Guard b(p,2);if(*p)return 7;return 9;}}}}' for i in range(n))
        yield f'bodies-{n}', bodies + f'int main(){{int trace=0;int result=N{n-1}::run(&trace);return result!=9||trace!=3;}}'
    for n in (12, 48000):
        yield f'array-{n}', f'''int created,destroyed;
struct G{{G(){{++created;}}~G(){{++destroyed;}}}};
int main(){{{{G elements[{n}];}}return created!={n}||destroyed!={n};}}'''


def main():
    compiler, work, dest = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve(), Path(sys.argv[3])
    work.mkdir(parents=True, exist_ok=True)
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    data = dict(protocol='performance-protocol.md#lifetime-continuation-campaign',
        comparison='B-only AAAA for newly correct semantics; no A/B speed claim', cpu=cpu,
        compiler=dict(path=str(compiler),sha256=prior.sha(compiler),text_bytes=prior.text_size(compiler)),
        implementation=prior.run(['git','rev-parse','HEAD'],cwd=ROOT).stdout.strip(),harness_sha256=prior.sha(__file__),
        backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),flags=['--emit-lowir','-O0'],
        native_flags=['-O0'],inputs={},observations=[],runtime={})
    def save(): dest.write_text(json.dumps(data,indent=2)+'\n')
    def observe(command):
        usage=work/'usage.txt'; start=time.perf_counter_ns()
        result=prior.run(['/usr/bin/time','-f','%M %U %S','-o',usage,*command])
        wall=(time.perf_counter_ns()-start)/1e9
        rss,user,system=usage.read_text().split()
        return dict(wall_s=wall,rss_kib=int(rss),user_s=float(user),system_s=float(system),stderr=result.stderr)
    for name,source in workloads():
        src,ir,exe=work/(name+'.cpp'),work/(name+'.lowir'),work/name
        src.write_text(source)
        command=[compiler,'--emit-lowir','-O0','-o',ir,src]
        prior.run([*command,'--validate-lowir'])
        prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]); prior.run([exe])
        data['inputs'][name]=dict(path=str(src),sha256=prior.sha(src),lowir_sha256=prior.sha(ir),lowir_bytes=ir.stat().st_size,exit_status=0)
        for ordinal in range(4):
            row=observe(command);row.update(group=name,ordinal=ordinal)
            assert not row['stderr'];data['observations'].append(row)
        telemetry=observe([*command,'--stats'])
        telemetry['phases']=[json.loads(line) for line in telemetry.pop('stderr').splitlines()]
        data['inputs'][name]['telemetry']=telemetry
        save();print('measured',name,flush=True)
    n=48000000
    expected=sum(range(1024))*(n//1024)+sum(range(n%1024));expected &= 65535
    src,ir,exe=work/'runtime.cpp',work/'runtime.lowir',work/'runtime'
    src.write_text(GUARD+f'''int main(){{volatile int n={n};int sum=0;
for(int i=0;i<n;++i){{Guard g(&sum,i&1023);}}return sum!={expected};}}''')
    prior.run([compiler,'--emit-lowir','-O0','--validate-lowir','-o',ir,src])
    prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);prior.run([exe])
    data['runtime']=dict(source_path=str(src),source_sha256=prior.sha(src),lowir_sha256=prior.sha(ir),executable_sha256=prior.sha(exe),
        text_proxy_bytes=prior.text_size(exe),exit_status=0,volatile_iterations=n,observations=[])
    for ordinal in range(4):
        row=observe([exe]);row['ordinal']=ordinal;data['runtime']['observations'].append(row)
    save();print('measured lifetime runtime',flush=True)

if __name__=='__main__': main()
