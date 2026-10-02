#!/usr/bin/env python3
"""Frozen source/ABI/lifecycle A/B: A/A calibration and six ABBA blocks."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity=['taskset','-c',os.environ.get('PERF_CPU','2')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
    assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
    return p
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},affinity=affinity,
       flags=['-O1','-c'],inputs={},runs=[],images={},summary={},
       budgets=dict(display='O(declarations + rendered bytes); 4 bytes/entity plus transient flat counters',
                    object_projection='O(1) completed-layout lookup per conversion; no IR growth',
                    roots='O(symbols + aliases + instructions + operands); no IR growth',
                    cold_actions='one helper per program; no cloning'))
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name in ['layout','move','landing']:
    count=1200;iterations=16000000
    declaration='extern "C" long kernel0(long);'
    if name=='layout':
        pre='''struct V {long n;V(long x):n(x){} ~V(){}};
struct D:virtual V {D(long x):V(x){} ~D(){}};
'''
        body='extern "C" __attribute__((noinline)) long kernel0(long x){D d(x);return d.n;}\n'
        driver='';check=''
    elif name=='move':
        pre='''struct H {long n;H(long x):n(x){} __attribute__((noinline)) H(H&& h):n(h.n){h.n=0;}};
struct D {H h;long a,b,c;D(long x):h(x),a(3),b(5),c(7){}
 __attribute__((noinline)) D(D&&)=default;};
'''
        body='extern "C" __attribute__((noinline)) long kernel0(long x){D a(x);D b(static_cast<D&&>(a));return b.h.n+b.a+b.b+b.c-15;}\n'
        driver='';check=''
    else:
        pre='''void touch(long);
template<int N> struct D {long n;D(long x):n(x){} __attribute__((noinline)) ~D(){touch(n+N);}};
'''
        body='__attribute__((noinline)) long kernel0(long x){D<NUMBER> d(x);return d.n;}\n'
        declaration='long kernel0(long);'
        driver='long observed;__attribute__((noinline)) void touch(long x){observed+=x;}\n'
        check='||observed!=sum'
    src=out/(name+'.cpp')
    src.write_text(pre+''.join(body.replace('kernel0(',f'kernel{k}(').replace('NUMBER',str(k)) for k in range(count)))
    main=out/(name+'-main.cpp');expected=iterations//256*32640
    main.write_text(f'''{declaration}
{driver}int main(int argc,char**argv){{if(argc!=2)return 3;long n=(argv[1][0]-48)*{iterations//8}L;
long sum=0,value=0;for(long i=0;i<n;++i){{value=kernel0((value+1)&255);sum+=value;}}
return sum!={expected}L{check};}}
''')
    r['inputs'][name]=dict(source=str(src),source_sha256=sha(src),main=str(main),main_sha256=sha(main),arguments=['8'],expected=expected)
    run([bins['A'],'-O0','-c',main,'-o',out/'main.o']);images={}
    for label in 'AB':
        obj=out/(name+label+'.o');exe=out/(name+label)
        run([bins[label],*r['flags'],src,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,'8'])
        images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),text_bytes=size(exe),
                           telemetry=[json.loads(s) for s in run([bins[label],*r['flags'],'--stats',src,'-o',out/'stats.o']).stderr.splitlines() if s.startswith('{')])
        (out/(name+label+'.asm')).write_text(run(['objdump','-d','--disassemble='+('_Z7kernel0l' if name=='landing' else 'kernel0'),obj]).stdout)
    r['images'][name]=images;save()
    for mode in ['compile','runtime']:
        for block,order in enumerate(['AAAA']+['ABBA']*6):
            for label in order:
                command=[bins[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),'8']
                start=time.perf_counter();proc=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*command])
                row=dict(workload=name,mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=proc.returncode)
                if mode=='compile':
                    row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
                r['runs'].append(row);save()
        rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];aa=[x['wall_s'] for x in rows if not x['block']]
        ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
        summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
        for label in 'AB':
            samples=[x for x in rows if x['block'] and x['label']==label]
            summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in samples),range_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
        r['summary'].setdefault(name,{})[mode]=summary;save();print(name,mode,json.dumps(summary),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())
