#!/usr/bin/env python3
"""Standalone costs for newly supported imported VTT and CRTP static owners."""
import hashlib,json,os,pathlib,statistics,subprocess,sys,time
root=pathlib.Path(__file__).resolve().parents[2];here=root/'student.tests/pa28'
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
binaries=dict(zip('AB',(pathlib.Path(p).resolve() for p in sys.argv[2:4])))
affinity=['taskset','-c',os.environ['PERF_CPU']] if 'PERF_CPU' in os.environ else []
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):return subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
for name in ['ownership152.h','lazy152.h']: (out/name).write_bytes((here/name).read_bytes())
helper=out/'host.cpp';helper.write_text((here/'ownership-host152.cpp').read_text()+'''\n#include "lazy152.h"
template<class T> Deferred152<T>::~Deferred152() {}
template<class T> Level152<T>::~Level152() {}
template class Deferred152<Leaf152>;
template class Level152<Leaf152>;
int cross152(Entry152* p){return dynamic_cast<Side152*>(p)!=0;}
''')
p=run(['g++','-std=c++11','-O0','-c',helper,'-o',out/'host.o']);assert not p.returncode,p.stderr
checksum=sum(i%127+1 for i in range(3000000))%1009
source=out/'ownership.cpp';source.write_text('''#include "ownership152.h"
#include "lazy152.h"
int cross152(Entry152*);
template<int N> Entry152* get152(){static Leaf152 leaf;return &leaf;}
int demanded(){int sum=0;
'''+''.join('sum+=cross152(get152<%d>());\n'%n for n in range(400))+'''
return sum;}
int main(int argc,char**){if(demanded()!=400)return 2;int sum=0;
{HostLeaf152 object;
for(int i=0;i<argc*3000000;++i){if(!fill152(object,i%127+1))return 3;sum=(sum+object.value)%1009;}}
return destroyed152==1 && sum=='''+str(checksum)+'''?0:4;}
''')
flags=['-O0','-c','--stats']
result=dict(binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in binaries.items()},inputs={p.name:sha(p) for p in [source,helper,out/'ownership152.h',out/'lazy152.h']},flags=flags,host_flags=['-std=c++11','-O0','-c'],host_version=run(['g++','--version']).stdout.decode(),affinity=affinity,runs=[],summary={})
p=run([binaries['A'],*flags,source,'-o',out/'A.o']);assert p.returncode!=0
result['unsupported_A']=dict(status=p.returncode,stderr=p.stderr.decode())
obj,exe=out/'B.o',out/'B'
p=run([binaries['B'],*flags,source,'-o',obj]);assert p.returncode==0,p.stderr
p=run(['g++',obj,out/'host.o','-o',exe]);assert p.returncode==0,p.stderr
p=run([exe]);assert p.returncode==0,(p.returncode,p.stderr)
def text_size(path):
 p=run(['size','-A',path]);assert p.returncode==0
 return sum(int(s.split()[1]) for s in p.stdout.decode().splitlines() if s.split() and s.split()[0].startswith('.text'))
result['image']=dict(object_sha256=sha(obj),executable_sha256=sha(exe),object_bytes=obj.stat().st_size,executable_text_bytes=text_size(exe),object_text_bytes=text_size(obj))
def save():(out/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
for mode in ('compile','runtime'):
 for i in range(12):
  args=[binaries['B'],*flags,source,'-o',out/'measure.o'] if mode=='compile' else [exe]
  start=time.perf_counter();p=run(['/usr/bin/time','-f','%M','-o',out/'rss',*affinity,*args]);elapsed=time.perf_counter()-start
  assert p.returncode==0,(mode,p.stderr)
  result['runs'].append(dict(mode=mode,sample=i,wall_s=elapsed,peak_rss_kib=int((out/'rss').read_text()),status=p.returncode,phase_counters=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]))
  save()
 rows=[r for r in result['runs'] if r['mode']==mode]
 result['summary'][mode]=dict(median_s=statistics.median(r['wall_s'] for r in rows),range_s=[min(r['wall_s'] for r in rows),max(r['wall_s'] for r in rows)],peak_rss_kib=max(r['peak_rss_kib'] for r in rows));save()
assert all(sha(binaries[k])==v['sha256'] for k,v in result['binaries'].items())
print(json.dumps(result['summary']),flush=True)
