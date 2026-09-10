#!/usr/bin/env python3
"""Frozen full-stage O0 audit: common A/A+ABBA, corrected paths absolute only."""
from pathlib import Path
import json
import os
import platform
import statistics
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'student.tests/pa10'))
import benchmark as shared

a, b, work, output = map(Path, sys.argv[1:5])
binaries = [a.resolve(), b.resolve()]
work.mkdir(parents=True, exist_ok=True)
cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
data = dict(protocol='warmups; common: four A/A rows then two ABBA blocks; corrected-only: six rows',
    cpu=cpu, platform=platform.platform(), compiler_flags=['--emit-lowir', '-O0'], native_flags=['-O0'],
    build_flags='g++ -std=gnu++11 -Wall -O3; course TEST_RUNNER_ENABLE',
    commits=['18ef3757', shared.run(['git', 'rev-parse', 'HEAD']).stdout.strip()],
    binaries=[dict(path=str(x), sha256=shared.sha(x), text_bytes=shared.text_size(x)) for x in binaries],
    harness_sha256=shared.sha(__file__), shared_harness_sha256=shared.sha(ROOT/'student.tests/pa10/benchmark.py'),
    backend_sha256=shared.sha(ROOT/'reference-binaries/lowir2native'),
    text_metric='compiler .text; sectionless native executable payload after ELF entry, including support/data',
    acceptance='PA12 O0 correctness and bounded work; no added numeric exit gates or speed claim for incorrect A',
    workloads={})

def observe(command):
    usage = work/'usage.txt'; start = time.perf_counter_ns()
    shared.run(['/usr/bin/time', '-f', '%M', '-o', usage, *command])
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(usage.read_text()), checked_exit=0)

def campaign(commands):
    warmups = [dict(binary=label, **observe(cmd)) for label, cmd in commands.items()]
    order = shared.ORDER if len(commands) == 2 else [1]*6
    rows = [dict(binary=label, **observe(commands[label])) for label in order]
    result = dict(warmups=warmups, observations=rows)
    if len(commands) == 2:
        result['aa_range_s'] = [min(r['wall_s'] for r in rows[:4]), max(r['wall_s'] for r in rows[:4])]
        result['paired_b_over_a'] = [statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 1)/
            statistics.mean(r['wall_s'] for r in rows[k:k+4] if r['binary'] == 0) for k in (4, 8)]
    return result

common = '''struct S{int a,b;S(int n):a(n),b(n+1){}int get()const{return a+b;}};
S make(int n){return S(n);}int take(S s){return s.get();}int run(int n){return take(make(n));}
'''
reference = '''struct S{int n;int* live;S(int x,int* p):n(x),live(p){++*live;}~S(){--*live;}};
int run(int n){int live=0;int answer=0;{
const int& value=n&1?S(n*2+1,&live).n:S(n*2+1,&live).n;
if(live!=1)__builtin_abort();answer=value;}
if(live)__builtin_abort();return answer;}
'''
volatile = '''struct S{int n;};int run(int n){volatile S&& value={n*2+1};return value.n;}
'''
heap = '''struct S{int n;};struct P{int S::*p;};
int run(int n){int count=(n&3)+1;P* values=new P[count]();
for(int i=0;i<count;++i){unsigned char* p=reinterpret_cast<unsigned char*>(&values[i].p);
for(unsigned j=0;j<sizeof(values[i].p);++j)if(p[j]!=255)__builtin_abort();}
delete[] values;return n*2+1;}
'''

corpus = [(name, source, mode, True, False) for name, source, mode, scale in shared.workloads()]
for label, body, counts, correct_a in [('class', common, (1000,4000), True),
        ('reference', reference, (100,400), False), ('volatile-list', volatile, (1000,4000), False),
        ('heap-zero', heap, (100,400), False)]:
    for count in counts:
        source = ''.join(f'namespace N{i}{{{body}}}' for i in range(count))
        corpus.append((f'{label}-{count}', source, '--emit-lowir', correct_a, False))
    count = 40000 if label == 'heap-zero' else 12000000
    expected = ((count//1024)*sum(2*i+1 for i in range(1024))+
                sum(2*i+1 for i in range(count%1024))) & 65535
    source = body+f'int main(){{volatile int n={count};int sum=0;for(int i=0;i<n;++i)sum=(sum+run(i&1023))&65535;return sum!={expected};}}'
    corpus.append((label+'-runtime', source, '--emit-lowir', correct_a, True))
for name, source in shared.runtimes(16):
    corpus.append((name+'-runtime', source, '--emit-lowir', True, True))
corpus.insert(0, ('startup', 'int main(){return 0;}', '--emit-lowir', True, True))

for name, contents, mode, correct_a, executable in corpus:
    source = work/(name+'.cpp'); source.write_text(contents)
    labels = (0,1) if correct_a else (1,)
    flags = ['-O0'] if mode == '--emit-lowir' else []
    commands, runtimes, records = {}, {}, []
    for label in labels:
        ir = work/(name+f'-{label}.out')
        command = [binaries[label], mode, *flags, '-o', ir, source]
        stats = shared.run([*command, '--stats', *(['--validate-lowir'] if mode == '--emit-lowir' else [])])
        record = dict(binary=label, path=str(ir), sha256=shared.sha(ir), bytes=ir.stat().st_size,
            telemetry=[json.loads(line) for line in stats.stderr.splitlines()])
        if executable:
            exe = work/(name+f'-{label}')
            shared.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); shared.run([exe])
            record['native'] = dict(path=str(exe), sha256=shared.sha(exe),
                text_bytes=shared.text_size(exe), checked_exit=0)
            runtimes[label] = [exe]
        records.append(record); commands[label] = command
    if correct_a:
        assert records[0]['sha256'] == records[1]['sha256'], (name, 'common output changed')
        if executable: assert records[0]['native']['sha256'] == records[1]['native']['sha256'], name
    entry = dict(source_path=str(source), source_sha256=shared.sha(source), mode=mode, outputs=records,
        equivalence='byte-identical correct A/B output' if correct_a else 'corrected B only; A violates the audited semantics',
        compiler=campaign(commands))
    if runtimes: entry['runtime'] = campaign(runtimes)
    data['workloads'][name] = entry
    output.write_text(json.dumps(data, indent=2)+'\n'); print(name, flush=True)
assert [shared.sha(x) for x in binaries] == [x['sha256'] for x in data['binaries']]
