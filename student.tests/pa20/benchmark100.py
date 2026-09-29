#!/usr/bin/env python3
"""Frozen PA20 retained-angle grammar O0 evidence: run ENTRY FINAL WORK OUTPUT.json, with fresh OUTPUT."""
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
result = dict(protocol='one warmup each, four A/A observations, four ABBA blocks',
    cpu=cpu, platform=platform.platform(), flags=['--emit-lowir','-O0'],
    build_flags='g++ -std=gnu++11 -Wall -O3; TEST_RUNNER_ENABLE',
    entry_commit=os.environ.get('PA20_ENTRY_COMMIT','2e31ab57e7ba3371ca5c59057ede55b435d4a666'),
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

sources = {'startup':'int main(){}'}
for n in (800,3200):
    sources['qualified-specializations-'+str(n)] = 'template<int N>struct Owner{template<int K>struct Item{int n;};};int main(){int sum=0;' + ''.join(f'{{Owner<{i}>::Item<0> x={{3}};sum+=x.n;}}' for i in range(n)) + f'return sum!={3*n};}}'
trace = (ROOT/'student.tests/pa20/trace100.cpp').read_text().split('int main()')[0]
for n in (800,3200):
    sources['new-retained-specializations-'+str(n)] = trace + 'int main(){' + ''.join(f'use<char[{i+1}]>();' for i in range(n)) + f'return calls!={n};}}'
for name in ('calls','memory','floating'):
    sources['runtime-'+name] = history['workloads']['runtime-'+name]['source']
sources['new-runtime-retained'] = trace + 'int main(){volatile int n=12000000;for(int i=0;i<n;++i)use<int>();return calls!=n;}'
explicit = trace.replace('{ item<select<sizeof(value)>::category<0>::result> value; }','{ item<(static_cast<int>(select<sizeof(value)>::category)<0)>::result > value; }')
sources['runtime-explicit-relation'] = explicit + 'int main(){volatile int n=12000000;for(int i=0;i<n;++i)use<int>();return calls!=n;}'
if os.environ.get('PA20_BENCH_CASES'):
    selected = os.environ['PA20_BENCH_CASES'].split(',')
    sources = {name:sources[name] for name in selected}
for name, source in sources.items():
    src = WORK/(name+'.cpp'); src.write_text(source)
    item = dict(source=source,source_sha256=shared.sha(src),outputs=[],runtime_timing='startup-dominated control' if 'runtime-' not in name else 'live checked loop')
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
