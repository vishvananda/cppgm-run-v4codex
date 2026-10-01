#!/usr/bin/env python3
"""Audit repaired-owner scaling; common equivalent A/B uses performance147_common."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
    if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
    return p
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O0','-c','--stats'],affinity=affinity,inputs={},images={},runs=[],summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for i in range(8):
    start=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,'true'])
    r['launchers'].append(dict(wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text())))
iterations=2000000;real=7;imag=0;total_real=0;total_imag=0
for i in range(iterations):
    real,imag=-imag+(i%7-3),real
    total_real+=real;total_imag+=imag
runtime=f'''D step(D x,int i){{return x*__builtin_complex(0.0,1.0)+double(i%7-3);}}
template<class T> T roundtrip(T x){{try{{throw x;}}catch(const T& y){{return y;}}}}
int main(int argc,char**argv){{if(!check())return 2;
D x=__builtin_complex(double(argc>1?argv[1][0]-'0':1),0.0);long r=0,s=0;
for(int i=0;i<{iterations};++i){{x=step(x,i);if(i%100==0)x=roundtrip(x);r+=long(__real__ x);s+=long(__imag__ x);}}
return r=={total_real}&&s=={total_imag}&&__real__ x=={real}&&__imag__ x=={imag}?0:1;}}
'''
for family in ['identity','lists']:
    for n in [600,1200,2400]:
        name=family+str(n);source='using D=_Complex double;\n'
        if family=='identity':
            source+=''.join(f'constexpr D v{i}=__builtin_complex(3.0,{i+1}.0),dup{i}=v{i};\n' for i in range(n))
            source+='const D* values[]={'+','.join(f'&v{i},&dup{i}' for i in range(n))+'};\n'
            source+=f'int check(){{long s=0;for(int i=0;i<{2*n};++i){{if(__real__ *values[i]!=3)return 0;s+=long(__imag__ *values[i]);}}return s=={n*(n+1)};}}\n'
        else:
            source+='struct flag{int n;constexpr explicit operator bool()const{return n==3;}};\n'
            source+=''.join(f'template<class T> void unused{i}(){{static_assert(flag{{3}},"fixed list");}}\n' for i in range(n))
            source+='int check(){return 1;}\n'
        source+=runtime
        src=out/(name+'.cpp');src.write_text(source);obj=out/(name+'.o');exe=out/name
        entry=[]
        for args in [[bins['A'],*r['flags'],src,'-o',out/'entry.o'],['g++',out/'entry.o','-o',out/'entry'],[out/'entry','7']]:
            p=run(args,False);entry.append(dict(command=list(map(str,args)),status=p.returncode,stderr=p.stderr))
            if p.returncode:break
        assert entry[-1]['status']!=0
        r['inputs'][name]=dict(source=source,sha256=sha(src),N=n,iterations=iterations,seed=7,expected=[total_real,total_imag,real,imag],entry_failure=entry)
        run([bins['B'],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe,'7'])
        text=sum(int(l.split()[1]) for l in run(['size','-A',exe]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
        r['images'][name]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text)
        run(['clang++','-std=c++11','-O0',src,'-o',out/'host']);run([out/'host','7'])
        for mode in ['compile','runtime']:
            for trial in range(8):
                args=[bins['B'],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [exe,'7']
                start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
                r['runs'].append(dict(workload=name,mode=mode,trial=trial,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]))
                save()
            rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode]
            r['summary'].setdefault(name,{})[mode]=dict(median_s=statistics.median(x['wall_s'] for x in rows),range_s=[min(x['wall_s'] for x in rows),max(x['wall_s'] for x in rows)],peak_rss_kib=max(x['peak_rss_kib'] for x in rows));save()
        print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(bins[k])==v['sha256'] for k,v in r['binaries'].items());save()
