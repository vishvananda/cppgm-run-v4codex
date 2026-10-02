#!/usr/bin/env python3
"""PA32 O0/O1 A/A + six ABBA blocks on the inherited fixed common inputs.
Usage: common_levels.py OUT FROZEN_COMPILER FROZEN_COMPILER
"""
import hashlib, json, os, pathlib, statistics, subprocess, sys, time
here = pathlib.Path(__file__).resolve().parent
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
binaries = dict(zip('AB', (pathlib.Path(p).resolve() for p in sys.argv[2:4])))
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p = subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
    assert p.returncode == 0, (args,p.returncode,p.stderr.decode(errors='replace'))
    return p
def text_size(p):
    return sum(int(line.split()[1]) for line in run(['size','-A',p]).stdout.decode().splitlines()
               if line.split() and line.split()[0].startswith('.text'))
prefix = 'template<int N> long item(long x){return (x+N)%97;}\nlong demanded(long x){long s=0;\n'
prefix += ''.join('s+=item<%d>(x);\n'%n for n in range(2400))+'return s;}\n'
# Exact inherited fixed inputs, with checksums independently computed by the
# PA26 generator. argc is runtime input and all loops contribute to the checks.
bodies = {
'memory': '''int step(int x,int y){return (x*17+y)%101;}
int main(int argc,char**){if(demanded(argc)!=114372)return 2;
int a[64];for(int i=0;i<64;++i)a[i]=i; int sum=0;
for(int i=0;i<argc*3000000;++i){int k=i&63;a[k]=(a[k]+step(k,i&255))&65535;sum=(sum+a[k])%1009;}
return sum==758?0:1;}''',
'floating': '''double step(double x,int i){return x*.5+(i&127);}
int main(int argc,char**){if(demanded(argc)!=114372)return 2;
double x=0;int sum=0;for(int i=0;i<argc*3000000;++i){x=step(x,i);sum=(sum+int(x))%1009;}
return sum==176?0:1;}''',
'exceptions': '''int dead;struct Guard{~Guard(){++dead;}};
int step(int x){try{Guard g;throw x;}catch(int n){return n;}}
int main(int argc,char**){if(demanded(argc)!=114372)return 2;
long sum=0;for(int i=0;i<argc*200000;++i)sum+=step(i%97);
return sum==9599419 && dead==200000?0:1;}'''}
known = json.loads((here.parent/'pa26/evidence144/common-performance.json').read_text())['inputs']
sources = {}
for name,body in bodies.items():
    p = out/(name+'.cpp'); p.write_text(prefix+body)
    assert sha(p) == known[name]['sha256'], (name,sha(p))
    sources[name] = p
p = out/'pruning.cpp'
p.write_text(''.join('static int unused%d(int x){return x+%d;}\n'%(n,n) for n in range(1200))+prefix+bodies['memory'])
sources['pruning'] = p
result = dict(affinity=affinity,binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in binaries.items()},
              flags={'A':['-O'+os.environ.get('PA32_BASE_LEVEL','0'),'-c','--stats'],
                     'B':['-O'+os.environ.get('PA32_FINAL_LEVEL','1'),'-c','--stats']}, inputs={k:sha(v) for k,v in sources.items()},
              host_linker=run(['g++','--version']).stdout.decode(),runs=[],images={},summary={})
def save(): (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for name,source in sources.items():
    executables = {}; images = {}
    for label,binary in binaries.items():
        obj=out/(name+label+'.o'); exe=out/(name+label)
        run([binary,*result['flags'][label],source,'-o',obj]); run(['g++',obj,'-o',exe]); run([exe])
        executables[label] = exe
        images[label] = dict(object_sha256=sha(obj),executable_sha256=sha(exe),
            object_bytes=obj.stat().st_size,executable_bytes=exe.stat().st_size,
            object_text_bytes=text_size(obj),executable_text_bytes=text_size(exe))
    result['images'][name] = images
    for mode in ('compile','runtime'):
        for block,order in enumerate(['AAAA']+['ABBA']*6):
            for label in order:
                args = [binaries[label],*result['flags'][label],source,'-o',out/'measure.o'] if mode=='compile' else [executables[label]]
                started=time.perf_counter()
                proc=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]); elapsed=time.perf_counter()-started
                result['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=elapsed,
                    peak_rss_kib=int((out/'rss').read_text()),status=proc.returncode,
                    phase_counters=[json.loads(s) for s in proc.stderr.decode().splitlines() if s.startswith('{')]))
                save()
        rows=[r for r in result['runs'] if r['workload']==name and r['mode']==mode]
        ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/
                statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
        aa=[r['wall_s'] for r in rows if not r['block']]
        summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,
            paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
        for label in 'AB':
            samples=[r for r in rows if r['block'] and r['label']==label]
            summary[label]=dict(median_s=statistics.median(r['wall_s'] for r in samples),
                range_s=[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)],
                peak_rss_kib=max(r['peak_rss_kib'] for r in samples))
        result['summary'].setdefault(name,{})[mode]=summary; save()
    print(name,json.dumps(result['summary'][name]),flush=True)
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())
