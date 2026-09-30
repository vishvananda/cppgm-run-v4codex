#!/usr/bin/env python3
"""Freeze inputs and compare two correct PA25 driver binaries; preserve all samples."""
import hashlib, json, os, pathlib, statistics, struct, subprocess, sys, time
out = pathlib.Path(sys.argv[1]); out.mkdir(parents=True,exist_ok=True)
A,B = map(lambda x:pathlib.Path(x).resolve(),sys.argv[2:4])
inputs = out/'inputs'; inputs.mkdir(exist_ok=True)
def save(name,text):
    p=inputs/name; p.write_text(text); return p
# Demanded templates, independent TUs and many external fixups.
templates=[]
for tu in range(8):
    text='template<int N> int item(int x){return (x+N)%97;}\n'
    text+=f'int chunk{tu}(int x) {{ int result=0;\n'
    text+=''.join(f'result+=item<{n}>(x);\n' for n in range(tu*600,(tu+1)*600))
    text+='return result; }\n'; templates.append(save(f'template{tu}.cc',text))
expected=sum((x+n)%97 for x in range(400) for n in range(4800))
main=''.join(f'int chunk{i}(int);\n' for i in range(8))
main+='int main(int argc,char**){long long result=0; for(int x=0;x<argc*400;++x){'
main+=''.join(f'result+=chunk{i}(x);' for i in range(8))
main+=f'}} return result=={expected}LL ? 0 : 1;}}\n'
templates.append(save('template-main.cc',main))
# Runtime-driven loops/calls/memory/floating operations, checked against an
# independently computed value. No volatile/static compile-time-only workload.
count=3000000
values=[i for i in range(64)]; total=0
for i in range(count):
    k=i&63; values[k]=(values[k]+((k*17+(i&255))%101))&65535
    total=(total+values[k])%1009
memory=save('memory.cc',f'''int step(int x,int y){{return (x*17+y)%101;}}
int main(int argc,char**){{int values[64]; for(int i=0;i<64;++i) values[i]=i;
int total=0;for(int i=0;i<argc*{count};++i){{int k=i&63;values[k]=(values[k]+step(k,i&255))&65535;total=(total+values[k])%1009;}}
return total=={total}?0:1;}}
''')
x=0.; total=0
for i in range(count):
    x=x*.5+(i&127); total=(total+int(x))%1009
floating=save('floating.cc',f'''double step(double x,int i){{return x*0.5+(i&127);}}
int main(int argc,char**){{double x=0;int total=0;for(int i=0;i<argc*{count};++i){{x=step(x,i);total=(total+(int)x)%1009;}}return total=={total}?0:1;}}
''')
workloads={'templates':(templates,1),'memory':([memory],64),'floating':([floating],64)}
if '--wide-baseline' in sys.argv[4:]:
    assert A.read_bytes()==B.read_bytes(), 'wide baseline requires the same correct binary'
    count=200000; modulus=(1<<119)+23; x=(1<<100)+1
    for i in range(count): x=(x*3+i)%modulus
    wide=save('wide.cc',f'''using U=unsigned __int128;
U step(U x,int i){{return (x*3+i)%((U(1)<<119)+23);}}
int main(int argc,char**){{U x=(U(1)<<100)+argc;
for(int i=0;i<argc*{count};++i)x=step(x,i);
return x==((U({x>>64}ULL)<<64)|U({x&((1<<64)-1)}ULL))?0:1;}}
''')
    workloads={'wide':([wide],64)}
if '--statements-baseline' in sys.argv[4:]:
    assert A.read_bytes()==B.read_bytes(), 'statement baseline requires the same correct binary'
    count=3000000
    expected=sum((i%97)*(3 if (i%97)&1 else 2) for i in range(count))
    statements=save('statements.cc',f'''struct Guard {{ int& dead;
Guard(int& x) noexcept:dead(x){{}} ~Guard() noexcept{{++dead;}} }};
int step(int n,int& dead){{return ({{Guard g(dead);if(n&1)return n*3;n*2;}});}}
int main(int argc,char**){{int dead=0;long long sum=0;
for(int i=0;i<argc*{count};++i)sum+=step(i%97,dead);
return sum=={expected}LL && dead==argc*{count}?0:1;}}
''')
    workloads={'statements':([statements],64)}
if '--class-baseline' in sys.argv[4:]:
    assert A.read_bytes()==B.read_bytes(), 'new class runtime needs a correct final/final baseline'
    classes=save('classes.cc',"""struct A{int value;constexpr A(int n):value(n){} virtual int f(){return value;}};
struct B{int value;constexpr B(int n):value(n){} virtual int g(){return value;}};
template<int N> struct D:A,B{constexpr D():A(N),B(N+1){} int f(){return value_a();} int value_a(){return A::value;}};
"""+''.join(f'D<{i}> item{i};\n' for i in range(400))+
        'int main(int argc,char**){long long total=0;for(int j=0;j<argc*20000;++j){\n'+''.join(f'total+=item{i}.f();\n' for i in range(400))+
        '}return total==1596000000LL*argc?0:1;}\n')
    casts=save('casts.cc',"""struct A{virtual int f(){return 2;}};
struct B{int v; B(int n) noexcept:v(n){} virtual int g(){return v;}};
template<int N> struct D:A,B{D(int n) noexcept:B(n+N){}};
int step(A* p){D<7>* d=dynamic_cast<D<7>*>(p);B* b=dynamic_cast<B*>(p);return d && b?b->g():0;}
int main(int argc,char**){D<7> d(argc*3); A* a=&d;long long total=0;
for(int i=0;i<argc*400000;++i)total+=step(a);
return total==4000000?0:1;}
""")
    allocations=save('allocations.cc',"""struct A{virtual ~A() noexcept{}};
struct B{virtual ~B() noexcept{}};
int destroyed; struct D:A,B{int v;D(int n) noexcept:v(n){} ~D() noexcept{++destroyed;}};
int main(int argc,char**){long long total=0;
for(int i=0;i<argc*10000;++i){D* d=new D(i);total+=d->v;B* b=d;delete b;}
return total==49995000 && destroyed==10000?0:1;}
""")
    workloads={'classes':([classes],2),'casts':([casts],64),'allocations':([allocations],64)}
if '--exception-baseline' in sys.argv[4:]:
    assert A.read_bytes()==B.read_bytes(), 'exception baseline requires one correct frozen compiler'
    exceptions=save('exceptions.cc','''int dead,copies;
struct A{int n;A(int x):n(x){}virtual ~A(){++dead;}};
struct B{int padding;B():padding(9){}};
struct D:B,A{D(int x):A(x){}D(const D& d):A(d.n){++copies;}};
int step(int n){try{try{throw D(n);}catch(A& a){if(a.n!=n)return -1;throw;}}
catch(D d){return d.n; }return -2;}
int main(int argc,char**){long long total=0;
for(int i=0;i<argc*60000;++i)total+=step(i%97);
return total==2878839 && dead==120000 && copies==60000?0:1;}
''')
    workloads={'exceptions':([exceptions],32)}
def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p=subprocess.run(list(map(str,args)),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if p.returncode: raise RuntimeError((args,p.returncode,p.stderr.decode(errors='replace')))
    return p
# Preserve ordinary ELF images from each frozen binary for equality and timing.
manifest={str(p):digest(p) for p in [A,B,*inputs.glob('*.cc')]}
results={'manifest':manifest,'flags':['-O0'],'policy':'O0 frozen comparison; interpretation and transform budgets belong to the accompanying report; new-behavior modes are final/final calibration', 'runs':[],'images':{}}
for name,(sources,_) in workloads.items():
    images=[]
    for label,binary in [('A',A),('B',B)]:
        exe=out/(name+'-'+label); run([binary,'-O0','-o',exe,*sources]); run([exe]); images.append(exe)
    data=images[0].read_bytes(); ph=struct.unpack_from('<Q',data,32)[0]
    # RX PT_LOAD includes the ELF header; generated instruction bytes follow it.
    text=struct.unpack_from('<Q',data,ph+32)[0]-176
    measurements={}
    for label,exe in zip(('A','B'),images):
        data=exe.read_bytes();ph=struct.unpack_from('<Q',data,32)[0]
        measurements[label]={'sha256':digest(exe),'text_bytes':struct.unpack_from('<Q',data,ph+32)[0]-176,'file_bytes':len(data)}
    results['images'][name]={**measurements['B'],'versions':measurements,'byte_identical':images[0].read_bytes()==images[1].read_bytes()}
    for mode in ('compile','runtime'):
        repeat=workloads[name][1] if mode=='compile' else 3
        for block,order in enumerate(['AAAA','ABBA','ABBA','ABBA','ABBA','ABBA','ABBA']):
            for label in order:
                binary=A if label=='A' else B
                rss=[]; started=time.perf_counter()
                for iteration in range(repeat):
                    timefile=out/'time.txt'
                    args=[binary,'-O0','-o',out/'measure.program',*sources] if mode=='compile' else [out/(name+'-'+label)]
                    run(['/usr/bin/time','-f','%M','-o',timefile,*args]); rss.append(int(timefile.read_text()))
                elapsed=time.perf_counter()-started
                results['runs'].append({'workload':name,'mode':mode,'block':block,'label':label,'repeat':repeat,'wall_s':elapsed,'peak_rss_kib':max(rss)})
                (out/'performance.json').write_text(json.dumps(results,indent=2)+'\n')
summary={}
for name in workloads:
    row={**results['images'][name]}
    for mode in ('compile','runtime'):
        sample=[x for x in results['runs'] if x['workload']==name and x['mode']==mode]
        pairs=[]
        for block in range(1,7):
            a=[x['wall_s'] for x in sample if x['block']==block and x['label']=='A']
            b=[x['wall_s'] for x in sample if x['block']==block and x['label']=='B']
            pairs.append(statistics.mean(b)/statistics.mean(a))
        noise=[x['wall_s'] for x in sample if x['block']==0]
        row[mode]={'ratio_median':statistics.median(pairs),'ratio_range':[min(pairs),max(pairs)],'AA_range_s':[min(noise),max(noise)],'B_median_s':statistics.median([x['wall_s'] for x in sample if x['label']=='B']),'B_peak_rss_kib':max(x['peak_rss_kib'] for x in sample if x['label']=='B')}
    summary[name]=row
results['summary']=summary
(out/'performance.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(summary,indent=2))
