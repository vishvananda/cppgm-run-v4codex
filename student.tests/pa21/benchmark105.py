#!/usr/bin/env python3
"""Audit performance: frozen STAGE ENTRY FINAL WORK OUTPUT; all observations."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run, sha, text_size
from audit105 import INFO, BASE
STAGE,ENTRY,FINAL,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:]]
assert not OUT.exists()
WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each; four A/A samples; four ABBA blocks; separate compiler/runtime observations',
    cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],
    stage_commit='ac988ea33d4997b44e82baaca5a86623fff3127a',entry_commit='3b87e462701e268eeaaaa9c9ddeb4594ec74439b',
    final_commit=run(['git','rev-parse','HEAD']).stdout.strip(),
    host=run(['g++','--version']).stdout.splitlines()[0],
    binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (STAGE,ENTRY,FINAL)],
    harness_sha256=sha(__file__),backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),
    size_metric='compiler .text; sectionless native ELF payload proxy includes static data and EH tables',
    acceptance='PA21/O0 spec section 9; required semantic work and bounds, no optional optimizer or numeric gate',workloads={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
def observe(cmd):
    usage=WORK/'usage.txt';start=time.perf_counter_ns()
    run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*cmd])
    rss,user,system,involuntary,voluntary=usage.read_text().split()
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),
                involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def measure(commands):
    warmups=[dict(binary=i,**observe(cmd)) for i,cmd in enumerate(commands)]
    rows=[dict(binary=i,**observe(commands[i])) for i in [0]*4+[0,1,1,0]*4]
    out=dict(warmups=warmups,observations=rows,aa_range_s=[min(r['wall_s'] for r in rows[:4]),max(r['wall_s'] for r in rows[:4])])
    out['paired_b_over_a']=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
    for i in (0,1):
        values=[r for r in rows[4:] if r['binary']==i]
        out[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in values),wall_range_s=[min(r['wall_s'] for r in values),max(r['wall_s'] for r in values)],peak_rss_kib=max(r['rss_kib'] for r in values))
    return out
prior=json.loads((ROOT/'student.tests/pa21/performance104.json').read_text())['workloads']
sources={name:(STAGE,prior[name]['source']) for name in ('startup','auto-specializations-9600','runtime-calls','runtime-memory','runtime-floating','runtime-reference-captures')}
for n in (800,3200):
    source=INFO+BASE+'template<int N>int f(B*p){return (dynamic_cast<D*>(p)!=0)+(typeid(*p)==typeid(D));}int main(){B b;D d;int sum=0;'
    source+=''.join(f'sum+=f<{i}>(&b);sum+=f<{i}>(&d);' for i in range(n))+f'return sum!={2*n};}}'
    sources['fixed-rtti-'+str(n)]=(ENTRY,source)
sources['list-specializations-3200']=(ENTRY,prior['list-specializations-3200']['source'])
capture=json.loads((ROOT/'student.tests/pa21/performance103.json').read_text())['workloads']
sources['capture-specializations-3200']=(ENTRY,capture['copy-capture-specializations-3200']['source'])
for name in ('runtime-scalar-lists','runtime-class-lists'):
    sources[name]=(ENTRY,prior[name]['source'])
rtti=json.loads((ROOT/'student.tests/pa21/performance102.json').read_text())['workloads']
for name in ('runtime-typeid','runtime-cast'):
    sources[name]=(ENTRY,rtti[name]['source'])
sources['runtime-copy-prefix']=(ENTRY,'int drops,copies;struct E{int n;E(int x):n(x){}~E(){++drops;}};struct S{int n;S(int x):n(x){}S(const S&s):n(s.n){++copies;}};int f(int n){E e(n);S s(n);auto a=[e,s](){return e.n+s.n;};auto b=a;return b();}int main(){volatile int n=1000000;int sum=0;for(int i=0;i<n;++i)sum+=f(i&1);return sum!=1000000||copies!=2000000||drops!=3000000;}')
sources['composition']=(ENTRY,(ROOT/'student.tests/pa21/audit105_trace.cpp').read_text())
for name,(baseline,source) in sources.items():
    src=WORK/(name+'.cpp');src.write_text(source)
    item=dict(source=source,source_sha256=sha(src),baseline=str(baseline),outputs=[],
              comparison='equivalent checked behavior',runtime_timing='live checked loop' if name.startswith('runtime-') else 'startup control')
    result['workloads'][name]=item;commands=[];executables=[];save()
    for i,cc in enumerate((baseline,FINAL)):
        ir=WORK/(name+f'-{i}.lowir');exe=WORK/(name+f'-{i}')
        cmd=[cc,'--emit-lowir','-O0','-o',ir,src]
        stats=run([*cmd,'--stats','--validate-lowir'])
        run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
        item['outputs'].append(dict(binary=i,lowir_sha256=sha(ir),lowir_bytes=ir.stat().st_size,
            telemetry=[json.loads(s) for s in stats.stderr.splitlines()],
            native=dict(sha256=sha(exe),payload_bytes=text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)))
        commands.append(cmd);executables.append([exe])
    item['compiler']=measure(commands);item['runtime']=measure(executables)
    for output in item['outputs']:assert sha(WORK/(name+f"-{output['binary']}.lowir"))==output['lowir_sha256']
    save();print(name,'complete',flush=True)
assert [sha(p) for p in (STAGE,ENTRY,FINAL)]==[b['sha256'] for b in result['binaries']]
