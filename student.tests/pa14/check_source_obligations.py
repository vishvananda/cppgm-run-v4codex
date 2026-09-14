#!/usr/bin/env python3
"""Source defaults and evaluated-expression recipes; N3485 [dcl.init]/7,9,12,
[class.ctor]/5, [expr.call]/11, [class.temporary]/1, [temp.res]/8.
The course accepts initialized/empty const classes consistently with CWG253.
No fixture/reference changes.
"""
from pathlib import Path
import hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BINARY=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
WORK=Path(sys.argv[2]) if len(sys.argv)>2 else Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa14-final-audit/source-controls'
WORK.mkdir(parents=True,exist_ok=True)
rows=[]
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def run(name,source,reject,backend_limit=False):
 assert all(r['name']!=name for r in rows),name
 path=WORK/(name+'.cpp');path.write_text(source)
 ir=WORK/(name+'.lowir');log=WORK/(name+'.log')
 p=subprocess.run([str(BINARY),'--emit-lowir','-O0','-o',str(ir),str(path),'--validate-lowir'],capture_output=True,text=True)
 log.write_text(p.stdout+p.stderr)
 assert p.returncode==int(reject),(name,p.returncode,p.stderr)
 assert 'Sanitizer' not in p.stderr and 'runtime error:' not in p.stderr,(name,p.stderr)
 row=dict(name=name,source_path=str(path),source_sha256=sha(path),reject=reject,exit_code=p.returncode,log=str(log),log_sha256=sha(log))
 if not reject:
  exe=WORK/name
  p=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
  native_log=WORK/(name+'-native.log');native_log.write_text(p.stdout+p.stderr)
  if backend_limit:
   assert p.returncode==1 and 'undefined native symbol: pure_virtual' in p.stderr,(name,p.stderr)
   row['backend_limitation']=dict(exit_code=p.returncode,log=str(native_log),log_sha256=sha(native_log))
  else:
   assert p.returncode==0,(name,p.stderr)
   p=subprocess.run([str(exe)],timeout=10)
   assert p.returncode==0,(name,p.returncode)
   row['native']=dict(path=str(exe),sha256=sha(exe),exit_code=p.returncode)
 rows.append(row);print(name,'PASS',flush=True)
invalid = {
 'reference': ('', 'int& r;'),
 'rvalue-reference': ('', 'int&& r;'),
 'const-scalar': ('', 'const int x;'),
 'const-array': ('', 'const int x[2];'),
 'const-indeterminate': ('struct D{int n;};', 'const D d;'),
 'deleted-default': ('struct D{D()=delete;};', 'D d;'),
 'private-default': ('class D{D(){}};', 'D d;'),
 'ambiguous-default': ('struct D{D(int=1){}D(double=2){}};', 'D d;'),
 'array-default': ('struct D{D()=delete;};', 'D d[2];'),
 'implicit-reference': ('struct D{int& r;};', 'D d;'),
 'implicit-const': ('struct D{const int n;};', 'D d;'),
 'base-default': ('struct B{B()=delete;};struct D:B{};', 'D d;'),
 'member-default': ('struct B{B()=delete;};struct D{B b;};', 'D d;'),
 'deleted-dtor': ('struct D{D(){}~D()=delete;};', 'D d;'),
 'deleted-call': ('struct C{int operator()()=delete;};', 'C c;c();'),
 'deleted-call-init': ('struct C{int operator()()=delete;};struct D{D(int){}};', 'C c;D d(c());'),
 'deleted-construction': ('struct C{C()=delete;};', 'C();'),
 'deleted-operator': ('struct C{int operator+(int)=delete;};', 'C c;c+1;'),
 'deleted-argument': ('struct A{A(){}A(const A&)=delete;};struct C{int operator()(A){return 0;}};', 'A a;C c;c(a);'),
 'deleted-conversion': ('struct C{operator int()=delete;};', 'C c;c+1;'),
 'type-query-default': ('struct C{C()=delete;};', 'using X=decltype(C());'),
 'type-query-call': ('struct C{int operator()()=delete;};', 'C c;using X=decltype(c());'),
 'type-query-conversion': ('struct C{C(){}~C()=delete;};', 'using X=decltype(C());'),
 'local-reference': ('', 'struct D{int& r;};D d;'),
 'local-private': ('', 'class D{D(){}};D d;'),
 'local-deleted': ('', 'struct D{D()=delete;};D d;'),
 'local-no-default': ('', 'struct D{D(int){}};D d;'),
 'local-ambiguous': ('', 'struct D{D(int=1){}D(double=2){}};D d;'),
 'local-dtor': ('', 'struct D{~D()=delete;};D d;'),
 'local-const-field': ('', 'struct D{int n;};D a;const D b;'),
 'local-pure': ('', 'struct D{virtual int g()=0;};D d;'),
 'local-deleted-call': ('', 'struct D{int operator()()=delete;};D d;d();'),
 'local-deleted-method': ('', 'struct D{void g()=delete;};D d;d.g();'),
 'local-default-argument': ('', 'struct D{D(int* p=1){}};D d;'),
 'deleted-static-cast': ('struct C{operator int*()=delete;};', 'C c;static_cast<int*>(c);'),
 'deleted-functional-cast': ('struct C{operator int*()=delete;};using P=int*;', 'C c;(P(c));'),
 'duplicate-variable': ('', 'int x;int x;'),
 'cast-declaration': ('struct C{};using P=int*;', 'C c;P(c);'),
 'implicit-deleted-dtor': ('struct C{~C()=delete;};struct B{C c;B();};', 'B b;'),
 'conditional-deleted-bool': ('struct C{operator bool()=delete;};', 'C c;c?1:2;'),
 'deleted-keyword-cast': ('struct C{operator int()=delete;};', 'C c;(int(c));'),
 'deleted-class-cast': ('struct C{C(int)=delete;};', 'static_cast<C>(3);'),
 'deleted-cstyle-class-cast': ('struct C{C(int)=delete;};', '(C)3;'),
 'conditional-deleted-conversion': ('struct C{operator int()=delete;};', 'C c;true?c:3;'),
 'conditional-both-directions': ('struct C{C(int){}operator int(){return 3;}};', 'C c(1);true?c:3;'),
 'conditional-ambiguous-match': ('struct C{operator int(){return 1;}operator double(){return 2;}};', 'C c;short n=3;true?c:n;'),
 'braced-constructor': ('struct C{C(int)=delete;};', 'C{3};'),
 'braced-narrowing': ('struct C{C(int){}};', 'C{3.5};'),
 'braced-aggregate': ('struct C{int* p;};', 'C{42};'),
 'braced-scalar': ('', 'int{3.5};'),
}
for name,(prefix,body) in invalid.items():
 for style in ('unused','used','member','qualified'):
  tail={'unused':'template<class T>void f(){'+body+'}int main(){}',
   'used':'template<class T>void f(){'+body+'}int main(){f<int>();}',
   'member':'template<class T>struct V{void f(){'+body+'}};int main(){}',
   'qualified':'template<class T>struct V{void f();};template<class T>void V<T>::f(){'+body+'}int main(){}'}[style]
  run(style+'-'+name,prefix+tail,True)
run('invalid-list-return','struct C{int operator()(){return 42;}};template<class T>int* f(){return {C()()};}int main(){}',True)
valid = {
 'call': ('int live;struct C{C(){++live;}~C(){--live;}int operator()(){return 42;}};', 'return C()();', 'f<int>()!=42||f<long>()!=42||live'),
 'builtin': ('int live;struct C{C(){++live;}~C(){--live;}operator int(){return 2;}};', 'C c;return c+3;', 'f<int>()!=5||f<long>()!=5||live'),
 'surrogate': ('int live;int g(int x){return x+2;}using F=int(*)(int);struct C{C(){++live;}~C(){--live;}operator F(){return g;}};', 'C c;return c(3);', 'f<int>()!=5||f<long>()!=5||live'),
 'prvalue-surrogate': ('int live;int g(int x){return x+2;}using F=int(*)(int);struct C{C(){++live;}~C(){--live;}operator F(){return g;}};', 'return C()(3);', 'f<int>()!=5||f<long>()!=5||live'),
 'list-return': ('struct C{int operator()(){return 42;}};', 'return {C()()};', 'f<int>()!=42||f<long>()!=42'),
 'operator-default': ('struct C{int operator()(int x=42){return x;}};', 'C c;return c();', 'f<int>()!=42||f<long>()!=42'),
 'operator-default-cleanup': ('int live;struct A{A(){++live;}~A(){--live;}operator int(){return 3;}};struct C{int operator()(int x=A()){return x+live;}};', 'C c;return c();', 'f<int>()!=4||f<long>()!=4||live'),
 'constructor-default-cleanup': ('int live;struct C{C(){++live;}~C(){--live;}operator int(){return 3;}};struct D{int n;D(int v=C()):n(v){}};', 'D a,b;return a.n+b.n+live;', 'f<int>()!=6||f<long>()!=6||live'),
 'const-provided': ('struct D{D(){}};', 'const D a;return 0;', 'f<int>()||f<long>()'),
 'const-empty': ('struct D{};', 'const D a;return 0;', 'f<int>()||f<long>()'),
 'const-initialized': ('struct D{int n=3;};', 'const D a;return a.n;', 'f<int>()!=3||f<long>()!=3'),
 'condition': ('struct C{explicit operator bool(){return true;}};', 'C c;if(c)return 3;return 0;', 'f<int>()!=3||f<long>()!=3'),
 'condition-declaration': ('struct C{C(int){}explicit operator bool(){return true;}};', 'if(C c=1)return 3;return 0;', 'f<int>()!=3||f<long>()!=3'),
 'switch': ('struct C{operator int(){return 2;}};', 'C c;switch(c){case 2:return 3;}return 0;', 'f<int>()!=3||f<long>()!=3'),
 'local-initialized': ('', 'int n=3;struct D{int& n;D(int& x):n(x){}};D d(n);return d.n;', 'f<int>()!=3||f<long>()!=3'),
 'const-mutable': ('struct C{mutable int n;};', 'const C c;return 0;', 'f<int>()||f<long>()'),
 'local-abstract-base': ('struct B{virtual int g()=0;B(){}};', 'struct D:B{int g(){return 3;}};D d;return d.g();', 'f<int>()!=3||f<long>()!=3'),
 'local-concrete-base': ('struct B{virtual int g(){return 0;}B(){}};', 'struct D:B{int g(){return 3;}};D d;B& b=d;return b.g();', 'f<int>()!=3||f<long>()!=3'),
 'static-cast': ('int calls;struct C{explicit operator int(){++calls;return 3;}};', 'C c;return static_cast<int>(c);', 'f<int>()!=3||f<long>()!=3||calls!=2'),
 'functional-cast': ('int calls;using R=int;struct C{explicit operator int(){++calls;return 3;}};', 'C c;return R(c);', 'f<int>()!=3||f<long>()!=3||calls!=2'),
 'conditional': ('int live;struct C{C(){++live;}~C(){--live;}operator int(){return 3;}};', 'C c;return true?c:3;', 'f<int>()!=3||f<long>()!=3||live'),
 'keyword-cast': ('int calls;struct C{explicit operator int(){++calls;return 3;}};', 'C c;return int(c);', 'f<int>()!=3||f<long>()!=3||calls!=2'),
 'class-cast': ('int live,calls;struct C{int n;C(int x):n(x){++calls;++live;}~C(){--live;}};', 'return static_cast<C>(3).n;', 'f<int>()!=3||f<long>()!=3||calls!=2||live'),
 'braced-construction': ('int live,calls;struct C{int n;C(int x):n(x){++calls;++live;}~C(){--live;}};', 'return C{3}.n;', 'f<int>()!=3||f<long>()!=3||calls!=2||live'),
 'braced-aggregate': ('int live;struct C{int n;~C(){--live;}};', '++live;return C{3}.n;', 'f<int>()!=3||f<long>()!=3||live'),
 'cstyle-class-cast': ('int live,calls;struct C{int n;C(int x):n(x){++calls;++live;}~C(){--live;}};', 'return ((C)3).n;', 'f<int>()!=3||f<long>()!=3||calls!=2||live'),
 'conditional-ref': ('int n=3,calls;struct C{operator int&(){++calls;return n;}};', 'C c;int& r=true?c:n;r+=1;return r;', 'f<int>()!=4||f<long>()!=5||calls!=2'),
 'conditional-distinct': ('int calls;struct C{operator int(){++calls;return 3;}};struct D{operator double(){++calls;return 4;}};', 'C c;D d;return int(true?c:d);', 'f<int>()!=3||f<long>()!=3||calls!=2'),
 'local-base': ('', 'struct B{int n=3;};struct D:B{};D d;return d.n;', 'f<int>()!=3||f<long>()!=3'),
 'local-dependent-base': ('struct B{int n=3;};', 'struct D:T{};D d;return d.n;', 'f<B>()!=3||f<B>()!=3'),
 'local-dependent': ('', 'struct D{T n;};D d;d.n=3;return d.n;', 'f<int>()!=3||f<long>()!=3'),
}
for name,(prefix,body,result) in valid.items():
 run(name,prefix+'template<class T>int f(){'+body+'}int main(){return '+result+';}',False,name=='local-abstract-base')
run('decltype-result','struct C{~C()=delete;};C g();template<class T>void f(){using X=decltype(g());}int main(){f<int>();}',False)
run('decltype-incomplete','struct C;C g();template<class T>void f(){using X=decltype(g());}int main(){f<int>();}',False)
run('unused-dependent-body','template<class T>struct D{D(){typename T::Missing a;}};template<class T>void f(){D<int> d;}int main(){}',False)
run('extern','struct D;template<class T>void f(){extern D d;extern int& r;}int main(){f<int>();}',False)
run('alias','struct D{D()=delete;};typedef D X;int main(){}',False)
run('extern-identity','namespace N{int value=7;template<class T>int f(){extern int value;return value;}}int main(){return N::f<int>()!=7||N::f<long>()!=7;}',False)
run('extern-before-definition','template<class T>int f(){extern int value;return value;}int main(){return f<int>()!=7||f<long>()!=7;}int value=7;',False)
run('standalone-construction','int calls;struct C{C(){++calls;}C(int n){calls+=n;}};template<class T>void f(){C();C(2);for(C();false;){} }int main(){f<int>();f<long>();return calls!=8;}',False)
run('dependent-initializer-fixed-pointer','int value;template<class T>int* g(){return &value;}template<class T>void f(){int* p=g<T>();++*p;}int main(){f<int>();f<long>();return value!=2;}',False)
run('local-forward-definition','template<class T>int f(){struct C;struct C{int n=3;};struct C;C c;return c.n;}int main(){return f<int>()!=3||f<long>()!=3;}',False)
print(len(rows),'source obligation checks PASS')
for row in rows:
 assert sha(row['source_path'])==row['source_sha256'],row['name']
 assert sha(row['log'])==row['log_sha256'],row['name']
(WORK/'checks.json').write_text(json.dumps(dict(binary=dict(path=str(BINARY),sha256=sha(BINARY)),harness_sha256=sha(__file__),checks=rows),indent=2)+'\n')
