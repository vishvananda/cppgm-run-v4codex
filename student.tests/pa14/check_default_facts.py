#!/usr/bin/env python3
"""Copy-initialization belongs to a stable, independent default fact."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else ROOT/'obj/dev'
san=len(sys.argv)>3 and sys.argv[3]=='sanitized'
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,cwd=ROOT,timeout=180)
 assert p.returncode==0,(cmd,p.returncode,p.stdout,p.stderr)
 return p.stdout
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if san else []
exe=WORK/'probe';run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),ROOT/'student.tests/pa14/default-facts.cc',*[objects/(n+'.o') for n in names],'-o',exe])
CASES={
 'valid':('template<class T>int f(T v=T(3));int(*pointer)(int)=f<int>;',False),
 'reference':('template<class T>int f(T& v=1);int(*pointer)(int&)=f<int>;',True),
 'pointer':('template<class T>int f(T v="wrong");int(*pointer)(int)=f<int>;',True),
 'explicit':('struct C{explicit C(int){}};template<class T>int f(T v=1);int(*pointer)(C)=f<C>;',True),
 'narrowing':('template<class T>int f(T v={1.5});int(*pointer)(int)=f<int>;',True),
 'dependent-name':('template<class T>int f(int v=T::missing);int(*pointer)(int)=f<int>;',True),
 'private-conversion':('class C{operator int(){return 1;}};template<class T>int f(int v=T());int(*pointer)(int)=f<C>;',True),
}
rows=[]
for name,(source,failed) in CASES.items():
 path=WORK/(name+'.cpp');path.write_text(source)
 text=run([exe,path,'failure' if failed else 'valid']);log=WORK/(name+'.log');log.write_text(text)
 stats=json.loads(text.splitlines()[1]);assert stats['template_default_argument_work']==1,stats
 rows.append(dict(name=name,source=source,failed=failed,log=str(log),exit_code=0));print(name,'PASS',flush=True)
(WORK/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
