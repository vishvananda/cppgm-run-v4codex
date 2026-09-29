#!/usr/bin/env python3
"""Frozen PA20 range/O0 evidence: run ENTRY FINAL WORK OUTPUT.json, with fresh OUTPUT."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'student.tests/pa10'))
import benchmark as shared

A, B, WORK, OUT = [Path(p).resolve() for p in sys.argv[1:]]
assert not OUT.exists(), 'Preserve previous measurements'
WORK.mkdir(parents=True, exist_ok=True)
cpu = int(os.environ.get('PA20_BENCH_CPU',max(os.sched_getaffinity(0))))
assert cpu in os.sched_getaffinity(0)
os.sched_setaffinity(0, {cpu})
history = json.loads((ROOT/'student.tests/pa19/performance92.json').read_text())
backend = ROOT/'reference-binaries/lowir2native'
result = dict(protocol='one warmup each, four A/A observations, four ABBA blocks; six final-only observations for new behavior',
    cpu=cpu, platform=platform.platform(), flags=['--emit-lowir','-O0'],
    build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
    entry_commit=os.environ.get('PA20_ENTRY_COMMIT','ef897177cd34e8bf1e878a7eb94208237240c0f1'),
    final_commit=shared.run(['git','rev-parse','HEAD']).stdout.strip(),
    harness_sha256=shared.sha(__file__), shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
    backend=dict(path=str(backend),sha256=shared.sha(backend),flags=['-O0']),
    binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in (A,B)],
    text_metric='compiler .text; supplied sectionless native ELF payload proxy includes static data',
    workloads={})
def save(): OUT.write_text(json.dumps(result,indent=2)+'\n')
def observe(command):
    usage = WORK/'usage.txt'
    start = time.perf_counter_ns()
    shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
    rss, user, system, involuntary, voluntary = usage.read_text().split()
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),user_s=float(user),system_s=float(system),
        involuntary=int(involuntary),voluntary=int(voluntary),checked_exit=0)
def measure(commands):
    both = len(commands)==2
    warmups = [dict(binary=i,**observe(c)) for i,c in commands.items()]
    order = [0]*4+[0,1,1,0]*4 if both else [1]*6
    rows = [dict(binary=i,**observe(commands[i])) for i in order]
    data = dict(warmups=warmups,observations=rows)
    if both:
        aa = [r['wall_s'] for r in rows[:4]]
        data['aa_range_s'] = [min(aa),max(aa)]
        data['paired_b_over_a'] = [statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1)/
            statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0) for j in range(4,len(rows),4)]
    for i in commands:
        selected = [r for r in rows[4 if both else 0:] if r['binary']==i]
        data[str(i)] = dict(median_wall_s=statistics.median(r['wall_s'] for r in selected),
            wall_range_s=[min(r['wall_s'] for r in selected),max(r['wall_s'] for r in selected)],
            peak_rss_kib=max(r['rss_kib'] for r in selected))
    return data

sources = {'startup': 'int main(){}'}
for n in (2400,9600):
    sources['auto-specializations-'+str(n)] = 'template<int N>auto f(int x){return x+N;}int main(){int sum=0;' + ''.join(
        f'sum+=f<{i}>(1);sum+=f<{i}>(2);' for i in range(n)) + f'return sum!={n*(n+2)};}}'
    sources['namespace-variable-'+str(n)] = history['workloads']['namespace-variable-'+str(n)]['source']
for name in ('calls','memory','floating'):
    sources['runtime-'+name] = history['workloads']['runtime-'+name]['source']
sources['new-array-pack'] = 'template<class...T>int sum(T...x){int s=0;using A=int[];(void)A{0,(s+=x,0)...};return s;}int main(){volatile int n=12000000;int s=0;for(int i=0;i<n;++i)s=(s+sum(i&7,3,5))&65535;return s!=46720;}'
for n in (800,3200):
    sources['new-array-ranges-'+str(n)] = 'template<int N>int f(int x){int a[3]={x,N,1};int sum=0;for(auto v:a)sum+=v;return sum;}int main(){int sum=0;' + ''.join(
        f'sum+=f<{i}>(1);sum+=f<{i}>(2);' for i in range(n)) + f'return sum!={n*(n+4)};}}'
    sources['new-member-ranges-'+str(n)] = 'struct R{int*a;int*begin(){return a;}int*end(){return a+3;}};template<int N>int f(R&r){int sum=N;for(auto v:r)sum+=v;return sum;}int main(){int a[3]={1,2,3};R r={a};int sum=0;' + ''.join(
        f'sum+=f<{i}>(r);sum+=f<{i}>(r);' for i in range(n)) + f'return sum!={n*(n+11)};}}'
sources['new-runtime-range-array'] = 'int main(){volatile int limit=8000000;int a[3]={1,2,3};int sum=0;for(int i=0;i<limit;++i){a[0]=i&7;for(int x:a)sum=(sum+x)&65535;}return sum!=39168;}'
sources['new-runtime-range-class'] = 'struct I{int*p;int operator*(){return *p;}I&operator++(){++p;return *this;}bool operator!=(const I&o){return p!=o.p;}};struct R{int*a;I begin(){return I{a};}I end(){return I{a+3};}};int main(){volatile int limit=4000000;int a[3]={1,2,3};R r={a};int sum=0;for(int i=0;i<limit;++i){a[0]=i&7;for(int x:r)sum=(sum+x)&65535;}return sum!=52352;}'
if os.environ.get('PA20_BENCH_CASES'):
    selected = os.environ['PA20_BENCH_CASES'].split(',')
    sources = {name:sources[name] for name in selected}
for name, source in sources.items():
    src = WORK/(name+'.cpp'); src.write_text(source)
    item = dict(source=source,source_sha256=shared.sha(src),outputs=[],runtime_timing='startup-dominated control' if not name.startswith(('runtime-','new-')) else 'live checked loop')
    result['workloads'][name] = item
    commands, executables = {}, {}
    for i,cc in enumerate((A,B)):
        ir,exe = WORK/(name+f'-{i}.lowir'),WORK/(name+f'-{i}')
        command = [cc,'--emit-lowir','-O0','-o',ir,src]
        check = subprocess.run([str(x) for x in [*command,'--stats','--validate-lowir']],capture_output=True,text=True,timeout=300)
        if check.returncode:
            assert i==0 and name.startswith('new-'), (name,check.stderr)
            item['entry_rejection'] = dict(exit=check.returncode,diagnostic=check.stderr)
            continue
        shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]); shared.run([exe])
        item['outputs'].append(dict(binary=i,sha256=shared.sha(ir),bytes=ir.stat().st_size,
            telemetry=[json.loads(line) for line in check.stderr.splitlines()],
            native=dict(sha256=shared.sha(exe),payload_bytes=shared.text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)))
        commands[i],executables[i] = command,[exe]
    assert 1 in commands
    item['comparison'] = 'same correct fixed source and checked result' if len(commands)==2 else 'new behavior; no A/B benefit claim'
    save()
    item['compiler'] = measure(commands)
    item['runtime'] = measure(executables)
    for output in item['outputs']:
        assert shared.sha(WORK/(name+f"-{output['binary']}.lowir"))==output['sha256']
    save(); print(name,'complete',flush=True)
assert [shared.sha(p) for p in (A,B)] == [b['sha256'] for b in result['binaries']]
