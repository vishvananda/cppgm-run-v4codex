#!/usr/bin/env python3
"""PA14 fixed initialization obligations and per-use object/lifetime controls.

N3485 [dcl.init]/16-17, [dcl.init.ref]/5, [dcl.init.list]/3,7,
[dcl.init.aggr]/2, [dcl.init.string]/2, [class.dtor]/11, [stmt.return]/2,
[temp.res]/8. PA14 requires definition-time checks for supported unused bodies.
No reference output or comparison rule is changed.
"""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
WORK=Path(sys.argv[2]) if len(sys.argv)>2 else Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa14-final-audit/initializer-controls'
WORK.mkdir(parents=True,exist_ok=True)
invalid={
'pointer':'int* p=42;',
'reference':'int& r=2;',
'narrow':'int i{3.4};',
'excess':'int i{1,2};',
'array-excess':'int a[2]={1,2,3};',
'array-pointer':'int* a[2]={0,42};',
'array-narrow':'int a[2]={0,3.4};',
'aggregate-narrow':'struct A{int n;};A a{3.4};',
'string-excess':'char s[2]="abc";',
'aggregate-pointer':'struct A{int* p;};A a{42};',
'ctor-deleted':'D a(1);',
'ctor-private':'P a(1);',
'ctor-explicit':'E a=1;',
'ctor-narrow':'E a{3.4};',
'dtor-deleted':'Z a(1);',
'query-ctor':'D a{F()()};',
'query-narrow':'char a{F()()};',
'function-pointer':'int (*p)(int)=g;',
'incomplete':'struct A;A a{};',
}
prefix='struct D{D(int)=delete;};struct P{private:P(int);};struct E{explicit E(int){}};struct Z{Z(int){}~Z()=delete;};struct F{int operator()()const{return 3;}};void g(int*);void g(char*);'
checks=0
records=[]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def run(name,source,reject=True):
 global checks
 path=WORK/(name+'.cpp');path.write_text(source)
 ir=WORK/(name+'.lowir')
 p=subprocess.run([str(BINARY),'--emit-lowir','-O0','-o',str(ir),str(path),'--validate-lowir'],capture_output=True,text=True)
 (WORK/(name+'.log')).write_text(p.stdout+p.stderr)
 row=dict(name=name,source_path=str(path),source_sha256=sha(path),reject=reject,exit_code=p.returncode,
  log=str(WORK/(name+'.log')),log_sha256=sha(WORK/(name+'.log')))
 assert (p.returncode!=0)==reject,(name,p.returncode,p.stderr)
 if not reject:
  exe=WORK/name
  p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
  assert p.returncode==0,(name,p.stderr)
  p=subprocess.run([str(exe)],timeout=10)
  assert p.returncode==0,(name,p.returncode)
  row['native']=dict(path=str(exe),sha256=sha(exe),exit_code=p.returncode)
 records.append(row)
 checks+=1;print(name,'PASS',flush=True)
for name,body in invalid.items():
 for style in ('function','member','qualified'):
  template={'function':'template<class T>void f(){'+body+'}',
   'member':'template<class T>struct C{void f(){'+body+'}};',
   'qualified':'template<class T>struct C{void f();};template<class T>void C<T>::f(){'+body+'}'}[style]
  run(style+'-'+name,prefix+template+'int main(){}')
for name,decl in {'pointer':'int* p=42;','reference':'int& p=42;','narrow':'int p{3.4};','array':'int a[2]={1,2,3};','ctor':'D d{1};','explicit':'E d=1;','dtor':'Z z{1};'}.items():
 run('field-'+name,prefix+'template<class T>struct C{'+decl+'};int main(){}')
for name,result,body in [('pointer','int*','{42}'),('reference','int&','{42}'),('narrow','int','{3.4}'),('explicit','E','{1}'),('deleted','D','{1}'),('excess','int','{1,2}')]:
 run('return-'+name,prefix+'template<class T>'+result+' f(){return '+body+';}int main(){}')
run('implicit-query-ctor',prefix+'struct B{int select()const{return 3;}};template<class T>struct C{struct N:B{int f()const{D d{select()};return 0;}};};int main(){}')
valid={
'scalar':'template<class T>int f(){const int n=3;constexpr int k=4;char a{n};char b{k};int* p=0;int v(7);int& r{v};int x[2]={a,b};r+=x[0]+x[1];return *(&v)+(p!=0);}int main(){return f<int>()!=14||f<long>()!=14;}',
'ctor':'int count;struct C{int n;explicit C(int a,int b=2):n(a+b){++count;}~C(){--count;}};template<class T>int f(int x){C c(x);C d{x,3};return c.n+d.n+count;}int main(){int x=f<int>(4);int y=f<long>(5);return x!=15||y!=17||count!=0;}',
'copy':'int count;struct C{int n;C(int x):n(x){++count;}C(const C& c):n(c.n){++count;}~C(){--count;}};template<class T>int f(int x){C a=x;C b(a);return b.n+count;}int main(){return f<int>(5)!=7||f<long>(8)!=10||count!=0;}',
'aggregate':'struct A{int x[2];int y;};template<class T>int f(int x){A a{{x,2},3};int v[2][2]={{x,4},{5,6}};char s[4]="abc";return a.x[0]+a.y+v[0][0]+s[1];}int main(){return f<int>(7)!=115||f<long>(9)!=119;}',
'list-return':'struct A{int x;};struct C{int x;C(int n):x(n){}};template<class T>A a(int n){return {n};}template<class T>C c(int n){return {n};}template<class T>int scalar(int n){return {n};}template<class T>int& ref(int& n){return {n};}int main(){int n=3;ref<int>(n)=5;return a<int>(7).x!=7||a<long>(9).x!=9||c<int>(4).x!=4||c<long>(6).x!=6||scalar<int>(n)!=5||ref<long>(n)!=5;}',
'dependent':'struct P{int n;P(int x):n(x){}};template<class T>T f(T x){T a{x};return a;}template<class T>struct C{T x;C(T a):x(a){}};int main(){C<int> c(4);return f<int>(c.x)!=4||f<P>(P(7)).n!=7;}',
'dependent-aggregate':'struct V{int n;V()=delete;V(int a):n(a){}};struct A{int n;};template<class T>int f(){struct C{T a;V b;};C c{{3},{4}};return c.a.n+c.b.n;}int main(){return f<A>()!=7;}',
'field':'struct V{int n;V(int a):n(a){}};template<class T>struct C{int n=4;V v{3};int f(){return n+v.n;}};int main(){C<int> a;C<long> b;return a.f()!=7||b.f()!=7;}',
'conversion':'struct C{explicit operator int()const{return 4;}};template<class T>int f(){C c;int x(c);return x;}int main(){return f<int>()!=4||f<long>()!=4;}',
'query':'struct F{int operator()()const{return 3;}};struct V{int n;explicit V(int a):n(a){}};struct B{int select()const{return 4;}};template<class T>struct C{struct N:B{int f()const{V d(select());V e{F()()};return d.n+e.n;}};};int main(){C<int>::N a;C<long>::N b;return a.f()!=7||b.f()!=7;}',
'overload-pointer':'int g(int x){return x+1;}int g(char*){return 0;}template<class T>int f(){int (*p)(int)=g;return p(3);}int main(){return f<int>()!=4||f<long>()!=4;}',
}
for name,source in valid.items():run(name,source,False)
print(checks,'initialization checks PASS')
(WORK/'checks.json').write_text(json.dumps(dict(binary=dict(path=str(BINARY),sha256=sha(BINARY)),harness_sha256=sha(__file__),checks=records),indent=2)+'\n')
