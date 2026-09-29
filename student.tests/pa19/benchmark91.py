#!/usr/bin/env python3
"""Frozen PA19/O0 evidence. Run A B WORK OUT [workload prefix]."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'student.tests/pa10'))
import benchmark as shared
A, B, WORK, OUT = map(Path, sys.argv[1:5])
A, B = A.resolve(), B.resolve()
WORK.mkdir(parents=True, exist_ok=True)
assert not OUT.exists(), 'Preserve all earlier observations'
assert not shared.run(['git', 'diff', 'HEAD', '--', 'dev']).stdout
cpu = max(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
result = dict(protocol='one warmup each, four A/A samples, four ABBA blocks; six final-only samples for new semantics',
    cpu=cpu, platform=platform.platform(), flags=['--emit-lowir', '-O0'],
    build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
    commits=['e5f4c3ed78972c8d161671d145bf525cb99033f4', shared.run(['git','rev-parse','HEAD']).stdout.strip()],
    harness_sha256=shared.sha(__file__), shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
    binaries=[dict(path=str(p), sha256=shared.sha(p), text_bytes=shared.text_size(p)) for p in (A,B)],
    backend=dict(sha256=shared.sha(ROOT/'reference-binaries/lowir2native'), flags=['-O0'], bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
    acceptance='PA19/O0: correctness costs measured separately; no optional transforms or new growth budget. Argument projection/substitution is linear in consumed types/arguments; completed lookups use canonical keys. No mandated numeric latency/RSS gate. Native optimization/self-hosting belong to later stages.',
    text_metric='compiler .text; supplied sectionless ELF executable payload proxy (includes static data)', workloads={})

def save(): OUT.write_text(json.dumps(result, indent=2)+'\n')
def observe(command):
    usage=WORK/'usage.txt'; start=time.perf_counter_ns()
    shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
    rss,user,system,iv,v=usage.read_text().split()
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(rss), user_s=float(user), system_s=float(system), involuntary=int(iv), voluntary=int(v), checked_exit=0)
def measure(commands):
    warmups=[dict(binary=i,**observe(c)) for i,c in commands.items()]
    order=[0]*4+[0,1,1,0]*4 if len(commands)==2 else [1]*6
    rows=[dict(binary=i,**observe(commands[i])) for i in order]
    data=dict(warmups=warmups, observations=rows)
    if len(commands)==2:
        aa=[r['wall_s'] for r in rows[:4]]; data['aa_range_s']=[min(aa),max(aa)]
        ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
        data['paired_b_over_a']=ratios; data['median_b_over_a']=statistics.median(ratios)
    for i in commands:
        samples=[r for r in rows[(4 if len(commands)==2 else 0):] if r['binary']==i]
        data[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in samples), wall_range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)], peak_rss_kib=max(r['rss_kib'] for r in samples))
    return data

sources={}
for n in (600,2400,9600):
    tag='template<int>struct Tag{};'
    mains='int main(){return f0()!=7||f'+str(n-1)+'()!=7;}'
    sources['ordinary-ordering-'+str(n)]=tag+'template<class T>int g(T){return 7;}template<class T,class...A>int g(T,A...){return 3;}'+''.join('int f'+str(i)+'(){return g(Tag<'+str(i)+'>());}' for i in range(n))+mains
    sources['member-alias-'+str(n)]=tag+'template<class,class>struct Pair{};template<template<class...>class F>using Apply=F<char>;template<class T>struct O{template<class U>using M=Pair<T,U>;using R=Apply<M>;};template<class,class>struct Same;template<class T>struct Same<T,T>{};'+''.join('Same<O<Tag<'+str(i)+'>>::R,Pair<Tag<'+str(i)+'>,char>> a'+str(i)+';' for i in range(n))+'int main(){}'
    sources['ordinary-member-'+str(n)]=sources['member-alias-'+str(n)].replace('using R=Apply<M>;', 'using R=Pair<T,char>;')
    sources['function-result-'+str(n)]=tag+'template<class T>T&& val();template<class>struct Result;template<class F,class...A>struct Result<F(A...)>{typedef decltype(val<F>()(val<A>()...)) type;};template<class>struct H{long operator()(int=0);};template<class T>struct B{template<class...A>typename Result<T(A...)>::type operator()(A&&...){return 7;}};'+''.join('int f'+str(i)+'(){B<H<Tag<'+str(i)+'>>> b;return b();}' for i in range(n))+mains
    sources['fixed-head-pack-'+str(n)]=tag+'template<class A,class B=int,class C=long>struct L{};template<class A,class...T>int g(L<A,T...>){return 5+sizeof...(T);}'+''.join('int f'+str(i)+'(){return g(L<Tag<'+str(i)+'>>());}' for i in range(n))+mains
    sources['empty-tail-ordering-'+str(n)]='template<class>struct Tag{};template<int>struct V{};template<class T>int g(T,int=0){return 3;}template<class T,class...A>int g(Tag<T>,A&&...){return 7;}'+''.join('int f'+str(i)+'(){return g(Tag<V<'+str(i)+'>>());}' for i in range(n))+mains
for name,source in shared.runtimes(4): sources['runtime-'+name]=source
sources['runtime-reference-cast']='int calls;struct X{int n;template<class T>operator T(){++calls;return T();}};template<class T>T&& forward(T&x){return static_cast<T&&>(x);}int main(){volatile int n=8000000;X x;x.n=0;for(int i=0;i<n;++i){X&&r=forward(x);r.n=(x.n+1)&65535;}return calls||x.n!=4608;}'
if len(sys.argv)>5: sources={k:v for k,v in sources.items() if k.startswith(sys.argv[5])}
empty=WORK/'empty.cpp';empty.write_text('int main(){}')
result['startup']=measure({i:[cc,'--emit-lowir','-O0','-o',WORK/f'empty-{i}.lowir',empty] for i,cc in enumerate((A,B))});save()
for name,source in sources.items():
    src=WORK/(name+'.cpp');src.write_text(source)
    item=dict(source=source,source_sha256=shared.sha(src),outputs=[]);result['workloads'][name]=item
    commands={}; executables={}
    for i,cc in enumerate((A,B)):
        ir=WORK/(name+f'-{i}.lowir');cmd=[cc,'--emit-lowir','-O0','-o',ir,src]
        r=subprocess.run([str(x) for x in [*cmd,'--validate-lowir','--stats']],capture_output=True,text=True,timeout=300)
        if r.returncode:
            assert i==0,(name,r.stderr)
            item['entry_rejection']=dict(exit=r.returncode,diagnostic=r.stderr);continue
        out=dict(binary=i,sha256=shared.sha(ir),bytes=ir.stat().st_size,telemetry=[json.loads(s) for s in r.stderr.splitlines()])
        exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
        native=subprocess.run([exe],capture_output=True,timeout=30)
        if native.returncode:
            assert i==0,(name,native.returncode)
            item['entry_wrong_execution']=dict(exit=native.returncode,ir_sha256=shared.sha(ir));continue
        out['native']=dict(sha256=shared.sha(exe),payload_bytes=shared.text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)
        item['outputs'].append(out);commands[i]=cmd;executables[i]=[exe]
    assert 1 in commands
    if len(commands)==2:
        same=item['outputs'][0]['sha256']==item['outputs'][1]['sha256']
        item['comparison']='exact' if same else 'checked-execution'
        if same: assert item['outputs'][0]['native']['sha256']==item['outputs'][1]['native']['sha256']
    else: item['comparison']='new-behavior'
    save();print(name,'preflight',flush=True)
    item['compiler']=measure(commands)
    if name.startswith('runtime-'): item['runtime']=measure(executables)
    save();print(name,'complete',flush=True)
