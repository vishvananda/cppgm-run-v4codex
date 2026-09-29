#!/usr/bin/env python3
"""Replay fixed whole-stage A/B inputs. Run A B WORK OUT (fresh paths)."""
from pathlib import Path
import json, os, platform, statistics, subprocess, sys, time
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'student.tests/pa10'))
import benchmark as shared

A, B, WORK, OUT = [Path(p).resolve() for p in sys.argv[1:]]
assert not OUT.exists(), 'Preserve prior measurements'
WORK.mkdir(parents=True, exist_ok=True)
cpu = max(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
base = json.loads((ROOT/'student.tests/pa19/performance91-initial.json').read_text())
last = json.loads((ROOT/'student.tests/pa19/performance92.json').read_text())
assert shared.sha(A) == base['binaries'][0]['sha256']
assert shared.sha(B) == last['binaries'][1]['sha256']
backend = ROOT/'reference-binaries/lowir2native'
assert shared.sha(backend) == last['backend']['sha256']
result = dict(protocol='one warmup each, four A/A observations, four ABBA blocks; six B observations for new behavior',
    cpu=cpu, platform=platform.platform(), commits=[base['commits'][0], shared.run(['git','rev-parse','HEAD']).stdout.strip()],
    flags=['--emit-lowir','-O0'], build_flags=last['build_flags'], backend=last['backend'],
    harness_source=Path(__file__).read_text(), harness_sha256=shared.sha(__file__),
    shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
    binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in (A,B)],
    text_metric='compiler .text; sectionless supplied ELF payload proxy including static data',
    workloads={})
def save(): OUT.write_text(json.dumps(result, indent=2)+'\n')
def observe(command):
    usage = WORK/'usage.txt'
    start = time.perf_counter_ns()
    shared.run(['/usr/bin/time','-f','%M %U %S %c %w','-o',usage,*command])
    rss, user, system, involuntary, voluntary = usage.read_text().split()
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9,rss_kib=int(rss),
        user_s=float(user),system_s=float(system),involuntary=int(involuntary),
        voluntary=int(voluntary),checked_exit=0)
def measure(commands):
    both = len(commands) == 2
    warmups = [dict(binary=i,**observe(c)) for i,c in commands.items()]
    order = [0]*4+[0,1,1,0]*4 if both else [1]*6
    rows = [dict(binary=i,**observe(commands[i])) for i in order]
    data = dict(warmups=warmups,observations=rows)
    if both:
        aa = [r['wall_s'] for r in rows[:4]]
        data['aa_range_s'] = [min(aa), max(aa)]
        data['paired_b_over_a'] = [
            statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==1) /
            statistics.mean(r['wall_s'] for r in rows[j:j+4] if r['binary']==0)
            for j in range(4,len(rows),4)]
    for i in commands:
        selected = [r for r in rows[4 if both else 0:] if r['binary']==i]
        data[str(i)] = dict(median_wall_s=statistics.median(r['wall_s'] for r in selected),
            wall_range_s=[min(r['wall_s'] for r in selected), max(r['wall_s'] for r in selected)],
            peak_rss_kib=max(r['rss_kib'] for r in selected))
    return data

empty = WORK/'empty.cpp'
empty.write_text('int main(){}')
result['startup'] = measure({i:[cc,'--emit-lowir','-O0','-o',WORK/f'empty-{i}.lowir',empty] for i,cc in enumerate((A,B))})
save()
inputs = [(base, 'ordinary-ordering-'+str(n)) for n in (2400,9600)]
inputs += [(last, name+'-'+str(n)) for name in ('namespace-variable','member-variable') for n in (2400,9600)]
inputs += [(last, 'runtime-'+name) for name in ('calls','memory','floating')]
for evidence, name in inputs:
    old = evidence['workloads'][name]
    src = WORK/(name+'.cpp')
    src.write_text(old['source'])
    assert shared.sha(src) == old['source_sha256']
    item = dict(source=old['source'], source_sha256=shared.sha(src), outputs=[])
    result['workloads'][name] = item
    commands, executables = {}, {}
    for i, cc in enumerate((A,B)):
        ir, exe = WORK/(name+f'-{i}.lowir'), WORK/(name+f'-{i}')
        command = [cc,'--emit-lowir','-O0','-o',ir,src]
        check = subprocess.run([str(x) for x in [*command,'--stats','--validate-lowir']],capture_output=True,text=True,timeout=300)
        if check.returncode:
            assert i==0 and name.startswith('member-variable'), (name,check.stderr)
            item['entry_rejection'] = dict(exit=check.returncode,diagnostic=check.stderr)
            continue
        shared.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
        shared.run([exe])
        item['outputs'].append(dict(binary=i,sha256=shared.sha(ir),bytes=ir.stat().st_size,
            telemetry=[json.loads(line) for line in check.stderr.splitlines()],
            native=dict(sha256=shared.sha(exe),payload_bytes=shared.text_size(exe),file_bytes=exe.stat().st_size,checked_exit=0)))
        commands[i], executables[i] = command, [exe]
    assert 1 in commands
    if len(commands)==2:
        assert item['outputs'][0]['sha256'] == item['outputs'][1]['sha256']
        assert item['outputs'][0]['native'] == item['outputs'][1]['native']
        item['comparison'] = 'exact'
    else: item['comparison'] = 'new-behavior'
    save()
    item['compiler'] = measure(commands)
    item['runtime'] = measure(executables)
    for output in item['outputs']:
        assert shared.sha(WORK/(name+f"-{output['binary']}.lowir")) == output['sha256']
    save()
    print(name, 'complete', flush=True)
assert [shared.sha(p) for p in (A,B)] == [b['sha256'] for b in result['binaries']]
