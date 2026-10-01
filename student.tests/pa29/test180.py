#!/usr/bin/env python3
"""Callable extensions: explicit controls, runtime, rejection and typed views."""
import pathlib,subprocess,sys,json,hashlib
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2] if len(sys.argv)>2 else root/'dev/cppgm++').resolve()
cases={
# Source attribute expansion must retain calls, branches and unwind semantics.
'inline-basic':('__attribute__((always_inline)) inline int f(int x){return x+1;} int main(){return f(5)!=6;}',True),
'inline-branch':('__attribute__((always_inline)) inline int f(int x){if(x<0)return -x;return x+1;} int main(){return f(-3)!=3||f(4)!=5;}',True),
'inline-loop':('__attribute__((always_inline)) inline int f(int x){int s=0;for(int i=0;i<x;++i)s+=i;return s;} int main(){return f(8)!=28||f(3)!=3;}',True),
'inline-nested':('__attribute__((always_inline)) inline int f(int x){return x+1;} __attribute__((always_inline)) inline int g(int x){return f(x)*2;} int main(){return g(4)!=10;}',True),
'inline-effects':('int n; __attribute__((always_inline)) inline int f(int x){return x+x;} int next(){return ++n;} int main(){return f(next())!=2||n!=1;}',True),
'inline-reference':('__attribute__((always_inline)) inline int& f(int& x){return x;} int main(){int n=3;f(n)=8;return n!=8;}',True),
'inline-float':('__attribute__((always_inline)) inline double f(double x){return x*.5+1.;} int main(){return f(6.)!=4.;}',True),
'inline-object':('struct P{long x,y;}; __attribute__((always_inline)) inline P f(P p){p.x+=p.y;return p;} int main(){P p={3,4};P q=f(p);return p.x!=3||q.x!=7||q.y!=4;}',True),
'inline-dtor':('int n;struct G{~G(){++n;}}; __attribute__((always_inline)) inline int f(int x){G g;return x+1;} int main(){int x=f(4);return x!=5||n!=1;}',True),
'inline-throw':('int n;struct G{~G(){++n;}}; __attribute__((always_inline)) inline void f(int x){G g;throw x;} int main(){try{f(4);}catch(int x){return x!=4||n!=1;}return 2;}',True),
'inline-catch':('__attribute__((always_inline)) inline int f(int x){try{throw x;}catch(int y){return y+1;}} int main(){return f(4)!=5;}',True),
'inline-noreturn':('__attribute__((always_inline,noreturn)) inline void f(){for(;;){}} int g(int x){if(x<0)f();return x*2;} int main(){return g(4)!=8;}',True),
'inline-static-local':('__attribute__((always_inline)) inline int f(){static int n=0;return ++n;} int main(){return f()!=1||f()!=2;}',True),
'inline-static-operator':('struct F{__attribute__((always_inline)) static int operator()(int x){return x*3;}}; int main(){return F()(4)!=12;}',True),
 'inline-long-double':('__attribute__((always_inline)) inline long double f(long double x){return x*.5L+1.L;} int main(){return f(6.L)!=4.L;}',True),
'inline-atomic':('int n; __attribute__((always_inline)) inline int f(){return __atomic_add_fetch(&n,1,5);} int main(){return f()!=1||f()!=2;}',True),
'inline-void':('int n; __attribute__((always_inline)) inline void f(){++n;} int main(){f();f();return n!=2;}',True),
'inline-rethrow':('int n;struct G{~G(){++n;}}; __attribute__((always_inline)) inline void f(){try{G g;throw 7;}catch(int){G h;throw;}} int main(){try{f();}catch(int x){return x!=7||n!=2;}return 3;}',True),
'static-dependent-query':('struct F { static constexpr int operator()(int x=5) noexcept {return x+2;} }; template<class T> constexpr auto call(T t)->decltype(t()){return t();} static_assert(call(F())==7,"value"); int main(){return call(F())!=7;}',True),
'static-noexcept-receiver':('struct F {F() noexcept(false){} static int operator()() noexcept{return 1;} }; static_assert(!noexcept(F()()),"receiver throws"); int main(){return F()()!=1;}',True),
'static-reference-qualifier':('struct F {static int operator()() & {return 1;} }; int main(){return 0;}',False),
'static-call':('struct F { static int operator()(int x){return x+1;} }; int main(){return F()(6)!=7;}',True),
'static-subscript':('struct F { static int operator[](int x){return x+1;} }; int main(){return F()[6]!=7;}',True),
'static-template':('struct F { template<class T> static T operator()(T x){return x+1;} }; template<class T> int call(T t){return t(8);} int main(){return call(F())!=9;}',True),
'static-default':('struct F { static int operator()(int x=9){return x;} }; int main(){return F()()!=9;}',True),
'static-effects':('int made,dead; struct F { F(){++made;} ~F(){++dead;} static int operator()(int x){return x+made;} }; int main(){int x=F()(8);return x!=9||made!=1||dead!=1;}',True),
'static-query':('struct F { static constexpr int operator()(int x=5) noexcept {return x+2;} }; template<class T> auto call(T t)->decltype(t()){return t();} static_assert(F()()==7,"value"); static_assert(noexcept(F()(1)),"noexcept"); int main(){return call(F())!=7;}',True),
'static-fixed':('struct F { static int operator()(int x=4){return x;} }; template<class T> int call(T){return F()();} int main(){return call(0)!=4;}',True),
'static-pointer':('struct F { static int operator()(int x){return x+1;} }; int main(){int(*p)(int)=&F::operator();return p(5)!=6;}',True),
'static-inherited':('struct F { static int operator()(int x){return x+1;} }; struct G:F{}; int main(){const G g;return g(5)!=6;}',True),
'static-mixed':('struct F { static int operator()(int x){return 1;} int operator()(long) const {return 2;} }; int main(){F f;return f(5)!=1||f(5L)!=2||f.operator()(5)!=1;}',True),
'static-partial':('struct F { template<class T> static int operator()(T*){return 1;} template<class T> static int operator()(T){return 2;} }; int main(){int x;return F()(&x)!=1||F()(x)!=2;}',True),
'static-reference':('struct F { static int& operator()(int& x){return x;} }; int main(){int x=3;F()(x)=7;return x!=7;}',True),
'static-private':('struct F { private: static int operator()(int){return 1;} }; int main(){return F()(1);}',False),
'static-deleted':('struct F { static int operator()(int)=delete; }; int main(){return F()(1);}',False),
'static-wrong-arity':('struct F { static int operator()(int){return 1;} }; int main(){return F()();}',False),
'static-illegal-op':('struct F { static int operator+(F){return 1;} }; int main(){return 0;}',False),
'static-ambiguous':('struct F { static int operator()(int,long){return 1;} int operator()(long,int)const{return 2;} }; int main(){return F()(1,1);}',False),
'static-this':('struct F { static F* operator()(){return this;} }; int main(){return 0;}',False),
'static-qualifier':('struct F { static int operator()() const{return 1;} }; int main(){return 0;}',False),
}
results=[]
def command(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 return dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr)
for name,(source,ok) in cases.items():
 src=out/(name+'.cpp');src.write_text(source+'\n'); row=dict(name=name,accepted=ok,sha256=hashlib.sha256(src.read_bytes()).hexdigest(),source=source,checks=[])
 for label,binary,flags in [('student',compiler,['-std=c++11']),('clang','clang++',['-std=c++11','-Wno-c++23-extensions'])]:
  obj=out/(name+label+'.o');exe=out/(name+label)
  r=command([binary,*flags,'-c',src,'-o',obj]);row['checks'].append(r)
  if (r['status']==0)!=ok:row['failure']=label+' acceptance'
  if ok and not r['status']:
   r=command(['g++',obj,'-o',exe]);row['checks'].append(r)
   if r['status']:row['failure']=label+' link'
   else:
    r=command([exe]);row['checks'].append(r)
    if r['status']:row['failure']=label+' runtime'
 if ok:
  r=command([compiler,'-c','--emit-lowir','--validate-lowir',src,'-o',out/(name+'.lowir')]);row['checks'].append(r)
  if r['status']:row['failure']='LowIR audit'
 results.append(row);(out/'controls.json').write_text(json.dumps(results,indent=2)+'\n');print(name,row.get('failure','pass'),flush=True)
assert not [r for r in results if 'failure' in r]
