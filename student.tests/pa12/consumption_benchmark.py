#!/usr/bin/env python3
"""Frozen common A/A+ABBA evidence and terminal branch consumption and scalar transfer proofs."""
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
    binaries=[dict(path=str(x), sha256=prior.sha(x), text_bytes=prior.text_size(x)) for x in binaries],
    backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
    compile_flags=['--emit-lowir', '-O0'], native_flags=['-O0'],
    text_metric='compiler .text; supplied sectionless ELF executable payload after entry', workloads={})

def observe(command):
    usage = work/'usage.txt'; start = time.perf_counter_ns()
    prior.run(['/usr/bin/time', '-f', '%M', '-o', usage, *command])
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(usage.read_text()))

def campaign(commands):
    warmups = [observe(x) for x in commands]
    order = prior.ORDER if len(commands) == 2 else [0]*6
    rows = []
    for label in order:
        row = observe(commands[label]); row['binary'] = label if len(commands) == 2 else 1; rows.append(row)
    result = dict(warmups=warmups, observations=rows)
    if len(commands) == 2:
        result['aa_range_s'] = [min(x['wall_s'] for x in rows[:4]), max(x['wall_s'] for x in rows[:4])]
        result['paired_b_over_a'] = [statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['binary'] == 1)/
            statistics.mean(x['wall_s'] for x in rows[k:k+4] if x['binary'] == 0) for k in (4, 8)]
    return result

common = '''struct S{int a,b;S(int n):a(n),b(n+1){} int get()const{return a+b;}};
S make(int n){return S(n);} int read(S s){return s.get();}
int run(int n){S s=make(n);return read(s);}
'''
terminal = """int live,shells;
struct Item {int n;Item(int x):n(x){++live;}Item(Item const& x):n(x.n){++live;}
 Item(Item&& x):n(x.n){++live;x.n=-1;}~Item(){--live;}};
struct Shell {Item value;Shell(int n):value(n){++shells;}~Shell(){--shells;}};
Item choose(int n){Item local(n);return n&1?local:Shell(n).value;}
int run(int n){Item value=choose(n);if(live!=1||shells)__builtin_abort();return value.n*2+1;}
"""
checked = terminal.replace('int live,shells;', 'int live,shells;void observe(){++shells;--shells;}').replace('Item(Item const& x):n(x.n){++live;}', 'Item(Item const& x):n(x.n){observe();++live;}')
loop = """int done;struct Marker{~Marker(){++done;}};
int compute(int n){Marker a,b;for(int i=0;i<2;++i){if(n&1)return n*2+1;}return n*2+1;}
int run(int n){done=0;int value=compute(n);if(done!=2)__builtin_abort();return value;}
"""
sources = []
for label, body, namespaces in [('common',common,(1000,4000)),('terminal',terminal,(100,400)),
                                 ('checked',checked,(100,400)),('loop',loop,(100,400))]:
    for copies in namespaces:
        text = ''.join(f'namespace N{i}{{{body}}}' for i in range(copies))
        sources.append((f'{label}-{copies}', text+f'int main(){{return N{copies-1}::run(7)!=15;}}', True))
    count=12000000
    expected = ((count//1024)*sum(2*i+1 for i in range(1024))+sum(2*i+1 for i in range(count%1024)))&65535
    text = body+f'int main(){{volatile int n={count};int sum=0;for(int i=0;i<n;++i)sum=(sum+run(i&1023))&65535;return sum!={expected};}}'
    sources.append((label+'-runtime',text,True))
for name, contents, both in sources:
    source = work/(name+'.cpp'); source.write_text(contents)
    labels = (0, 1) if both else (1,)
    commands, executables, irs, records = [], [], [], []
    for label in labels:
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
        equivalence='byte-identical LowIR/native and checked exit 0' if name.startswith('common') else 'both validate and return checked exit 0; equivalent live counts and dynamic values',
        compiler=campaign(commands), runtime=campaign(executables))
    output.write_text(json.dumps(data, indent=2)+'\n'); print(name, flush=True)
