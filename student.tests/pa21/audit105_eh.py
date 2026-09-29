#!/usr/bin/env python3
"""Check bulk-copy prefix lifetime preservation with host-thrown exceptions."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
decl='extern "C" void copied(int);extern "C" void destroyed(int);'
item='struct S{int n;S(int x):n(x){}S(const S&s):n(s.n+100){copied(n);}~S(){destroyed(n);}};'
prefix='struct E{int n;E(int x=50):n(x){}~E(){destroyed(n);}};'
cases={
 'closure_prefix':(decl+prefix+item+'extern "C" void run(){E e;S s(1);auto f=[e,s](){};auto g=f;}',201,[50,101,50,1,50]),
 'ordinary_prefix':(decl+prefix+item+'struct Box{E e;S s;Box():s(1){}};extern "C" void run(){Box a;Box b=a;}',101,[50,1,50]),
 'base_prefix':(decl+prefix+item+'struct Box:E{S s;Box():s(1){}};extern "C" void run(){Box a;Box b=a;}',101,[50,1,50]),
}
for n in (3,16):
 cases['array_prefix_'+str(n)]=(decl+prefix+item+'extern "C" void run(){E e['+str(n)+'];S s(1);auto f=[e,s](){};auto g=f;}',201,[50]*n+[101]+[50]*n+[1]+[50]*n)
rows=[]
for name,(source,fail,expected) in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source)
 driver=WORK/(name+'-host.cpp');ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=WORK/name
 driver.write_text('extern "C" void run();int seq[200],used;extern "C" void copied(int n){if(n=='+str(fail)+')throw n;}extern "C" void destroyed(int n){seq[used++]=n;}int main(){bool caught=false;try{run();}catch(int n){caught=n=='+str(fail)+';}int expected[]={'+','.join(map(str,expected))+'};if(!caught||used!='+str(len(expected))+')return 1;for(int i=0;i<used;++i)if(seq[i]!=expected[i])return 2;return 0;}')
 row=dict(name=name,source=source,harness=driver.read_text(),expected_destructions=expected,commands=[])
 cmds=[[CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',driver,obj,'-o',exe],[exe]]
 for cmd in cmds:
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60)
  row['commands'].append(dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr))
  if p.returncode:break
 row['passed']=len(row['commands'])==4 and p.returncode==0
 if ir.exists():row['lowir_sha256']=hashlib.sha256(ir.read_bytes()).hexdigest()
 rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',p.stderr,flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
