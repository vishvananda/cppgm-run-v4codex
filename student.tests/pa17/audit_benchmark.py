#!/usr/bin/env python3
"""Frozen cumulative/checkpoint PA17 audit: A B WORK OUT [stage-base]."""
from pathlib import Path
import json, os, platform, statistics, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
A,B,WORK,OUT=map(Path,sys.argv[1:5])
A=A.resolve();B=B.resolve();WORK.mkdir(parents=True,exist_ok=True)
base=len(sys.argv)>5 and sys.argv[5]=='stage-base'
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
prior=ROOT/'student.tests/pa17/definition-performance.json'
result=dict(protocol='one warmup per binary; four A/A samples; four ABBA blocks',
    comparison='stage-base' if base else 'checkpoint',cpu=cpu,platform=platform.platform(),
    flags=['--emit-lowir','-O0'],build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
    host=shared.run(['g++','--version']).stdout.splitlines()[0],
    source_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),
    source_diff=shared.run(['git','diff','HEAD','--','dev']).stdout,
    harness_sha256=shared.sha(__file__),shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
    corpus_sha256=shared.sha(prior),
    binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in (A,B)],
    backend=dict(path=str(ROOT/'reference-binaries/lowir2native'),sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
        flags=['-O0'],bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98'),
    acceptance='PA17/O0: preserve required semantics, bounded work and all measurements; no numerical exit ceiling or optional transform.',
    native_size_metric='Executable payload after ELF entry; these runtime sources contain no static data.',workloads={})

def observe(command):
    usage=WORK/'usage.txt';start=time.perf_counter_ns()
    shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
    rss,user,system,iv,v=usage.read_text().split()
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),
        system_s=float(system),involuntary=int(iv),voluntary=int(v),checked_exit=0)

def measure(commands):
    warmups=[dict(binary=i,**observe(c)) for i,c in enumerate(commands)]
    rows=[dict(binary=i,**observe(commands[i])) for i in [0]*4+[0,1,1,0]*4]
    aa=[r['wall_s'] for r in rows[:4]]
    ratios=[statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/
        statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
    data=dict(warmups=warmups,observations=rows,aa_range_s=[min(aa),max(aa)],
        paired_b_over_a=ratios,median_b_over_a=statistics.median(ratios))
    for i in (0,1):
        samples=[r for r in rows[4:] if r['binary']==i]
        data[str(i)]=dict(median_wall_s=statistics.median(r['wall_s'] for r in samples),
            wall_range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],
            peak_rss_kib=max(r['rss_kib'] for r in samples))
    return data

corpus=[]
for name,w in json.loads(prior.read_text())['workloads'].items():
    if base and not name.startswith(('common-partials-','common-loop-float-','runtime-')):continue
    corpus.append((name,w['source'],'runtime' in w))
if not base:
    for width in (128,512):
        params=','.join('class T'+str(j) for j in range(width))
        names=','.join('T'+str(j) for j in range(width))
        prefix='template<int N>struct Tag{};'
        alias=prefix+f'template<{params}>using A=void({names});\n'
        partial=prefix+f'template<class...>struct L{{}};template<class,class>struct P;template<{params},class...U>struct P<L<{names}>,L<U...>>{{static const bool v=true;}};\n'
        for i in range(300):
            args=','.join(f'Tag<{i}>' for _ in range(width))
            alias+=f'static_assert(sizeof(A<{args}>*)==8,"");\n'
            partial+=f'static_assert(P<L<{args}>,L<int,char>>::v,"");\n'
        corpus.extend([(f'wide-alias-{width}',alias,False),(f'wide-partial-{width}',partial,False)])

def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
result['started_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime())
for name,source,native in corpus:
    src=WORK/(name+'.cpp');src.write_text(source)
    item=dict(source=source,source_sha256=shared.sha(src),comparison='exact',outputs=[])
    result['workloads'][name]=item;commands=[];executables=[]
    for i,binary in enumerate((A,B)):
        ir=WORK/(name+f'-{i}.lowir');cmd=[binary,'--emit-lowir','-O0','-o',ir,src];commands.append(cmd)
        r=shared.run([*cmd,'--stats','--validate-lowir'])
        out=dict(binary=i,path=str(ir),sha256=shared.sha(ir),bytes=ir.stat().st_size,
            telemetry=[json.loads(s) for s in r.stderr.splitlines()]);item['outputs'].append(out)
        if native:
            exe=WORK/(name+f'-{i}');shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);shared.run([exe]);executables.append([exe])
            out['native']=dict(path=str(exe),sha256=shared.sha(exe),text_bytes=shared.text_size(exe),checked_exit=0)
    assert len({o['sha256'] for o in item['outputs']})==1,name
    if native:assert len({o['native']['sha256'] for o in item['outputs']})==1,name
    print(name,'preflight',flush=True);save()
    item['compiler']=measure(commands)
    if native:item['runtime']=measure(executables)
    for o in item['outputs']:assert shared.sha(o['path'])==o['sha256']
    save();print(name,'measured',flush=True)
for b in result['binaries']:assert shared.sha(b['path'])==b['sha256']
result['finished_utc']=time.strftime('%Y-%m-%d %H:%M:%S',time.gmtime());save()
