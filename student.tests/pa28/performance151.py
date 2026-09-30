#!/usr/bin/env python3
"""New PA28 ABI workload: standalone costs, not a failing-A speed comparison."""
import hashlib, json, os, pathlib, statistics, subprocess, sys, time
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True,exist_ok=True)
binaries = dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity = ['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    return subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
source = out/'naming.cpp'
source.write_text('''
template<int N> struct __attribute__((abi_tag("alpha","beta"))) Cell {
  long value;
  Cell(long v):value(v){}
  long read() const { return value+N; }
};
template<int N> long instance(long v){Cell<N> c(v);return c.read();}
template<class T> __attribute__((const)) __decay(T) pass(T* p){return *p;}
long demanded(long v){long sum=0;
''' + ''.join('sum+=instance<%d>(v);\n'%n for n in range(600)) + '''
return sum;}
int main(int argc,char**) {
  if(demanded(argc)!=180300)return 2;
  int sum=0;
  for(int i=0;i<argc*12000000;++i){int value=i&127;sum=(sum+pass(&value))%1009;}
  return sum==''' + str(sum(i&127 for i in range(12000000))%1009) + '''?0:1;
}
''')
result = dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in binaries.items()},
              source_sha256=sha(source),flags=['-O0','-c','--stats'],
              affinity=affinity,runs=[],summary={})
p = run([binaries['A'],'-O0','-c',source,'-o',out/'A.o'])
assert p.returncode != 0
result['unsupported_A'] = dict(status=p.returncode,stderr=p.stderr.decode())
obj,exe = out/'B.o',out/'B'
assert run([binaries['B'],'-O0','-c',source,'-o',obj]).returncode == 0
assert run(['g++',obj,'-o',exe]).returncode == 0
assert run([exe]).returncode == 0
symbols = run(['nm',obj]).stdout.decode()
assert '_ZN4CellB5alphaB4betaILi17EEC1El' in symbols
assert '_Z4passIiEu7__decayIT_EPS0_' in symbols
def text_size(path):
    p = run(['size','-A',path]); assert p.returncode == 0
    return sum(int(s.split()[1]) for s in p.stdout.decode().splitlines()
               if s.split() and s.split()[0].startswith('.text'))
result['image'] = dict(object_sha256=sha(obj),executable_sha256=sha(exe),
                       object_bytes=obj.stat().st_size,object_text_bytes=text_size(obj),
                       executable_bytes=exe.stat().st_size,executable_text_bytes=text_size(exe))
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for mode in ('compile','runtime'):
    for i in range(12):
        args = [binaries['B'],'-O0','-c','--stats',source,'-o',out/'measure.o'] if mode=='compile' else [exe]
        start = time.perf_counter()
        p = run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
        elapsed = time.perf_counter()-start
        assert p.returncode == 0,(mode,p.stderr.decode())
        result['runs'].append(dict(mode=mode,sample=i,wall_s=elapsed,
            peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,
            phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]))
        save()
    rows = [r for r in result['runs'] if r['mode']==mode]
    result['summary'][mode] = dict(median_s=statistics.median(r['wall_s'] for r in rows),
        range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],
        peak_rss_kib=max(r['peak_rss_kib'] for r in rows))
    save()
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())
print(json.dumps(result['summary']),flush=True)
