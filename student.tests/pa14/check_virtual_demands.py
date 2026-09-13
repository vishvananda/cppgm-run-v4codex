#!/usr/bin/env python3
"""Key availability and lifecycle edges, including publication during finish."""
from pathlib import Path
import json, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else ROOT/'obj/dev'
sanitized=len(sys.argv)>3 and sys.argv[3]=='sanitized'
CASES={
 'reverse':('struct A{virtual int f();};struct B{virtual int f();};int B::f(){return 2;}int A::f(){return 1;}',2,2),
 'lifecycle':('struct A{virtual int f();virtual ~A(){}}; A a; int A::f(){return 1;}',1,0),
 'late':('template<class T>struct C{virtual int f();};template<class T>int C<T>::f(){return 7;} C<int>& instance();template<class T>int invoke(C<T>& c){return c.C<T>::f();}int result(){return invoke(instance());}',1,1),
 'unused':('template<class T>struct C{virtual int f(){return T::missing;}}; C<int>* pointer;',0,0),
 'failure':('struct Target{virtual ~Target(){};static void operator delete(void*,char*);};template<class T>void invoke(T* p){p->~T();}void test(Target* p){invoke(p);}',0,0),
 'body-failure':('template<class T>struct C{virtual int f();};template<class T>int C<T>::f(){return T::missing;} C<int>& instance();template<class T>int invoke(C<T>& c){return c.C<T>::f();}int result(){return invoke(instance());}',1,1),
}
def run(command):
 p=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,text=True,timeout=180)
 assert p.returncode==0,(command,p.returncode,p.stdout,p.stderr)
 return p.stdout
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if sanitized else []
exe=WORK/'probe'
run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),ROOT/'student.tests/pa14/virtual-demands.cc',*[objects/(n+'.o') for n in names],'-o',exe])
rows=[]
for name,(source,emissions,notifications) in CASES.items():
 path=WORK/(name+'.cpp');path.write_text(source)
 output=run([exe,path,name if 'failure' in name else 'success'])
 log=WORK/(name+'.log');log.write_text(output)
 stats=json.loads(output.splitlines()[-1].replace('{,','{'))
 assert stats['semantic_vtable_emissions']==emissions,(name,stats)
 assert stats['semantic_key_vtable_notifications']==notifications,(name,stats)
 assert stats['semantic_key_vtable_processed']==notifications,(name,stats)
 rows.append(dict(name=name,source=source,emissions=emissions,notifications=notifications,log=str(log),exit_code=0))
 print(name,'PASS',flush=True)
(WORK/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
