#!/usr/bin/env python3
"""Default conversion, dependency publication and body failure have separate owners."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else ROOT/'obj/dev'
san=len(sys.argv)>3 and sys.argv[3]=='sanitized'
def run(cmd):
 p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,cwd=ROOT,timeout=180)
 assert p.returncode==0,(cmd,p.returncode,p.stdout,p.stderr)
 return p.stdout
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if san else []
exe=W/'probe';run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),ROOT/'student.tests/pa14/default-demand-states.cc',*[objects/(n+'.o') for n in names],'-o',exe])
cases={
 'dependency-failure':'template<class T>struct C{static int get();};template<class T>int C<T>::get()noexcept(sizeof(typename T::missing)){return 4;}int f(int n=C<int>::get());',
 'body-failure':'template<class T>int make(){return T::missing;}int f(int n=make<int>());',
 'isolation':'template<class T>struct C{static int f(T v=T(4));};C<int> a;C<long> b;',
}
rows=[]
for name,source in cases.items():
 path=W/(name+'.cpp');path.write_text(source);text=run([exe,path,name]);log=W/(name+'.log');log.write_text(text)
 stats=json.loads(text.splitlines()[1]);expected=2 if name=='isolation' else 1
 assert stats['template_default_argument_work']==stats['template_default_facts']==expected,stats
 assert stats['template_default_demands']==expected,stats
 rows.append(dict(name=name,source=source,log=str(log),exit_code=0));print(name,'PASS',flush=True)
(W/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
