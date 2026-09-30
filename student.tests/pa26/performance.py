#!/usr/bin/env python3
"""Host object baseline: frozen final/final A/A and ABBA, all observations kept."""
import hashlib,json,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
binary=pathlib.Path(sys.argv[2]).resolve()
def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    r=subprocess.run(list(map(str,args)),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if r.returncode: raise RuntimeError((args,r.returncode,r.stderr.decode()))
    return r
# Fixed emitted template demand ensures compiler samples dominate process startup.
prefix='template<int N> long item(long x){return (x+N)%97;}\nlong demanded(long x){long s=0;\n'
prefix+=''.join('s+=item<%d>(x);\n'%n for n in range(2400))+'return s;}\n'
expected=sum((1+n)%97 for n in range(2400))
count=3000000
values=list(range(64)); total=0
for i in range(count):
    k=i&63; values[k]=(values[k]+((k*17+(i&255))%101))&65535; total=(total+values[k])%1009
memory=f'''int step(int x,int y){{return (x*17+y)%101;}}
int main(int argc,char**){{if(demanded(argc)!={expected})return 2;
int a[64];for(int i=0;i<64;++i)a[i]=i; int sum=0;
for(int i=0;i<argc*{count};++i){{int k=i&63;a[k]=(a[k]+step(k,i&255))&65535;sum=(sum+a[k])%1009;}}
return sum=={total}?0:1;}}'''
x=0.; total=0
for i in range(count): x=x*.5+(i&127); total=(total+int(x))%1009
floating=f'''double step(double x,int i){{return x*.5+(i&127);}}
int main(int argc,char**){{if(demanded(argc)!={expected})return 2;
double x=0;int sum=0;for(int i=0;i<argc*{count};++i){{x=step(x,i);sum=(sum+int(x))%1009;}}
return sum=={total}?0:1;}}'''
count=200000; total=sum(i%97 for i in range(count))
exceptions=f'''int dead;struct Guard{{~Guard(){{++dead;}}}};
int step(int x){{try{{Guard g;throw x;}}catch(int n){{return n;}}}}
int main(int argc,char**){{if(demanded(argc)!={expected})return 2;
long sum=0;for(int i=0;i<argc*{count};++i)sum+=step(i%97);
return sum=={total} && dead=={count}?0:1;}}'''
result={'binary':str(binary),'sha256':digest(binary),'flags':['-O0','-c','--stats'],
        'policy':'Final/final host baseline; no speedup claim. Link time is excluded.',
        'runs':[],'images':{},'inputs':{}}
for name,body in [('memory',memory),('floating',floating),('exceptions',exceptions)]:
    source=out/(name+'.cpp'); source.write_text(prefix+body); result['inputs'][name]=digest(source)
    obj=out/(name+'.o'); exe=out/name
    run([binary,'-O0','-c','--stats','-o',obj,source]); run(['g++',obj,'-o',exe]); run([exe])
    def text_size(path):
        return sum(int(line.split()[1]) for line in run(['size','-A',path]).stdout.decode().splitlines() if line.split() and line.split()[0]=='.text')
    result['images'][name]={'object_sha256':digest(obj),'executable_sha256':digest(exe),
       'object_text_bytes':text_size(obj),'executable_text_bytes':text_size(exe),
       'object_bytes':obj.stat().st_size,'executable_bytes':exe.stat().st_size}
    for mode in ['compile','runtime']:
        for block,order in enumerate(['AAAA']+['ABBA']*6):
            for label in order:
                args=[binary,'-O0','-c','--stats','-o',out/'measure.o',source] if mode=='compile' else [exe]
                started=time.perf_counter(); timefile=out/'time.txt'
                proc=run(['/usr/bin/time','-f','%M','-o',timefile,*args]); elapsed=time.perf_counter()-started
                result['runs'].append({'workload':name,'mode':mode,'block':block,'label':label,
                    'wall_s':elapsed,'peak_rss_kib':int(timefile.read_text()),
                    'phase_counters':[json.loads(line) for line in proc.stderr.decode().splitlines() if line.startswith('{')]})
                (out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
summary={}
for name in result['images']:
    summary[name]={'image':result['images'][name]}
    for mode in ['compile','runtime']:
        rows=[r for r in result['runs'] if r['workload']==name and r['mode']==mode]
        ratios=[]
        for b in range(1,7):
            means=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']==l) for l in 'AB']
            ratios.append(means[1]/means[0])
        aa=[r['wall_s'] for r in rows if r['block']==0]
        summary[name][mode]={'median_s':statistics.median(r['wall_s'] for r in rows),
            'min_max_s':[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],
            'peak_rss_kib':max(r['peak_rss_kib'] for r in rows),'AA_range_s':[min(aa),max(aa)],
            'paired_ratio_median':statistics.median(ratios),'paired_ratio_range':[min(ratios),max(ratios)]}
result['summary']=summary
(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(summary,indent=2))
