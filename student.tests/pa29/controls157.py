#!/usr/bin/env python3
"""Explicit layout, retained-attribute, and member-designator controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]; cc=root/'dev/cppgm++'
work=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-157/controls').resolve();work.mkdir(parents=True,exist_ok=True)
rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90,cwd=root)
 good=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=good,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not good: print(name,p.returncode,p.stderr.decode(errors='replace')[-1400:],flush=True)
 return good
for src in sorted((root/'student.tests/pa29/controls157').glob('*.cpp')):
 obj=work/(src.stem+'.o');exe=work/src.stem
 if run(src.stem+' compile',[cc,'-std=c++14','-c',src,'-o',obj] if src.stem=='offsetof-constexpr-loop' else [cc,'-c',src,'-o',obj]):
  if run(src.stem+' link',['g++',obj,'-o',exe]):run(src.stem+' execute',[exe])
host=root/'student.tests/pa29/host157'
if run('host ABI student compile',[cc,'-c',host/'student.cpp','-o',work/'abi-student.o']):
 if run('host ABI host compile',['g++','-std=c++11','-c',host/'host.cpp','-o',work/'abi-host.o']):
  if run('host ABI link',['g++',work/'abi-student.o',work/'abi-host.o','-o',work/'abi']):run('host ABI execute',[work/'abi'])
negative=[
 'template<int N> struct A{typedef int T __attribute__((aligned(N))); T a[2];}; A<32> a;',
 'template<int N> struct A{typedef int T __attribute__((aligned(N))); T a;}; A<3> a;',
 'typedef int Over __attribute__((aligned(32))); Over x[2];',
 'typedef int Bad alignas(8);',
 'enum class Index{a}; struct A{int x[3];}; int f(){return __builtin_offsetof(A,x[Index::a]);}',
 'struct __attribute__((aligned(3))) A {}; A a;',
 'template<int N> struct __attribute__((aligned(N))) A {}; A<3> a;',
 'struct __attribute__((aligned(0))) A {}; A a;',
 'struct alignas(1) A {long x;}; A a;',
 'int x __attribute__((aligned(-8)));',
 'int n=8; struct __attribute__((aligned(n))) A {}; A a;',
 'struct A{int x:3;}; int f(){return __builtin_offsetof(A,x);}',
 'struct A{static int x;}; int f(){return __builtin_offsetof(A,x);}',
 'struct A{int x;}; int f(){return __builtin_offsetof(A,missing);}',
 'class A{int x;}; int f(){return __builtin_offsetof(A,x);}',
 'struct A{int *x;}; int f(){return __builtin_offsetof(A,x[2]);}',
 'struct A{int x[3];}; int f(){return __builtin_offsetof(A,x[1.0]);}',
 'int f(){return __builtin_offsetof(int,x);}',
 'struct A{int &x;}; int f(){return __builtin_offsetof(A,x);}',
]
for i,source in enumerate(negative):
 src=work/('bad%d.cpp'%i);src.write_text(source+'\n')
 run('reject '+source,[cc,'-c',src,'-o',work/'bad.o'],False)
(work/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
print('%d/%d explicit checks passed'%(sum(r['passed'] for r in rows),len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
