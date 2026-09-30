#!/usr/bin/env python3
"""Explicit PA25 GNU statement-expression semantic/control/lifetime controls."""
import json, os, pathlib, shlex, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[2]
compiler=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else root/'dev/cppgm++').resolve()
out=pathlib.Path(sys.argv[2] if len(sys.argv)>2 else '/tmp/pa25-statements'); out.mkdir(parents=True,exist_ok=True)
cases={
'scalar':('int main(){int n=2; int x=({int n=4; ++n; n*3;}); return x==15 && n==2 ? 0:1;}',0),
'void':('int main(){int n=0; ({}); ({n=3; ;}); ({int a=4;}); ({if(n) ++n;}); return n==4?0:1;}',0),
'decay':('int f(int x){return x+1;} int main(){int a[2]={3,7}; int* p=({a;}); auto g=({f;}); return p==a && g(p[1])==8?0:1;}',0),
'nested':('int main(){int x=({int a=({int b=3; b+2;}); ({int c=a*2;c+1;});}); return x==11?0:1;}',0),
'loop':('int main(){int sum=0;for(int i=0;i<8;++i){int x=({if(i==2)continue; if(i==5)break; i*2;});sum+=x;}return sum==16?0:1;}',0),
'goto':('int main(){int n=0; int x=({++n; goto done; 7;}); n+=x; done:return n==1?0:1;}',0),
'early-operand':('int calls; int f(){++calls;return 4;} int g(){return 3+({return 7;1;})+f();} int main(){return g()==7 && calls==0?0:1;}',0),
'conditional-return':('int f(int n){return n?({return 4; 1;}):({return 5; 2;});} int main(){return f(1)==4 && f(0)==5?0:1;}',0),
'loop-condition':('int main(){int n=0;for(int i=0;i<4;++i){while(({++n;break;true;})){n+=10;} n+=100;}return n==1?0:1;}',0),
'loop-step':('int main(){int n=0;for(int i=0;i<4;++i){for(int j=0;j<2;({++n;break;})){++j;}n+=100;}return n==1?0:1;}',0),
'loop-init':('int main(){int n=0;for(int i=0;i<4;++i){for(({++n;continue;});false;){++n;} n+=100;}return n==4?0:1;}',0),
'labels':('int main(){return ({int n=0;again: ++n;if(n<3)goto again;n;})==3?0:1;}',0),
'switch':('int main(){int n=0;switch(3){case 3:n=({switch(2){case 2:n=7;break;}n+1;});break;default:return 1;}return n==8?0:1;}',0),
'template':('template<class T> T f(T n){return ({T k=n+2; k*3;});} int main(){return f(2)==12 && f(3L)==15?0:1;}',0),
'template-nested':('template<int N> int f(int x){return ({int k=({x+N;}); if(k<0)return 8; k;});} int main(){return f<3>(2)==5 && f<3>(-5)==8?0:1;}',0),
'auto-result':('int main(){auto x=({int a=3;a+1;});decltype(({2L;})) y=5;return sizeof(x)==4 && sizeof(y)==8 && x+y==9?0:1;}',0),
'volatile':('int main(){volatile int x=3;int n=({x;});return n==3?0:1;}',0),
'noexcept':('int f() noexcept{return 3;} int g(){return 4;} int main(){static_assert(noexcept(({f();2;})), "no throw");static_assert(!noexcept(({g();2;})), "throw");return 0;}',0),
'lambda-return':('int main(){auto f=[](){return ({return 7;0;});};return f()==7?0:1;}',0),
'class-result':('struct A{int x;A(int n) noexcept:x(n){} int value()const{return x;} bool same(const A* p)const{return this==p;}};int main(){A a(3);return ({a;}).value()==3 && !({a;}).same(&a)?0:1;}',0),
'copy-local':('int copies,dead;struct A{int x;A(int n) noexcept:x(n){} A(const A& a) noexcept:x(a.x){++copies;} ~A() noexcept{++dead;}};int main(){A x=({A a(3);a;}); return x.x==3 && copies==1 && dead==1?0:1;}',0),
'inner-temporary':('int dead;struct A{A() noexcept{} ~A() noexcept{++dead;}};int main(){int x=({A();dead;});return x==1 && dead==1?0:1;}',0),
'outer-temporary':('int dead;struct A{A() noexcept{} ~A() noexcept{++dead;}};int main(){int x=(A(),({A();dead;}));return x==1 && dead==2?0:1;}',0),
'outer-local':('int dead;struct A{A() noexcept{} ~A() noexcept{++dead;}};int main(){A a;int x=({A b;3;});return x==3 && dead==1?0:1;}',0),
'early-cleanup':('int log;struct A{int n;A(int x) noexcept:n(x){} ~A() noexcept{log=log*10+n;}};int f(){A a(1);return (A(2),({A b(3);return 7;0;}));}int main(){int n=f();return n==7 && log==321?0:1;}',0),
'nested-cleanup':('int log;struct A{int n;A(int x) noexcept:n(x){} ~A() noexcept{log=log*10+n;}};int f(){A a(1);return (A(2),({A b(3);(A(4),({A c(5);return 7;0;}));0;}));}int main(){int n=f();return n==7 && log==54321?0:1;}',0),
'break-cleanup':('int dead;struct A{A() noexcept{} ~A() noexcept{++dead;}};int main(){for(int i=0;i<2;++i){(A(),({A b;break;}));}return dead==2?0:1;}',0),
'goto-cleanup':('int dead;struct A{A() noexcept{} ~A() noexcept{++dead;}};int main(){(A(),({A b;goto done;}));done:return dead==2?0:1;}',0),
'goto-skipped-class':('int made,dead;struct A{A() noexcept{++made;}~A() noexcept{++dead;}};int f(int n){A a=({if(n)goto done;A();});done:return 3;}int main(){f(1);if(made||dead)return 1;f(0);return made==1 && dead==1?0:1;}',0),
'noexcept-local-ctor':('struct A{A();~A() noexcept;};int main(){static_assert(!noexcept(({A a;2;})), "ctor effect");return 0;}',0),
'noexcept-local-dtor':('struct A{A() noexcept;~A() noexcept(false);};int main(){static_assert(!noexcept(({A a;2;})), "dtor effect");return 0;}',0),
'statement-type-query':('template<class T> T f(T x){decltype(({T a=x;a;})) y=x;return y;}int main(){return f(3)==3 && f(4L)==4?0:1;}',0),
'statement-query-operator':('template<class T> int f(T x){decltype(({T a=x;a+a;})) y=x+x;return y;}int main(){return f(3)==6 && f(4L)==8?0:1;}',0),
'noexcept-template':('template<class T> int f(){static_assert(noexcept(({T a;2;})), "local effects");return 0;}int main(){return f<int>();}',0),
'reference-result':('int made,dead;struct A{int n;A(int x) noexcept:n(x){++made;}~A() noexcept{++dead;}};int main(){const A& a=({A(4);});return a.n==4 && made==1 && dead==0?0:1;}',0),
'goto-skipped-reference':('int made,dead;struct A{A() noexcept{++made;}~A() noexcept{++dead;}};int f(int n){const A& a=({if(n)goto done;A();});done:return 3;}int main(){f(1);if(made||dead)return 1;f(0);return made==1 && dead==1?0:1;}',0),
'bad-scope':('int main(){int n=({int x=3;x;});return x;}',None),
'bad-lvalue':('int main(){int x=1;({x;})=4;return 0;}',None),
'bad-void':('int main(){int n=({int x=3;});return n;}',None),
'bad-global':('int x=({3;});int main(){return x;}',None),
'bad-constexpr':('int main(){constexpr int x=({3;});return x;}',None),
'bad-goto-in':('int main(){goto inside;int n=({inside:; 3;});return n;}',None),
'bad-case-in':('int main(){switch(1){int n=({case 1:;3;});}return 0;}',None),
'bad-loop-condition':('int main(){while(({break;true;})){}return 0;}',None),
'bad-template-lookup':('template<class T> int f(){return ({missing();3;});}int main(){return 0;}',None),
}
results=[]
for name,(source,expected) in cases.items():
    src=out/(name+'.cc');src.write_text(source+'\n');exe=out/(name+'.program')
    c=subprocess.run([str(compiler),*shlex.split(os.environ.get('CPPGM_CONTROL_FLAGS','')),'-o',str(exe),str(src)],capture_output=True,text=True)
    actual=None; message=c.stderr.strip()
    if c.returncode==0:
        try:
            r=subprocess.run([str(exe)],capture_output=True,text=True,timeout=3);actual=r.returncode
        except subprocess.TimeoutExpired:actual='timeout'
    ok=(c.returncode>0 if expected is None else c.returncode==0 and actual==expected)
    row=dict(name=name,passed=ok,compile=c.returncode,actual=actual,expected=expected,diagnostic=message);results.append(row)
    print(('PASS' if ok else 'FAIL'),name, '' if ok else row,flush=True)
# Shared template and class lifetimes across independent translation units.
header=out/'boundary.h'
header.write_text("struct Trace{int n;Trace(int) noexcept;~Trace() noexcept;};extern int log;int run(int);\n"
                  "template<class T>T bump(T x){return ({T n=x+1;n;});}\n")
sources=[out/'boundary-a.cc',out/'boundary-b.cc']
sources[0].write_text('#include "boundary.h"\nint run(int n){Trace a(1);return ({Trace b(2);if(n)return bump(6);bump(2);});}\n')
sources[1].write_text('#include "boundary.h"\nint log;Trace::Trace(int x) noexcept:n(x){}Trace::~Trace() noexcept{log=log*10+n;}int main(){int n=run(1);if(n!=7||log!=21)return 1;log=0;return run(0)==bump(2) && log==21?0:1;}\n')
objects=[out/'boundary-a.obj',out/'boundary-b.obj']
def command(args,env=None):
    r=subprocess.run(list(map(str,args)),capture_output=True,text=True,env=env)
    return r.returncode,r.stderr.strip()
for src,obj in zip(sources,objects):
    code,message=command([compiler,'-c','-o',obj,src])
    if code: raise RuntimeError((src,code,message))
for label,inputs in [('separate',objects),('direct',sources),('mixed-a',[objects[0],sources[1]]),('mixed-b',[sources[0],objects[1]])]:
    exe=out/('boundary-'+label+'.program');env=dict(os.environ);env['PATH']='/no-external-tools'
    code,message=command([compiler,'-o',exe,*inputs],env)
    actual=command([exe])[0] if code==0 else None
    ok=code==0 and actual==0
    results.append(dict(name='boundary-'+label,passed=ok,compile=code,actual=actual,diagnostic=message))
    print(('PASS' if ok else 'FAIL'),'boundary-'+label,message,flush=True)
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print(f'{sum(x["passed"] for x in results)}/{len(results)}')
sys.exit(any(not x['passed'] for x in results))
