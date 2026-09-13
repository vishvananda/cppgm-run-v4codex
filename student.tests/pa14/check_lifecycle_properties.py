#!/usr/bin/env python3
"""Destructor declaration properties, compatible redeclarations and late availability."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
B,WORK=map(lambda p:Path(p).resolve(),sys.argv[1:3]);WORK.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[3]).resolve() if len(sys.argv)>3 else ROOT/'obj/dev'
san=len(sys.argv)>4 and sys.argv[4]=='sanitized'
CASES={
 'implicit-to-throwing':('struct C{~C();};C::~C()noexcept(false){}',False),
 'throwing-to-implicit':('struct C{~C()noexcept(false);};C::~C(){}',False),
 'field-to-noexcept':('struct M{~M()noexcept(false){}};struct C{M field;~C();};C::~C()noexcept{}',False),
 'noexcept-to-field':('struct M{~M()noexcept(false){}};struct C{M field;~C()noexcept;};C::~C(){}',False),
 'late-subobject-override':('struct Base{virtual~Base()noexcept{}};template<class T>struct Leaf{~Leaf()noexcept(false){}};struct Derived:Base{Leaf<int> field;};',False),
 'valid-throwing-subobject':('struct M{~M()noexcept(false){}};struct C{M field;~C()noexcept(false);};C::~C(){}',True),
 'valid-inferred-safe':('struct C{~C();};C::~C()noexcept{}',True),
 'template-implicit-to-throwing':('template<class T>struct C{~C();};template<class T>C<T>::~C()noexcept(false){} C<int> object;',False),
 'template-throwing-to-implicit':('template<class T>struct C{~C()noexcept(false);};template<class T>C<T>::~C(){} C<int> object;',False),
 'dependent-implicit-match':('struct M{~M()noexcept(false){}};template<class T>struct C{T field;~C()noexcept(false);};template<class T>C<T>::~C(){} C<M> object;',True),
 'unused-body':('struct Base{virtual~Base()noexcept{}};template<class T>struct Leaf{~Leaf()noexcept{typename T::missing x;}};struct Derived:Base{Leaf<int> field;};',True),
 'pointer-subobject':('struct Base{virtual~Base()noexcept{}};template<class T>struct Leaf{~Leaf(){typename T::missing x;}};struct Derived:Base{Leaf<int>* field;};',True),
}
def run(cmd):return subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True,timeout=180)
rows=[]
for name,(source,valid) in CASES.items():
 path=WORK/(name+'.cpp');path.write_text(source);ir=WORK/(name+'.lowir')
 p=run([B,'--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,path]);assert (p.returncode==0)==valid,(name,p.returncode,p.stderr)
 assert 'AddressSanitizer' not in p.stderr and 'runtime error:' not in p.stderr
 if name in ('unused-body','pointer-subobject'):
  stats=json.loads(p.stderr.splitlines()[0]);assert stats['semantic_member_demands']==0,stats
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 rows.append(dict(name=name,source=source,valid=valid,exit_code=p.returncode,log=str(log)))
 print(name,'PASS',flush=True)
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).stdout.split()
exe=WORK/'probe';flags=['-fsanitize=address,undefined','-fno-pie','-no-pie'] if san else []
p=run(['g++','-std=c++11','-O2','-DEXPECT_PROPERTY_COMPLETION',*flags,'-I'+str(ROOT/'dev/src'),ROOT/'student.tests/pa14/lifecycle-properties.cc',*[objects/(n+'.o') for n in names],'-o',exe]);assert p.returncode==0,p.stderr
for name,source,mode in [('forward','struct Target;struct Target{~Target(){}};','forward'),('template','template<class T>struct C{~C(){}};using Target=C<int>;','template'),('definition-failure','template<class T>struct C{typename T::missing field;};using Target=C<int>;','failure'),('cyclic-triviality','struct Target{Target field;};','failure')]:
 path=WORK/(name+'.cpp');path.write_text(source);p=run([exe,path,mode]);assert p.returncode==0,(name,p.returncode,p.stderr)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 rows.append(dict(name=name,source=source,mode=mode,exit_code=p.returncode,log=str(log)))
 print(name,'PASS',flush=True)
(WORK/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
