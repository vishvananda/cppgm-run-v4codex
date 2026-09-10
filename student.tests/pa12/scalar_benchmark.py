#!/usr/bin/env python3
"""Frozen A/A+ABBA evidence for scalar initialization and branch cleanup."""
from pathlib import Path
import importlib.util
import json
import os
import platform
import statistics
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec); spec.loader.exec_module(prior)
a, b, work, output = map(Path, sys.argv[1:5])
binaries = [a.resolve(), b.resolve()]; work.mkdir(parents=True, exist_ok=True)
cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
data = dict(protocol='all workloads: warmups, four A/A observations and two ABBA blocks',
    cpu=cpu, platform=platform.platform(), harness_sha256=prior.sha(__file__),
    shared_harness_sha256=prior.sha(ROOT/'student.tests/pa10/benchmark.py'),
    binaries=[dict(path=str(x), sha256=prior.sha(x), text_bytes=prior.text_size(x)) for x in binaries],
    backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
    compile_flags=['--emit-lowir', '-O0'], native_flags=['-O0'],
    diagnostic_budgets=dict(compiler_text_growth_bytes=8192, common_compile_median_ratio=1.05),
    text_metric='compiler .text; supplied sectionless ELF executable payload after entry', workloads={})

def observe(command):
    usage = work/'usage.txt'; start = time.perf_counter_ns()
    prior.run(['/usr/bin/time', '-f', '%M', '-o', usage, *command])
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(usage.read_text()))

def campaign(commands):
    warmups = [observe(x) for x in commands]
    rows = []
    for label in prior.ORDER:
        row = observe(commands[label]); row['binary'] = label; rows.append(row)
    return dict(warmups=warmups, observations=rows,
        aa_range_s=[min(x['wall_s'] for x in rows[:4]), max(x['wall_s'] for x in rows[:4])],
        paired_b_over_a=[statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['binary'] == 1)/
            statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['binary'] == 0) for k in (4, 8)])

common = '''struct S{int a,b;S(int n):a(n),b(n+1){} int get()const{return a+b;}};
S make(int n){return S(n);} int read(S s){return s.get();}
int run(int n){S s=make(n);return read(s);}
'''
prefix = '''struct Pair{long a,b;Pair(int n):a(n),b(n+1){}};
Pair make(int n){return Pair(n);}
struct Guard{long* events;Guard(long* p):events(p){++*events;}~Guard(){++*events;}};
long read(Pair const& p,Guard const&){return p.a+p.b;}
'''
dynamic = prefix+'''int run(int n){long events=0;bool choose=n&1;
long result=choose?read(make(n),Guard(&events)):read(make(n+1),Guard(&events));
if(events!=2)__builtin_abort();return result;}
'''
constant = dynamic.replace('bool choose=n&1;', 'bool choose=true;')
mutated = dynamic.replace('int run(int n)', 'void set(bool& out,int n){out=n&1;} int run(int n)').replace(
    'bool choose=n&1;', 'bool choose=true;set(choose,n);')
observed = '''struct Pair{long a,b;Pair(int n):a(n),b(n+1){}};
Pair make(int n){return Pair(n);}
struct Guard{long* events;long* out;long expected;
Guard(long* p,long* q,long n):events(p),out(q),expected(n){++*events;}
~Guard(){if(*out!=expected)__builtin_abort();++*events;}};
long read(Pair const& p,Guard const&){return p.a+p.b;}
int run(int n){long events=0;bool choose=n&1;
long result=choose?read(make(n),Guard(&events,&result,n*2+1)):
 read(make(n+1),Guard(&events,&result,n*2+3));
if(events!=2)__builtin_abort();return result;}
'''
sources = []
for label, body, scales in [('common',common,(1000,4000)),('dynamic',dynamic,(100,400)),
        ('constant',constant,(100,400)),('mutated',mutated,(100,400)),('observed',observed,(100,400))]:
    for copies in scales:
        contents = ''.join(f'namespace N{i}{{{body}}}' for i in range(copies))
        sources.append((f'{label}-{copies}',contents+f'int main(){{return N{copies-1}::run(7)!=15;}}'))
    count = 12000000
    cycle = [2*i+1+(2 if label in ('dynamic','mutated','observed') and not i&1 else 0) for i in range(1024)]
    expected = ((count//1024)*sum(cycle)+sum(cycle[:count%1024]))&65535
    contents = body+f'int main(){{volatile int n={count};int sum=0;for(int i=0;i<n;++i)sum=(sum+run(i&1023))&65535;return sum!={expected};}}'
    sources.append((label+'-runtime',contents))
for name, contents in sources:
    source = work/(name+'.cpp'); source.write_text(contents)
    commands, executables, irs, records = [], [], [], []
    for label in (0,1):
        ir = work/(name+f'-{label}.lowir'); exe = work/(name+f'-{label}')
        command = [binaries[label], '--emit-lowir', '-O0', '-o', ir, source]
        stats = prior.run([*command, '--validate-lowir', '--stats'])
        prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', exe, ir]); prior.run([exe])
        records.append(dict(binary=label, lowir_sha256=prior.sha(ir), lowir_bytes=ir.stat().st_size,
            executable_sha256=prior.sha(exe), text_bytes=prior.text_size(exe), checked_exit=0,
            telemetry=[json.loads(line) for line in stats.stderr.splitlines()]))
        commands.append(command); executables.append([exe]); irs.append(ir)
    if name.startswith('common'):
        assert irs[0].read_bytes() == irs[1].read_bytes(), name+': common LowIR changed'
        assert records[0]['executable_sha256'] == records[1]['executable_sha256'], name+': native output changed'
    data['workloads'][name] = dict(source_path=str(source), source_sha256=prior.sha(source), outputs=records,
        equivalence='byte-identical LowIR/native and checked exit 0' if name.startswith('common') else 'both validate and return checked exit 0; same branch values, destructor effects and observed destinations',
        compiler=campaign(commands), runtime=campaign(executables))
    output.write_text(json.dumps(data, indent=2)+'\n'); print(name, flush=True)
