#!/usr/bin/env python3
"""Frozen owner A/A + ABBA evidence and corrected initializer scaling."""
import hashlib,json,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc={k:pathlib.Path(v).resolve() for k,v in zip('AB',sys.argv[2:4])}
flags=['-std=c++11','-O0','-c','--stats']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,allow=False):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
    if not allow:assert not p.returncode,(args,p.returncode,p.stderr)
    return p
def size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
inputs={}
for stage,family,filename in [(175,'source','source-performance'),(176,'declaration','declaration-performance'),(177,'selection','selection-performance')]:
    old=json.loads((root/('student.tests/pa29/evidence%d/%s.json'%(stage,filename))).read_text())
    for n in [600,1200,2400]:
        prior=old['inputs'][str(n)];src=pathlib.Path(prior['path']);assert sha(src)==prior['sha256']
        inputs[family+str(n)]=dict(path=str(src),sha256=sha(src),expected=prior['expected'],args=prior['runtime_args'],n=n,paired=True)
for n in [600,1200,2400]:
    src=out/('bound%d.cpp'%n);reps=24000000//n
    src.write_text('extern "C" long strtol(const char*,char**,int); extern "C" int printf(const char*,...);\nint calls;template<int N> long init(){++calls;return N;}\ntemplate<int N>struct Box{inline static long values[]={init<N>()};};\n'+''.join('static_assert(sizeof(Box<%d>::values)==sizeof(long),"bound");\n'%i for i in range(n))+'long work(long x){long s=0;\n'+''.join('s+=(Box<%d>::values[0]+x)%%97;\n'%i for i in range(n))+'return s;}\nint main(int argc,char** argv){if(calls!=%d)return 2;long seed=argc>1?strtol(argv[1],0,10):3;long sum=0;for(int i=0;i<%d;++i)sum+=work(seed+i%%13);printf("%%ld\\n",sum);}\n'%(n,reps))
    expected=str(sum(sum((j+17+i)%97 for j in range(n))*(reps//13+(i<reps%13)) for i in range(13)))+'\n'
    inputs['bound'+str(n)]=dict(path=str(src),sha256=sha(src),expected=expected,args=['17'],n=n,paired=False)
r=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in cc.items()},flags=flags,inputs=inputs,images={},runs=[],summary={},entry_checks={},launcher_s=[])
def save():(out/'performance.json').write_text(json.dumps(r,separators=(',',':'))+'\n')
for _ in range(8):
    begin=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss','/bin/true']);r['launcher_s'].append(time.perf_counter()-begin)
for name,item in inputs.items():
    src=pathlib.Path(item['path']);labels='AB' if item['paired'] else 'B';executables={};images={}
    if not item['paired']:
        p=run([cc['A'],*flags,src,'-o',out/'entry.o'],True);record=dict(status=p.returncode,stderr=p.stderr)
        if not p.returncode:
            p=run(['g++',out/'entry.o','-o',out/'entry-check'],True);record['link_status']=p.returncode
            if not p.returncode:
                p=run([out/'entry-check',*item['args']],True);record.update(runtime_status=p.returncode,stdout=p.stdout)
                assert p.returncode or p.stdout!=item['expected']
        r['entry_checks'][name]=record
    for label in labels:
        obj=out/(name+label+'.o');exe=out/(name+label)
        run([cc[label],*flags,src,'-o',obj]);run(['g++',obj,'-o',exe]);assert run([exe,*item['args']]).stdout==item['expected']
        executables[label]=exe;images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,text_bytes=size(exe))
    r['images'][name]=images
    for mode in ['compile','runtime']:
        orders=['AAAA']+['ABBA']*6 if item['paired'] else ['BBBB']*2
        for block,order in enumerate(orders):
            for label in order:
                args=[cc[label],*flags,src,'-o',out/'measure.o'] if mode=='compile' else [executables[label],*item['args']]
                begin=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);wall=time.perf_counter()-begin
                if mode=='runtime':assert p.stdout==item['expected']
                r['runs'].append(dict(workload=name,mode=mode,block=block,label=label,wall_s=wall,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]))
                save()
        rows=[a for a in r['runs'] if a['workload']==name and a['mode']==mode];s={}
        for label in labels:
            a=[x for x in rows if x['label']==label and (not item['paired'] or x['block'])]
            s[label]=dict(median_s=statistics.median(x['wall_s'] for x in a),range_s=[min(x['wall_s'] for x in a),max(x['wall_s'] for x in a)],peak_rss_kib=max(x['peak_rss_kib'] for x in a))
        if item['paired']:
            ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
            aa=[x['wall_s'] for x in rows if not x['block']]
            s.update(paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)],AA_range_s=[min(aa),max(aa)])
        r['summary'].setdefault(name,{})[mode]=s;save()
    print(name,json.dumps(r['summary'][name]),flush=True)
assert all(sha(cc[k])==v['sha256'] for k,v in r['binaries'].items())
