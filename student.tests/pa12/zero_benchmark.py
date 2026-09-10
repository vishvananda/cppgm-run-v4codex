#!/usr/bin/env python3
"""Frozen common A/A+ABBA evidence and typed zero-initialization and member-pointer null paths."""
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
data = dict(protocol='common: warmups, four A/A observations, two ABBA blocks; new: warmup and six observations',
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
boundary = """struct Base { volatile long first; long second; };
union Choice { long first; char bytes[16]; };
struct Owner : Base { Choice choice; Owner():Base(),choice(){} };
int run(int n){Owner owner;if(owner.first||owner.second||owner.choice.first)__builtin_abort();return 2*n+1;}
"""
def members(count):
    return """struct Value { int n; }; typedef int Value::*Data;
struct Pointers { Data pointers[COUNT]; }; struct Owner {Pointers p; Owner():p(){} };
int run(int n){Owner owner;
 unsigned char const* bytes=reinterpret_cast<unsigned char const*>(&owner.p);
 for(unsigned j=0;j<sizeof(Pointers);++j)if(bytes[j]!=255)__builtin_abort();
 return 2*n+1;}
""".replace('COUNT',str(count))
sources = []
for label, body, both, namespaces, count in [
 ('common',common,True,(1000,4000),12000000),
 ('boundary',boundary,True,(1000,4000),12000000),
 ('members8',members(8),False,(100,400),150000),
 ('members9',members(9),False,(100,400),150000),
 ('members1024',members(1024),False,(100,400),2000)]:
    for copies in namespaces:
        text = ''.join(f'namespace N{i}{{{body}}}' for i in range(copies))
        sources.append((f'{label}-{copies}', text+f'int main(){{return N{copies-1}::run(7)!=15;}}', both))
    expected = ((count//1024)*sum(2*i+1 for i in range(1024))+sum(2*i+1 for i in range(count%1024)))&65535
    text = body+f'int main(){{volatile int n={count};int sum=0;for(int i=0;i<n;++i)sum=(sum+run(i&1023))&65535;return sum!={expected};}}'
    sources.append((label+'-runtime',text,both))
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
        equivalence='byte-identical LowIR/native and checked exit 0' if name.startswith('common') else 'validated checked exit 0; boundary A/B values equivalent, new null representation B only',
        compiler=campaign(commands), runtime=campaign(executables))
    output.write_text(json.dumps(data, indent=2)+'\n'); print(name, flush=True)
