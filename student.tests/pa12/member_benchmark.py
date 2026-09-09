#!/usr/bin/env python3
"""Frozen O0 member/delegation checkpoint evidence; A/A followed by ABBA blocks."""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import platform
import statistics
import subprocess
import sys
import time
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('prior', ROOT/'student.tests/pa10/benchmark.py')
prior = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prior)

def main():
    a, b, work, destination = map(Path, sys.argv[1:5])
    work.mkdir(parents=True, exist_ok=True)
    binaries = [a.resolve(), b.resolve()]
    cpu = min(os.sched_getaffinity(0)); os.sched_setaffinity(0, {cpu})
    data = dict(protocol='A/A (four A observations), two ABBA blocks; O0; pinned CPU',
        cpu=cpu, platform=platform.platform(), harness_sha256=prior.sha(__file__),
        binaries=[dict(path=str(x), sha256=prior.sha(x), text_bytes=prior.text_size(x)) for x in binaries],
        backend_sha256=prior.sha(ROOT/'reference-binaries/lowir2native'),
        compile_flags=['--emit-lowir', '-O0'], native_flags=['-O0'],
        text_metric='compiler .text; supplied sectionless ELF executable payload after entry',
        startup=[], workloads={})
    def save(): destination.write_text(json.dumps(data, indent=2)+'\n')
    def observe(cmd):
        start = time.perf_counter_ns(); usage = work/'usage.txt'
        prior.run(['/usr/bin/time', '-f', '%M', '-o', usage, *cmd])
        return dict(wall_s=(time.perf_counter_ns()-start)/1e9, rss_kib=int(usage.read_text()))
    def command(label, source, ir): return [binaries[label], '--emit-lowir', '-O0', '-o', ir, source]
    def campaign(cmds):
        rows=[]
        for ordinal, label in enumerate(prior.ORDER):
            row=observe(cmds[label]); row.update(ordinal=ordinal, binary=label); rows.append(row)
        a_rows=[x['wall_s'] for x in rows[:4]]
        pairs=[]
        for begin in (4,8):
            block=rows[begin:begin+4]
            pairs.append(statistics.mean(x['wall_s'] for x in block if x['binary']==1)/
                         statistics.mean(x['wall_s'] for x in block if x['binary']==0))
        return dict(observations=rows, aa_range_s=[min(a_rows), max(a_rows)], paired_b_over_a=pairs)
    empty=work/'empty.cpp'; empty.write_text('int main(){return 0;}')
    for label in (0,1):
        for _ in range(4):
            row=observe(command(label,empty,work/'empty.lowir')); row['binary']=label; data['startup'].append(row)
    sources=[]
    for count in (1000,4000):
        body='struct A{int x; A(int v):x(v){} int get() const{return x;}}; int run(int n){A a(n);return a.get();}'
        source=''.join(f'namespace N{i}{{{body}}}' for i in range(count))
        sources.append((f'members-{count}', source+f'int main(){{return N{count-1}::run(7)-7;}}', True))
    n=12000000; expected=sum((i*17+3)&1023 for i in range(n%1024))
    expected=(expected+(n//1024)*sum((i*17+3)&1023 for i in range(1024)))&65535
    sources.append(('empty-destructor-runtime', f'''struct A{{int x;A(int n):x(n){{}}~A(){{}}int get(){{return (x*17+3)&1023;}}}};
int main(){{volatile int n={n};int sum=0;for(int i=0;i<n;++i){{A a(i);sum=(sum+a.get())&65535;}}return sum!={expected};}}''', False))
    for name, text, identical in sources:
        source=work/(name+'.cpp'); source.write_text(text)
        irs=[work/(name+f'-{j}.lowir') for j in (0,1)]
        executables=[work/(name+f'-{j}') for j in (0,1)]
        entry=dict(source_path=str(source), source_sha256=prior.sha(source), outputs=[])
        for label in (0,1):
            prior.run([*command(label,source,irs[label]), '--validate-lowir'])
            prior.run([ROOT/'dev/lowir2native-ref', '-O0', '-o', executables[label], irs[label]])
            prior.run([executables[label]])
            entry['outputs'].append(dict(lowir_sha256=prior.sha(irs[label]), lowir_bytes=irs[label].stat().st_size,
                executable_sha256=prior.sha(executables[label]), text_bytes=prior.text_size(executables[label]), checked_exit=0))
        if identical: assert irs[0].read_bytes()==irs[1].read_bytes(), name+': common output changed'
        entry['equivalence']='identical LowIR and native exit 0' if identical else 'same volatile loop and checked checksum; only empty destructor calls omitted'
        entry['compiler']=campaign([command(j,source,irs[j]) for j in (0,1)])
        entry['runtime']=campaign([[exe] for exe in executables])
        data['workloads'][name]=entry; save(); print(name, entry['compiler']['paired_b_over_a'], entry['runtime']['paired_b_over_a'], flush=True)
    save()
if __name__=='__main__': main()
