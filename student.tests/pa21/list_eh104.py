#!/usr/bin/env python3
"""Host throws through student-generated list construction and lifetime code."""
from pathlib import Path
import hashlib,json,subprocess,sys
from list104 import LIB,GOOD
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
decl='extern "C" void made(int);extern "C" void destroyed(int);extern "C" void used();'
obj='struct S{int n;S(int n):n(n){made(n);}S(const S&s):n(s.n){made(n);}~S(){destroyed(n);}};'
prefix=LIB+decl+obj
cases={}
for count in (1,3,16):
 for fail in (1,count,99,0):
  for owner in ('local','argument'):
   values=','.join(str(n) for n in range(1,count+1))
   body='std::initializer_list<S>x{'+values+'};used();' if owner=='local' else 'consume({'+values+'});'
   source=prefix+'void consume(std::initializer_list<S>){used();}extern "C" void run(){'+body+'}'
   destroyed=list(range((fail-1 if fail<=count and fail else count),0,-1))
   cases[f'{owner}_{count}_{fail}']=(source,fail,destroyed)
cases['nested']=(prefix+'extern "C" void run(){std::initializer_list<std::initializer_list<S>>x{{1,2},{3,4}};used();}',99,[4,3,2,1])
cases['constructor_argument']=(prefix+'struct Box{Box(std::initializer_list<S>){used();}};extern "C" void run(){Box b{1,2,3};}',99,[3,2,1])
def invoke(cmd):return subprocess.run([str(x) for x in cmd],capture_output=True,text=True,timeout=60)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
rows=[]
def check(name,source,host):
 src=WORK/(name+'.cpp');src.write_text(source)
 driver=WORK/(name+'-host.cpp');driver.write_text(host)
 ir=src.with_suffix('.lowir');objfile=src.with_suffix('.o');exe=WORK/name
 row=dict(name=name,source=source,harness=host,commands=[])
 cmds=[[CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',objfile,ir],['g++','-no-pie',driver,objfile,'-o',exe],[exe]]
 for cmd in cmds:
  p=invoke(cmd);row['commands'].append(dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
  if p.returncode:break
 row['passed']=len(row['commands'])==4 and p.returncode==0
 if ir.exists():row['lowir_sha256']=sha(ir)
 rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',p.stderr.strip(),flush=True)
for name,(source,fail,expected) in cases.items():
 host='extern "C" void run();int seq[100],count;extern "C" void made(int n){if(n=='+str(fail)+')throw n;}extern "C" void used(){if('+str(fail)+'==99)throw 99;}extern "C" void destroyed(int n){seq[count++]=n;}int main(){bool caught=false;try{run();}catch(int n){caught=n=='+str(fail)+';}int expected[]={'+','.join(map(str,expected or [0]))+'};if(caught!='+str(bool(fail)).lower()+'||count!='+str(len(expected))+')return 1;for(int i=0;i<count;++i)if(seq[i]!=expected[i])return 2;return 0;}'
 check(name,source,host)
source=LIB+decl+obj+'extern "C" void run(){static std::initializer_list<S>x{1,2};}'
host='#include <cstdlib>\nextern "C" void run();int count,seq;extern "C" void made(int){++count;}extern "C" void destroyed(int n){--count;seq=seq*10+n;}void verify(){if(count||seq!=21)std::_Exit(3);}int main(){std::atexit(verify);run();run();return count!=2;}'
check('local_static_atexit',source,host)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=sha(CC),object_backend_sha256=sha(ROOT/'reference-binaries/cppgm++'),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
