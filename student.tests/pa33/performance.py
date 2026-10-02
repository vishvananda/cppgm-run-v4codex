#!/usr/bin/env python3
"""Freeze inputs and A/B compilers, check results, then A/A and six ABBA blocks.
Host C++ only supplies measurement drivers and links compiler-produced objects.
"""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
bins=dict(zip('AB',[pathlib.Path(x).resolve() for x in sys.argv[2:4]]))
affinity=['taskset','-c',os.environ.get('PERF_CPU','2')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
    assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
    return p
def size(p):return sum(int(x.split()[1]) for x in run(['size','-A',p]).stdout.splitlines() if x.split() and x.split()[0].startswith('.text'))
workloads={}
for name,call in [('loop',False),('calls',True)]:
    header='function @step(%x : i64) -> i64 [binding=strong, linkage=c, no_inline=yes, unwind=no] { block ^entry: return i64 %x }\n' if call else ''
    body='''function @kernel{n}(%n : i64, %seed : i64) -> i64 [binding=strong, linkage=c, no_inline=yes] {{
 block ^entry:
  jump ^head
 block ^head:
  %i = phi i64 [^entry: 0, ^body: %next]
  %acc = phi i64 [^entry: %seed, ^body: %value]
  %more = cmp lt i64 %i, %n
  branch %more, ^body, ^done
 block ^body:
  %low = binary and i64 %i, 127
  {step}
  %plus = binary add i64 %acc, %part
  %value = binary and i64 %plus, 65535
  %next = binary add i64 %i, 1
  jump ^head
 block ^done:
  return i64 %acc
}}
'''
    step='%part = call i64 @step(%low)' if call else '%part = copy i64 %low'
    text=header+''.join(body.format(n=n,step=step) for n in range(1200))
    main='''extern "C" long kernel0(long,long);
int main(int argc,char**argv){if(argc!=2)return 2;long n=(argv[1][0]-48)*10000000L;
long expected=(7+(n/128)*8128+(n%128)*(n%128-1)/2)&65535;
return kernel0(n,7)==expected?0:1;}
'''
    workloads[name]=(text,main,['4'])
for name,length in [('short-string',7),('long-string',127)]:
    header='declare function @measure(%s : ptr) -> i64 [object=cppgm_builtin_strlen, effects=readonly, unwind=no]\n'
    body='function @kernel{n}(%s : ptr) -> i64 [binding=strong, linkage=c, no_inline=yes] {{ block ^entry: %v = call i64 @measure(%s) return i64 %v }}\n'
    main='''#include <cstring>
extern "C" long kernel0(const char*);
extern "C" long cppgm_builtin_strlen(const char* p){return std::strlen(p);}
int main(int argc,char**argv){if(argc!=2)return 2;char s[160];int length=LENGTH+(argv[1][0]-48);
std::memset(s,'a',length);s[length]=0;long sum=0;
for(int n=0;n!=30000000;++n)sum+=kernel0(s);
return sum==30000000L*length?0:1;}
'''.replace('LENGTH',str(length))
    workloads[name]=(header+''.join(body.format(n=n) for n in range(2000)),main,['0'])
header='''function @copy(%d : ptr [alias=noalias], %s : ptr [alias=noalias], %n : i64) -> ptr [object=cppgm_builtin_memcpy, unwind=no, no_inline=yes, binding=internal] {
 block ^entry: jump ^head
 block ^head:
  %i = phi i64 [^entry: 0, ^body: %next]
  %more = cmp lt i64 %i, %n
  branch %more, ^body, ^done
 block ^body:
  %src = index i8 %s, %i
  %v = load u8 %src
  %dst = index i8 %d, %i
  store u8 %v, %dst
  %next = binary add i64 %i, 1
  jump ^head
 block ^done: return ptr %d
}
'''
body='function @kernel{n}(%d : ptr, %s : ptr, %n : i64) -> void [binding=strong, linkage=c, no_inline=yes] {{ block ^entry: %unused = call ptr @copy(%d,%s,%n) return void }}\n'
main='''#include <cstring>
extern "C" void kernel0(char*,const char*,long);
int main(int argc,char**argv){if(argc!=2)return 2;char s[320],d[320];long length=(argv[1][0]-48)*64;
for(int i=0;i<320;++i)s[i]=char(i*17);long sum=0;
for(int i=0;i<10000000;++i){kernel0(d,s,length);sum+=static_cast<unsigned char>(d[i%length]);}
long expected=0;for(int i=0;i<10000000;++i)expected+=static_cast<unsigned char>(s[i%length]);
return sum==expected&&!std::memcmp(s,d,length)?0:1;}
'''
workloads['dynamic-copy']=(header+''.join(body.format(n=n) for n in range(2000)),main,['4'])
r=dict(binaries={k:dict(path=str(v),sha256=sha(v)) for k,v in bins.items()},flags=['-O2','-c'],affinity=affinity,runs=[],images={},inputs={},summary={})
def save():(out/'performance.json').write_text(json.dumps(r,indent=2)+'\n')
for name,(text,main,args) in workloads.items():
    src=out/(name+'.lowir');src.write_text(text)
    driver=out/(name+'.cpp');driver.write_text(main)
    r['inputs'][name]=dict(source_sha256=sha(src),driver_sha256=sha(driver),arguments=args)
    images={}
    for label in 'AB':
        obj=out/(name+label+'.o');exe=out/(name+label)
        run([bins[label],*r['flags'],src,'-o',obj]);run(['g++','-std=c++11','-O2',driver,obj,'-o',exe]);run([exe,*args])
        images[label]=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_text_bytes=size(obj),text_bytes=size(exe))
    r['images'][name]=images
    for mode in ['compile','runtime']:
        for block,order in enumerate(['AAAA']+['ABBA']*6):
            for label in order:
                cmd=[bins[label],*r['flags'],src,'-o',out/'measure.o'] if mode=='compile' else [out/(name+label),*args]
                start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*cmd])
                row=dict(workload=name,mode=mode,block=block,label=label,wall_s=time.perf_counter()-start,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode)
                if mode=='compile':
                    row['object_sha256']=sha(out/'measure.o');assert row['object_sha256']==images[label]['object_sha256']
                r['runs'].append(row);save()
        rows=[x for x in r['runs'] if x['workload']==name and x['mode']==mode]
        pairs=[statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='B')/statistics.mean(x['wall_s'] for x in rows if x['block']==b and x['label']=='A') for b in range(1,7)]
        aa=[x['wall_s'] for x in rows if not x['block']]
        summary=dict(AA_range_s=[min(aa),max(aa)],paired_ratios=pairs,paired_ratio_median=statistics.median(pairs),paired_ratio_range=[min(pairs),max(pairs)])
        for label in 'AB':
            samples=[x for x in rows if x['block'] and x['label']==label]
            summary[label]=dict(median_s=statistics.median(x['wall_s'] for x in samples),range_s=[min(x['wall_s'] for x in samples),max(x['wall_s'] for x in samples)],peak_rss_kib=max(x['peak_rss_kib'] for x in samples))
        r['summary'].setdefault(name,{})[mode]=summary;save()
        print(name,mode,json.dumps(summary),flush=True)
assert all(sha(v)==r['binaries'][k]['sha256'] for k,v in bins.items())
