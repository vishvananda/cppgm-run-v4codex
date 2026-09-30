#!/usr/bin/env python3
"""Explicit reduced GNU/header/template controls, independent of course fixtures."""
import json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
records=[]
def run(args,okay=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 records.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr))
 (out/'controls.json').write_text(json.dumps(records,indent=2)+'\n')
 assert (p.returncode==0)==okay,records[-1]
 return p.stdout
for name in ('one','two'): (out/name).mkdir(exist_ok=True)
(out/'one/first.h').write_text('''#define FIRST_INCLUDED 19
#if !__has_include_next(<first.h>)
#error missing next header
#endif
#include_next <first.h>
''')
(out/'two/first.h').write_text('''#define SECOND_INCLUDED 23
#if __has_include_next(<first.h>)
#error spurious next header
#endif
''')
for source in ('hosted-prerequisites','header-probe','friend-partial','extern-overload','void-member-signature','friend-redeclaration','friend-access'):
 for label,binary in [('student',compiler),('host','g++')]:
  obj=out/(source+label+'.o');exe=out/(source+label)
  run([binary,'-std=c++11','-I',out/'one','-I',out/'two','-c',root/'student.tests/pa27'/(source+'.cpp'),'-o',obj])
  run(['g++',obj,'-o',exe]);run([exe])
for binary in (compiler,'g++'):
 source=root/'student.tests/pa27/friend-access-private.cpp'
 run([binary,'-std=c++11','-c',source,'-o',out/'access.o'],False)
 run([binary,'-std=c++11','-DSFINAE_CONTROL','-c',source,'-o',out/'access.o'])
 run(['g++',out/'access.o','-o',out/'access']);run([out/'access'])
# Both builtin spellings have ordinary signatures and diagnose invalid arguments.
for name,source in [('strcmp-arity','int f(){return __builtin_strcmp("a");}'),
 ('strncmp-type','int f(){return __builtin_strncmp(7,"a",1);}'),
 ('named-variadic-tail','#define F(args..., after) args\n'),
 ('named-va-args','#define F(args...) __VA_ARGS__\n'),
 ('explicit-empty','#define F(x,args...) x, ##args\nint f(){return F(1,); }'),
 ('atomic-address','auto p=&__atomic_fetch_add;'),
 ('instantiation-ambiguous','template<class T> void f(T*,int); template<class T> void f(int*,T); extern template void f(int*,int);'),
 ('atomic-const','int f(const int* p){return __atomic_fetch_add(p,1,5);}'),
 ('atomic-bool','bool f(bool* p){return __atomic_fetch_add(p,1,5);}'),
 ('atomic-float','float f(float* p){return __atomic_fetch_add(p,1,5);}'),
 ('unrelated-friend','class S{int value;template<class T> friend struct Friend;}; template<class T> struct Other; template<class T> struct Other<T*> {int f(S s){return s.value;}};'),
 ('probe-operand','#if __has_include(7)\n#endif\n')]:
 p=out/(name+'.cpp');p.write_text(source)
 run([compiler,'-c',p,'-o',out/'bad.o'],False)
run([compiler,'-c',root/'student.tests/pa27/atomic-integers.cpp','-o',out/'atomic.o'])
run(['g++','-std=c++11','-pthread',out/'atomic.o',root/'student.tests/pa27/atomic-host.cpp','-o',out/'atomic'])
run([out/'atomic'])
run(['objdump','-dr',out/'atomic.o'])
print('PASS',len(records),'commands')
