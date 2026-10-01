#!/usr/bin/env python3
"""Template-definition/assertion demand: frozen A/A and six ABBA blocks."""
import hashlib
import json
import os
import pathlib
import statistics
import subprocess
import sys
import time

out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
binaries = dict(zip('AB', (pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity = ['taskset', '-c', os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def run(args):
    p = subprocess.run(list(map(str, args)), capture_output=True, text=True, timeout=60)
    assert p.returncode == 0, (args, p.returncode, p.stdout, p.stderr)
    return p


record = dict(binaries={k: dict(path=str(v), sha256=sha(v)) for k,v in binaries.items()}, flags=['-O0', '-c', '--stats'],
              affinity=affinity, inputs={}, runs=[], launchers=[], images={}, summary={})


def save():
    (out / 'performance.json').write_text(json.dumps(record, indent=2) + '\n')


for i in range(8):
    start = time.perf_counter()
    run(['/usr/bin/time', '-f', '%M', '-o', out / 'rss', *affinity, 'true'])
    record['launchers'].append(dict(wall_s=time.perf_counter() - start,
                                   peak_rss_kib=int((out / 'rss').read_text())))

iterations = 3000000
state, total = 7, 0
for i in range(iterations):
    state = (state * 17 + (i & 127)) % 1009
    total = (total + state) % 65521

prefix = '''template<int N> struct character { int code; character(int v):code(v){} operator int()const{return code;} };
template<class T> struct traits;
template<class T> struct probe {typedef typename traits<T>::int_type int_type;
static T from(int_type n){return traits<T>::from(n);} static int_type to(T n){return traits<T>::to(n);}
static int dormant(){return T::missing;}};
template<class T> struct traits {typedef int int_type; static T from(int n){return T(n);} static int to(T n){return n;}};
template<bool B> struct flag {static const bool value=B;};
template<class T> struct source_property:flag<false>{};
template<class T> struct invert:flag<!T::value>{};
'''
runtime = f'''int main(int argc,char**argv){{int state=argc>1?argv[1][0]-'0':1,total=0;
for(int i=0;i<{iterations};++i){{auto c=probe<character<0>>::from((state*17+(i&127))%1009);
state=probe<character<0>>::to(c);total=(total+state)%65521;}}
return total=={total}&&state=={state}?0:1;}}
'''
for n in [600, 1200, 2400]:
    name = 'demand' + str(n)
    src, obj, exe = [out / (name + suffix) for suffix in ['.cpp', '.o', '']]
    source = prefix + ''.join(
        f'static_assert(__is_same(probe<character<{i}>>::int_type,int),"member");'
        f'static_assert(invert<source_property<character<{i}>>>::value,"source false");\n'
        for i in range(n)) + runtime
    src.write_text(source)
    record['inputs'][name] = dict(source=source, sha256=sha(src), N=n,
                                 expected=[state, total], seed=7, iterations=iterations)
    executables = {}
    images = {}
    for label, compiler in binaries.items():
        obj = out / (name + label + '.o')
        exe = out / (name + label)
        run([compiler, *record['flags'], src, '-o', obj])
        run(['g++', obj, '-o', exe])
        run([exe, '7'])
        executables[label] = exe
        syms = run(['nm', '-C', obj]).stdout
        assert 'dormant' not in syms
        size = sum(int(l.split()[1]) for l in run(['size', '-A', exe]).stdout.splitlines()
                   if l.split() and l.split()[0].startswith('.text'))
        images[label] = dict(object_sha256=sha(obj), executable_sha256=sha(exe), text_bytes=size)
    assert images['A'] == images['B']
    record['images'][name] = images
    host = out / 'host'
    run(['g++', '-std=c++11', '-O0', src, '-o', host])
    run([host, '7'])
    for mode in ['compile', 'runtime']:
        for block, order in enumerate(['AAAA'] + ['ABBA'] * 6):
            for label in order:
                args = [binaries[label], *record['flags'], src, '-o', out / 'measure.o'] if mode == 'compile' else [executables[label], '7']
                start = time.perf_counter()
                p = run(['/usr/bin/time', '-f', '%M', '-o', out / 'rss', *affinity, *args])
                record['runs'].append(dict(workload=name, mode=mode, block=block, label=label,
                                          wall_s=time.perf_counter() - start,
                                          peak_rss_kib=int((out / 'rss').read_text()), status=p.returncode,
                                          counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]))
                save()
        rows = [r for r in record['runs'] if r['workload'] == name and r['mode'] == mode]
        summary = {}
        for label in 'AB':
            samples = [r for r in rows if r['block'] and r['label'] == label]
            summary[label] = dict(median_s=statistics.median(r['wall_s'] for r in samples),
                                 range_s=[min(r['wall_s'] for r in samples), max(r['wall_s'] for r in samples)],
                                 peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
        ratios = [statistics.mean(r['wall_s'] for r in rows if r['block'] == b and r['label'] == 'B') /
                  statistics.mean(r['wall_s'] for r in rows if r['block'] == b and r['label'] == 'A') for b in range(1,7)]
        aa = [r['wall_s'] for r in rows if not r['block']]
        summary.update(paired_ratios=ratios, paired_ratio_median=statistics.median(ratios),
                       paired_ratio_range=[min(ratios), max(ratios)], AA_range_s=[min(aa), max(aa)])
        record['summary'].setdefault(name, {})[mode] = summary
    print(name, json.dumps(record['summary'][name]), flush=True)
assert all(sha(v) == record['binaries'][k]['sha256'] for k,v in binaries.items())
save()
