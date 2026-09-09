#!/usr/bin/env python3
"""PA12 prefix-vs-fieldwise evidence, with equivalent typed transfers on both sides."""
from pathlib import Path
import importlib.util
import json
import os
import statistics
import sys
import time
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec); spec.loader.exec_module(prior)
a, b, work, output = map(Path, sys.argv[1:5])
binaries = [a.resolve(), b.resolve()]; work.mkdir(parents=True, exist_ok=True)
cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
data = dict(protocol='four A/A observations, two ABBA blocks; fieldwise A versus prefix B', cpu=cpu,
    binaries=[dict(path=str(p), sha256=prior.sha(p), text_bytes=prior.text_size(p)) for p in binaries],
    harness_sha256=prior.sha(__file__), backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
    compile_flags=['--emit-lowir','-O0'], native_flags=['-O0'], workloads={})
def observe(cmd):
    usage=work/'usage.txt'; start=time.perf_counter_ns()
    prior.run(['/usr/bin/time','-f','%M','-o',usage,*cmd])
    return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(usage.read_text()))
def campaign(commands):
    rows=[]
    for ordinal,label in enumerate(prior.ORDER):
        row=observe(commands[label]); row.update(ordinal=ordinal,binary=label); rows.append(row)
    pairs=[]
    for start in (4,8):
        block=rows[start:start+4]
        pairs.append(statistics.mean(x['wall_s'] for x in block if x['binary']==1)/statistics.mean(x['wall_s'] for x in block if x['binary']==0))
    return dict(observations=rows, paired_b_over_a=pairs, aa_range_s=[min(x['wall_s'] for x in rows[:4]),max(x['wall_s'] for x in rows[:4])])
body='''struct Tail { unsigned value; Tail(unsigned v):value(v){} Tail(const Tail& t):value(t.value){} Tail& operator=(const Tail& t){value=t.value;return *this;} };
struct Packet { unsigned a,b,c,d,e,f,g,h; Tail tail; Packet(unsigned v):a(v),b(v+1),c(v+2),d(v+3),e(v+4),f(v+5),g(v+6),h(v+7),tail(v+8){} };
int run(){Packet a(7);Packet b(a);b=a;return b.a+b.h+b.tail.value;}'''
sources=[]
for count in (1000,4000):
    source=''.join(f'namespace N{i}{{{body}}}' for i in range(count))+f'int main(){{return N{count-1}::run()-36;}}'
    sources.append((f'transfers-{count}',source))
count=4000000
expected=((count//1024)*sum(i+29 for i in range(1024))+sum(i+29 for i in range(count%1024)))&65535
sources.append(('prefix-runtime',body+f'''int main(){{Packet source(7);Packet dest(0);volatile int n={count};int sum=0;for(int i=0;i<n;++i){{source.a=i&1023;dest=source;sum=(sum+dest.a+dest.h+dest.tail.value)&65535;}}return sum!={expected};}}'''))
if '--prefix-probes' in sys.argv[5:]:
    sources=[]
    for fields in (2,4,8,16):
        declarations=','.join(f'f{i}' for i in range(fields))
        initializers=','.join(f'f{i}(v+{i})' for i in range(fields))
        prefix=body[:body.index('struct Packet')]
        packet=f'struct Packet{{unsigned {declarations};Tail tail;Packet(unsigned v):{initializers},tail(v+{fields}){{}}}};'
        constant=2*fields+13
        expected=((count//1024)*sum(i+constant for i in range(1024))+sum(i+constant for i in range(count%1024)))&65535
        source=prefix+packet+f'int main(){{Packet source(7);Packet dest(0);volatile int n={count};int sum=0;for(int i=0;i<n;++i){{source.f0=i&1023;dest=source;sum=(sum+dest.f0+dest.f{fields-1}+dest.tail.value)&65535;}}return sum!={expected};}}'
        sources.append((f'prefix-{fields*4}-bytes',source))
if '--storage-prefix' in sys.argv[5:]:
    prefix=body[:body.index('struct Packet')]
    packet='union Payload{unsigned long long bits;unsigned word;};struct Packet{unsigned a;Payload payload;Tail tail;Packet(unsigned v):a(v),payload(),tail(v+8){payload.bits=v+7;}};'
    expected=((count//1024)*sum(i+29 for i in range(1024))+sum(i+29 for i in range(count%1024)))&65535
    source=prefix+packet+f'int main(){{Packet source(7);Packet dest(0);volatile int n={count};int sum=0;for(int i=0;i<n;++i){{source.a=i&1023;dest=source;sum=(sum+dest.a+static_cast<unsigned>(dest.payload.bits)+dest.tail.value)&65535;}}return sum!={expected};}}'
    sources=[('storage-prefix-runtime',source)]
for name,contents in sources:
    source=work/(name+'.cpp');source.write_text(contents)
    irs=[work/(name+f'-{j}.lowir') for j in (0,1)]; exes=[work/(name+f'-{j}') for j in (0,1)]
    commands=[[binary,'--emit-lowir','-O0','-o',ir,source] for binary,ir in zip(binaries,irs)]
    entry=dict(source_path=str(source),source_sha256=prior.sha(source),outputs=[],
        equivalence='fieldwise scalar transfers versus the same disjoint leading scalar prefix; identical selected Tail functions and checked checksum')
    for label in (0,1):
        stats=prior.run([*commands[label],'--validate-lowir','--stats'])
        prior.run([ROOT/'dev/lowir2native-ref','-O0','-o',exes[label],irs[label]]);prior.run([exes[label]])
        entry['outputs'].append(dict(lowir_sha256=prior.sha(irs[label]),lowir_bytes=irs[label].stat().st_size,
            executable_sha256=prior.sha(exes[label]),text_bytes=prior.text_size(exes[label]),checked_exit=0,
            telemetry=[json.loads(line) for line in stats.stderr.splitlines()]))
    entry['compiler']=campaign(commands);entry['runtime']=campaign([[exe] for exe in exes])
    data['workloads'][name]=entry;output.write_text(json.dumps(data,indent=2)+'\n')
    print(name,entry['compiler']['paired_b_over_a'],entry['runtime']['paired_b_over_a'],flush=True)
