#!/usr/bin/env python3
"""Final fixed workloads and a many-call scratch-lifetime benchmark, sequential."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
ROOT=pathlib.Path(__file__).resolve().parents[2]
OUT=pathlib.Path(sys.argv[1]).resolve();OUT.mkdir(parents=True,exist_ok=True)
ART=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])
commands=[]
def run(args,env=None):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,env=env,timeout=180)
    assert p.returncode==0,(args,p.returncode,p.stderr)
    return p
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
for lane,script,level in [('affected','pa33/performance.py',None),('common-o0','pa32/common_levels.py','0'),('common-o2','pa32/common_levels.py','2'),('selfhost','pa32/selfhost_performance.py',None)]:
    cmd=['python3',ROOT/'student.tests'/script,OUT/lane,ART/'pa33-219/entry/cppgm++',OUT/'final/cppgm++']
    env=dict(os.environ,PERF_CPU='2')
    if level is not None:env.update(PA32_BASE_LEVEL=level,PA32_FINAL_LEVEL=level)
    with (OUT/(lane+'.log')).open('w') as log:p=subprocess.run(list(map(str,cmd)),env=env,stdout=log,stderr=subprocess.STDOUT)
    commands.append(dict(command=list(map(str,cmd)),status=p.returncode,level=level))
    (OUT/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
    assert p.returncode==0,lane
    print(lane,'PASS',flush=True)
# Alternate a mixed 8-GPR/2-XMM/stack call, a small call and a zero-argument
# call. Repeated calls must clear done flags and stack/ABI facts without
# releasing capacity. Every call contributes to the externally checked result.
out=OUT/'scratch';out.mkdir(exist_ok=True)
src=out/'scratch.lowir';driver=out/'driver.cpp'
header='''declare function @wide(%a : i64, %x : f64, %b : i64, %y : f64, %c : i64, %d : i64, %e : i64, %f : i64, %g : i64, %h : i64) -> i64
declare function @small(%a : i64) -> i64
declare function @empty() -> i64
'''
functions=[]
for f in range(160):
    body=[f'function @kernel{f}(%seed : i64) -> i64 [binding=strong, linkage=c, no_inline=yes] {{ block ^entry:']
    value='%seed'
    for n in range(64):
        body.extend([f'%w{n} = call i64 @wide({value},1.25,1,2.75,2,3,4,5,6,7)',f'%s{n} = call i64 @small(%w{n})',f'%e{n} = call i64 @empty()',f'%v{n} = binary add i64 %s{n}, %e{n}'])
        value=f'%v{n}'
    functions.append('\n'.join(body+[f'return i64 {value}', '}']))
src.write_text(header+'\n'.join(functions)+'\n')
driver.write_text('''extern "C" long kernel0(long);
extern "C" long wide(long a,double x,long b,double y,long c,long d,long e,long f,long g,long h){return (a+b+c+d+e+f+g+h+long(x+y))&65535;}
extern "C" long small(long a){return a+1;}
extern "C" long empty(){return 1;}
int main(int argc,char**argv){if(argc!=2)return 2;long seed=argv[1][0]-48,sum=0;
for(int i=0;i<400000;++i)sum+=kernel0(seed);
return sum==400000L*(seed+64*34)?0:1;}
''')
bins={'A':ART/'pa33-219/final/cppgm++','B':OUT/'final/cppgm++'}
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},flags=['-O2','-c'],affinity=['taskset','-c','2'],inputs={str(p):sha(p) for p in [src,driver]},runs=[],images={},summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for k in 'AB':
    obj=out/(k+'.o');exe=out/k
    run([bins[k],*r['flags'],src,'-o',obj]);run(['g++','-O2',driver,obj,'-o',exe]);run([exe,'3'])
    r['images'][k]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),text_bytes=size(exe))
assert (out/'A.o').read_bytes()==(out/'B.o').read_bytes()
for mode in ['compile','runtime']:
    for block,order in enumerate(['AAAA']+['ABBA']*6):
        for k in order:
            args=[bins[k],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [out/k,'3']
            start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*r['affinity'],*args])
            row=dict(mode=mode,block=block,label=k,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode)
            if mode=='compile':
                row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==r['images'][k]['object_sha256']
            r['runs'].append(row);save()
    rows=[v for v in r['runs'] if v['mode']==mode]
    ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
    aa=[v['wall_s'] for v in rows if not v['block']]
    s=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
    for k in 'AB':
        v=[x for x in rows if x['block'] and x['label']==k]
        s[k]=dict(median_s=statistics.median(x['wall_s'] for x in v),range_s=[min(x['wall_s'] for x in v),max(x['wall_s'] for x in v)],peak_rss_kib=max(x['peak_rss_kib'] for x in v))
    r['summary'][mode]=s;save()
print('scratch PASS',flush=True)
