#!/usr/bin/env python3
"""Audit-owner frozen A/A+ABBA and newly correct cv-array scaling, four dimensions."""
import pathlib,subprocess,json,hashlib,statistics,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=dict(zip('AB',[pathlib.Path(p).resolve() for p in sys.argv[2:4]]))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
r=dict(flags=['-std=c++17','-O0','-c','--stats'],binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in cc.items()},inputs={},runs=[],summary={},launchers=[])
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
def run(args,ok=True):
    args=list(map(str,args));p=subprocess.run(args,capture_output=True,text=True,timeout=120)
    if ok:assert p.returncode==0,(args,p.returncode,p.stderr)
    return p
def text_size(p):return sum(int(l.split()[1]) for l in run(['size','-A',p]).stdout.splitlines() if l.split() and l.split()[0].startswith('.text'))
for _ in range(8):
    begin=time.perf_counter();run(['/usr/bin/time','-f','%M','-o',out/'rss','/bin/true']);r['launchers'].append(time.perf_counter()-begin)
for family,n in [('straight',2400),('branch',1200),('floating',1200),('cv-array',600),('cv-array',1200),('cv-array',2400)]:
    name=family+str(n);src=out/(name+'.cpp');reps=12000000//n
    if family in ['straight','cv-array']:
        body='return (x+N)%97;';fn=lambda i,x:(x+i)%97
    elif family=='branch':
        body='if((x+N)%3==0)return (x+N)%97;return (x+N)%37;';fn=lambda i,x:(x+i)%(97 if (x+i)%3==0 else 37)
    else:
        body='double y=double((x+N)%97)*.5+1.;return long(y);';fn=lambda i,x:((x+i)%97)//2+1
    if family=='cv-array':body='const int a[2]={int(x),N};auto [first,second]=a;static_assert(__is_same(decltype(first),const int),"cv");return (first+second)%97;'
    head='template<int N> '+('' if family=='cv-array' else '__attribute__((always_inline)) inline ')+'long work(long x){'+body+'}'
    src.write_text('extern "C" long strtol(const char*,char**,int);extern "C" int printf(const char*,...);\n'+head+'\nlong demanded(long x){long sum=0;'+''.join('sum+=work<%d>(x);'%i for i in range(n))+'return sum;}\nint main(int argc,char** argv){long seed=argc>1?strtol(argv[1],0,10):3;long sum=0;for(int i=0;i<%d;++i)sum+=demanded(seed+i%%13);printf("%%ld\\n",sum);}\n'%reps)
    expected=str(sum(sum(fn(i,17+j) for i in range(n))*(reps//13+(j<reps%13)) for j in range(13)))+'\n'
    entry=run([cc['A'],*r['flags'],src,'-o',out/'entry.o'],False)
    assert bool(entry.returncode)==(family=='cv-array'),entry.stderr
    labels='B' if family=='cv-array' else 'AB';images={};executables={}
    for label in labels:
        obj=out/(name+label+'.o');exe=out/(name+label)
        run([cc[label],*r['flags'],src,'-o',obj]);run(['g++',obj,'-o',exe]);assert run([exe,'17']).stdout==expected
        images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),text_bytes=text_size(exe));executables[label]=exe
    host=out/(name+'-host');run(['g++','-std=c++17','-O0',src,'-o',host]);assert run([host,'17']).stdout==expected
    r['inputs'][name]=dict(path=str(src),source=src.read_text(),sha256=sha(src),n=n,family=family,args=['17'],expected=expected,entry_status=entry.returncode,entry_stderr=entry.stderr,images=images)
    for mode in ['compile','runtime']:
        for block,order in enumerate(['BBBB','BBBB'] if family=='cv-array' else ['AAAA']+['ABBA']*6):
            for label in order:
                args=[cc[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [executables[label],'17']
                begin=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*args]);elapsed=time.perf_counter()-begin
                if mode=='runtime':assert p.stdout==expected
                r['runs'].append(dict(workload=name,mode=mode,label=label,block=block,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,counters=[json.loads(l) for l in p.stderr.splitlines() if l.startswith('{')]))
                save()
        rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode];summary={}
        for label in labels:
            samples=[x for x in rows if x['label']==label and (family=='cv-array' or x['block'])]
            summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in samples),range_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
        if family!='cv-array':
            aa=[x['wall_s'] for x in rows if not x['block']]
            ratios=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
            summary.update(AA_range_s=[min(aa),max(aa)],paired_ratios=ratios,paired_ratio_median=statistics.median(ratios),paired_ratio_range=[min(ratios),max(ratios)])
        r['summary'].setdefault(name,{})[mode]=summary;save()
    print(name,r['summary'][name],flush=True)
assert all(sha(p)==r['binaries'][k]['sha256'] for k,p in cc.items())
