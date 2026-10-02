#!/usr/bin/env python3
"""Frozen affine-loop A/B, checked runtime inputs, A/A then six ABBA blocks."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert r.returncode==0,(args,r.returncode,r.stderr)
 return r
def size(p):return sum(int(s.split()[1]) for s in run(['size','-A',p]).stdout.splitlines() if s.split() and s.split()[0].startswith('.text'))
def kernel(name,unroll):
    body=''' %next_acc = binary add i64 %acc, %i
 store volatile i64 %next_acc, @observed
''' if unroll else ''
    carried=' %acc = phi i64 [^entry: %seed, ^body: %next_acc]\n' if unroll else ''
    return f'''function @{name}(%seed : i64) -> i64 [binding=strong, linkage=c, no_inline=yes] {{
block ^entry: jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
{carried} %more = cmp lt i64 %i, {4 if unroll else 32}
 branch %more, ^body, ^exit
block ^body:
{body} %next = binary add i64 %i, 1
 jump ^head
block ^exit:
 %answer = binary add i64 {'%acc' if unroll else '%seed'}, %i
 return i64 %answer
}}
'''
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in bins.items()},affinity=affinity,
 runs=[],inputs={},images={},summary={},diagnostic_targets=dict(compiler_ratio=1.5,rss_ratio=1.5,text_ratio=1.25),
 enforced_bounds=dict(trips=4,body_clones_per_loop=64,reserved_per_function=256,reserved_per_unit='min(4096,2*(I+1))',candidate_work='16*(I+O+E+1)'))
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name,unroll,level in [('finite',False,1),('unroll',True,3)]:
    source=out/(name+'.lowir');source.write_text('global @observed : i64 = 0\n'+''.join(kernel('kernel'+str(n),unroll) for n in range(1600)))
    # Use every kernel for compiler budgets, and execute a representative kernel
    # repeatedly with a runtime-varying value and independently checked result.
    main=out/(name+'.cpp');main.write_text('''extern "C" long kernel0(long);
int main(int argc,char**argv){if(argc!=2)return 3;long n=(argv[1][0]-48)*1000000L;
long x=0;for(long i=0;i<n;++i)x=(kernel0(x)+i)%%1009;
return x==%d?0:1;}
'''%(814 if unroll else 0))
    x=0
    for i in range(12000000):x=(x+(10 if unroll else 32)+i)%1009
    main.write_text(main.read_text().replace('x==814' if unroll else 'x==0','x=='+str(x)))
    r['inputs'][name]=dict(lowir_sha256=sha(source),main_sha256=sha(main),expected=x,runtime_arguments=['<'],flags=[f'-O{level}','-c'])
    # '<'-'0' is twelve, keeping the source independent of constant workload size.
    run([bins['A'],'-c','-O0',main,'-o',out/'main.o']);images={}
    for label in 'AB':
        obj=out/(name+label+'.o');exe=out/(name+label)
        run([bins[label],*r['inputs'][name]['flags'],source,'-o',obj]);run(['g++',out/'main.o',obj,'-o',exe]);run([exe,'<'])
        images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),text_bytes=size(exe),
          telemetry=[json.loads(s) for s in run([bins[label],*r['inputs'][name]['flags'],'--stats',source,'-o',out/'stats.o']).stderr.splitlines() if s.startswith('{')])
    r['images'][name]=images
    for mode in ['compile','runtime']:
        for block,order in enumerate(['AAAA']+['ABBA']*6):
            for label in order:
                args=[bins[label],*r['inputs'][name]['flags'],source,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),'<']
                start=time.perf_counter();proc=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args])
                row=dict(workload=name,mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=proc.returncode)
                if mode=='compile':
                    row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
                r['runs'].append(row);save()
        rows=[v for v in r['runs'] if v['workload']==name and v['mode']==mode]
        aa=[v['wall_s'] for v in rows if not v['block']]
        ratios=[statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='B')/statistics.mean(v['wall_s'] for v in rows if v['block']==b and v['label']=='A') for b in range(1,7)]
        summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
        for label in 'AB':
            samples=[v for v in rows if v['block'] and v['label']==label]
            summary[label]=dict(median_s=statistics.median(v['wall_s'] for v in samples),range_s=[min(v['wall_s'] for v in samples),max(v['wall_s'] for v in samples)],peak_rss_kib=max(v['peak_rss_kib'] for v in samples))
        r['summary'].setdefault(name,{})[mode]=summary;save();print(name,mode,json.dumps(summary),flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in bins.items())
