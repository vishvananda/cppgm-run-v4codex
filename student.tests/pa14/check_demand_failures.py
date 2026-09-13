#!/usr/bin/env python3
"""Terminal definition/layout demands; public API repeated-request controls."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
objects=Path(sys.argv[2]).resolve() if len(sys.argv)>2 else ROOT/'obj/dev'
mode=sys.argv[3] if len(sys.argv)>3 else 'current'
CASES={
 'function_body':('template<class T>int f(){return T::missing;} int main(){return f<int>();}', 'finish'),
 'member_body':('template<class T>struct C{int f(){return T::missing;}}; int main(){C<int> c;return c.f();}', 'finish'),
 'class_definition':('template<class T>struct C{using value=typename T::missing;};using Target=C<int>;struct Good{int value;};', 'size'),
 'void_field':('template<class T>struct C{T field;};using Target=C<void>;struct Good{int value;};','size'),
 'class_layout':('template<class T>struct C{alignas(1) T field;};using Target=C<long>;struct Good{int value;};','size'),
 'recursive_layout':('template<class T>struct C{C field;};using Target=C<int>;struct Good{int value;};','size'),
 'nested_definition':('template<class T>struct Outer{struct Inner;};template<class U>struct Outer<U>::Inner{typename U::missing field;};using Target=Outer<int>::Inner;struct Good{int value;};','size'),
 'direct_layout':('struct Target{Target field;};struct Good{int value;};','layout'),
 'nested_late_definition':('template<class T>struct Outer{struct Inner;};template<class U>struct alignas(1) Outer<U>::Inner{U field;};using Target=Outer<long>::Inner;struct Good{int value;};','definition'),
}
def run(command):
 p=subprocess.run(list(map(str,command)),cwd=ROOT,capture_output=True,text=True,timeout=180)
 assert p.returncode==0,(command,p.returncode,p.stdout,p.stderr)
 return p.stdout
names=run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
exe=WORK/'probe'
flags=[] if mode=='entry' else ['-DEXPECT_TERMINAL_FACTS']
if mode=='sanitized': flags += ['-fsanitize=address,undefined','-fno-pie','-no-pie']
run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),ROOT/'student.tests/pa14/demand-failures.cc',*[objects/(n+'.o') for n in names],'-o',exe])
records=[]
for name,(source,kind) in CASES.items():
 path=WORK/(name+'.cpp');path.write_text(source)
 output=run([exe,path,kind]);log=WORK/(name+'.log');log.write_text(output)
 records.append(dict(name=name,kind=kind,source=source,log=str(log),exit_code=0))
 print(name,'PASS',flush=True)
(WORK/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
