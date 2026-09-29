#!/usr/bin/env python3
"""Accumulated checkpoint audit performance: frozen ENTRY FINAL WORK OUTPUT; all observations."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
from benchmark import run, sha, text_size
ENTRY,FINAL,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:5]]
assert not OUT.exists()
WORK.mkdir(parents=True,exist_ok=True)
cpu=max(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu})
result=dict(protocol='one warmup each; four A/A samples; four ABBA blocks; separate compiler/runtime observations',
    cpu=cpu,platform=platform.platform(),flags=['--emit-lowir','-O0'],
    stage_commit='ac988ea33d4997b44e82baaca5a86623fff3127a',entry_commit='f65eae8d7d434735a0ce981173a347eb8f8a1e59',
    final_parent=run(['git','rev-parse','HEAD']).stdout.strip(),
    final_diff_sha256=__import__('hashlib').sha256(run(['git','diff','--','dev']).stdout.encode()).hexdigest(),
    host=run(['g++','--version']).stdout.splitlines()[0],
    binaries=[dict(path=str(p),sha256=sha(p),text_bytes=text_size(p)) for p in (ENTRY,FINAL)],
    harness_sha256=sha(__file__),backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),
    size_metric='compiler .text; sectionless native ELF payload proxy includes static data and EH tables',
    acceptance='PA21/O0 spec section 9; required semantic work and bounds, no optional optimizer or numeric gate',workloads={})
if len(sys.argv)>5:
    previous,post_eh=[Path(p).resolve() for p in sys.argv[5:7]]
    initial=result
    result=json.loads(previous.read_text())
    assert result['binaries']==initial['binaries']
    result['continuation']=dict(prior_path=str(previous),prior_sha256=sha(previous),
        harness_sha256=sha(__file__),post_eh_baseline=dict(path=str(post_eh),sha256=sha(post_eh),text_bytes=text_size(post_eh)),
        reason='Last-reviewed compiler predates source throw; three throwing-source rows compare checkpoint entry with final. Completed cumulative observations are unchanged.')
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
sources={name:(ENTRY,prior[name]['source']) for name in ('startup','auto-specializations-9600','runtime-calls','runtime-memory','runtime-floating','runtime-reference-captures','runtime-class-lists')}
sources['composition']=(ENTRY,(ROOT/'student.tests/pa21/audit105_trace.cpp').read_text())
sources['runtime-logical-temporaries']=(ENTRY,'int drops;struct G{~G()noexcept{++drops;}};bool use(const G&,bool v)noexcept{return v;}int f(int n){if((n&1)&&(use(G(),(n&2)!=0)&&true))return 1;return 0;}int main(){volatile int count=12000000;int sum=0;for(int i=0;i<count;++i)sum+=f(i);return sum!=3000000||drops!=6000000;}')
sources['runtime-private-scalar-member']=(ENTRY,'int drops;struct G{~G()noexcept{++drops;}int get()const noexcept{return 1;}};int f(){bool gate=true;int n=gate?G().get():0;return n;}int main(){volatile int count=12000000;int sum=0;for(int i=0;i<count;++i)sum+=f();return sum!=12000000||drops!=12000000;}')
for n in (256,1024):
    source='int drops;struct G{~G()noexcept{++drops;}};bool use(const G&,bool b)noexcept{return b;}'
    source+=''.join('int f%d(int x){if((x&1)&&(use(G(),(x&2)!=0)&&true))return 1;return 0;}'%i for i in range(n))
    source+='int main(){return f0(3)!=1||f%d(0)||drops!=1;}'%(n-1)
    sources['logical-functions-'+str(n)]=(ENTRY,source)
sources['runtime-nrvo']=(ENTRY,'int live;struct R{int n;R(int x):n(x){++live;}R(const R&r):n(r.n){++live;}~R(){--live;}};R f(int x){R r(x);return r;}int main(){volatile int count=6000000;int sum=0;for(int i=0;i<count;++i){R r=f(i&3);sum+=r.n;}return sum!=9000000||live;}')
sources['runtime-member-prefix']=(ENTRY,'int live;volatile bool fail;struct M{int n;M(int x):n(x){if(fail)throw x;++live;}M(const M&m):n(m.n){if(fail)throw n;++live;}~M(){--live;}};struct A{M a,b,c;};int f(int n){M m(n);A a={m,m,m};return a.a.n+a.b.n+a.c.n;}int main(){volatile int count=2000000;int sum=0;for(int i=0;i<count;++i)sum+=f(i&3);return sum!=9000000||live;}')
sources['runtime-array-defaults']=(ENTRY,'int live,made;struct G{~G()noexcept{}};struct M{M(const G&g=G()){++live;++made;}~M(){--live;}};struct A{M m[32];A(){}};int f(){A a;return live;}int main(){volatile int count=200000;int sum=0;for(int i=0;i<count;++i)sum+=f();return sum!=6400000||made!=6400000||live;}')
for n in (256,1024):
    source='int live;volatile bool fail;struct M{M(int n){if(fail)throw n;++live;}M(const M&){++live;}~M(){--live;}};struct A{M a,b,c;};'
    source+=''.join('int f%d(){M m(1);A a={m,m,m};return live;}'%i for i in range(n))
    source+='int main(){return f0()!=4||f%d()!=4||live;}'%(n-1)
    sources['construction-functions-'+str(n)]=(ENTRY,source)
for name,(baseline,source) in sources.items():
    if name in result['workloads'] and 'runtime' in result['workloads'][name]:
        assert result['workloads'][name]['source']==source
        continue
    if len(sys.argv)>5 and (name=='runtime-member-prefix' or name.startswith('construction-functions-')):
        baseline=post_eh
    if name in result['workloads'] and 'runtime' in result['workloads'][name]:
        assert result['workloads'][name]['source']==source
        continue
    src=WORK/(name+'.cpp');src.write_text(source)
    item=dict(source=source,source_sha256=sha(src),baseline=str(baseline),baseline_sha256=sha(baseline),outputs=[],
              comparison='equivalent checked behavior',runtime_timing='live checked loop' if name.startswith('runtime-') else 'startup control')
    result['workloads'][name]=item;commands=[];executables=[];save()
    for i,cc in enumerate((baseline,FINAL)):
        ir=WORK/(name+f'-{i}.lowir');exe=WORK/(name+f'-{i}')
        cmd=[cc,'--emit-lowir','-O0','-o',ir,src]
        stats=run([*cmd,'--stats','--validate-lowir'])
        hosted=name=='runtime-member-prefix' or name.startswith('construction-functions-')
        if hosted:
            obj=exe.with_suffix('.o')
            run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir])
            run(['g++','-no-pie',obj,'-o',exe])
        else:run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
        run([exe])
        item['outputs'].append(dict(binary=i,lowir_sha256=sha(ir),lowir_bytes=ir.stat().st_size,
            telemetry=[json.loads(s) for s in stats.stderr.splitlines()],
            native=dict(size_metric='actual ELF .text, supplied object backend and host linking' if hosted else 'sectionless ELF payload proxy',sha256=sha(exe),payload_bytes=text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)))
        commands.append(cmd);executables.append([exe])
    item['compiler']=measure(commands);item['runtime']=measure(executables)
    for output in item['outputs']:assert sha(WORK/(name+f"-{output['binary']}.lowir"))==output['lowir_sha256']
    save();print(name,'complete',flush=True)
assert [sha(p) for p in (ENTRY,FINAL)]==[b['sha256'] for b in result['binaries']]

# The old compiler does not implement these protected exits. Measure final-only
# checked costs; rejected/invalid LowIR is not an optimization baseline.
result['new_semantics']={}
for n in (256,1024):
    source='int drops;struct G{~G(){++drops;}};'
    source+=''.join('int f%d(int x){for(int i=0;i<2;++i){try{G g;if(x)throw x;break;}catch(int n){if(n==x)continue;return 9;}}return 0;}'%i for i in range(n))
    source+='int main(){return f0(7)||f%d(0)||drops!=3;}'%(n-1)
    src=WORK/('jump-'+str(n)+'.cpp');src.write_text(source)
    ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=src.with_suffix('')
    cmd=[FINAL,'--emit-lowir','-O0','-o',ir,src]
    stats=run([*cmd,'--stats','--validate-lowir'])
    run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe]);run([exe])
    result['new_semantics'][str(n)]=dict(source=source,source_sha256=sha(src),compiler_samples=[observe(cmd) for _ in range(4)],
        telemetry=[json.loads(s) for s in stats.stderr.splitlines()],lowir_bytes=ir.stat().st_size,lowir_sha256=sha(ir),
        host_text_bytes=text_size(exe),checked_exit=0,comparison='Final-only semantic scaling, no optimization comparison')
    save();print('Jump scaling',n,flush=True)
source='int drops;struct G{~G(){++drops;}};int f(int x){for(int i=0;i<2;++i){try{G g;if(x)throw x;break;}catch(int n){if(n==x)continue;return 9;}}return 0;}int main(){volatile int count=100000;int sum=0;for(int i=0;i<count;++i)sum+=f(i&1);return sum||drops!=150000;}'
src=WORK/'runtime-jumps.cpp';src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=src.with_suffix('')
cmd=[FINAL,'--emit-lowir','-O0','-o',ir,src];run(cmd)
run([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);run(['g++','-no-pie',obj,'-o',exe]);run([exe])
result['new_semantics']['runtime']=dict(source=source,source_sha256=sha(src),compiler_samples=[observe(cmd) for _ in range(4)],
    runtime_samples=[observe([exe]) for _ in range(4)],host_text_bytes=text_size(exe),checked_exit=0)
save()

# Repeat the cumulative template measurement because the first wall pairs
# were slower despite nearly equal separately observed phase times.
name='auto-specializations-9600'
src=WORK/(name+'.cpp')
commands=[[cc,'--emit-lowir','-O0','-o',WORK/(name+f'-repeat-{i}.lowir'),src] for i,cc in enumerate((ENTRY,FINAL))]
result['template_repeat']=measure(commands)
for i in (0,1):
    assert sha(WORK/(name+f'-repeat-{i}.lowir'))==result['workloads'][name]['outputs'][i]['lowir_sha256']
save()
