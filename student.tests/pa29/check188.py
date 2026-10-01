#!/usr/bin/env python3
"""Explicit contextual grammar/binding controls; OUT holds generated artifacts."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
rows=[]
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 rows.append(dict(command=list(map(str,args)),status=p.returncode,expected_success=ok,stdout=p.stdout,stderr=p.stderr))
 (out/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
 assert (p.returncode==0)==ok,(args,p.returncode,p.stdout,p.stderr)
 return p
for src in sorted((root/'student.tests/pa29/controls188').glob('*.cpp')):
 obj=out/(src.stem+'.o');exe=out/src.stem
 run([compiler,'-std=c++11','-O0','-c',src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
 run([compiler,'--emit-ast','-o',out/(src.stem+'.ast'),src])
 run([compiler,'--emit-lowir','-o',out/(src.stem+'.lowir'),src])
 run([root/'dev/lowir',out/(src.stem+'.lowir'),'-o',out/(src.stem+'.canonical')])
 if src.stem in ('identifiers','throw-comma'):
  for host in ['g++','clang++']:
   run([host,'-std=c++11',src,'-o',out/'host']);run([out/'host'])
# Assert operator grouping and absence of executable coroutine/throw tails.
ast=(out/'precedence.ast').read_text()
for text in [
 'binary-expression OP_PLUS:+\n                  await-expression co_await\n                    call-expression',
 'yield-expression co_yield\n                    assignment-expression OP_ASS:=',
 'yield-expression co_yield\n                    conditional-expression',
 'binary-expression OP_COMMA:,\n                    yield-expression co_yield',
 'coroutine-return-statement co_return\n          binary-expression OP_PLUS:+']:
 assert text in ast,text
symbols=run(['nm','-C',out/'contextual.o']).stdout
for name in ['coroutine<','forms<','empty_return<','constant_operand<','::member(']:assert name not in symbols,name
lir=(out/'throw-comma.lowir').read_text()
assert not re.search(r'(?:call|invoke).*@wrong',lir)
run([compiler,'--emit-ast',root/'pa5/tests/general/200-unsupported-co-return-bad.t','-o',out/'rejected.ast'],False)
negative={
 'namespace':'template<class T> T value=co_await T{};',
 'member-initializer':'template<class T> struct A{T value=co_await T{};};',
 'default':'template<class T> void f(T t=co_await T{});',
 'local-default':'template<class T> void f(T t){void g(int x=(co_await t));}',
 'local-class-initializer':'template<class T> void f(T t){struct A{int x=(co_await 1);};}',
 'sizeof':'template<class T> void f(T t){sizeof(co_await t);}',
 'decltype':'template<class T> void f(T t){using U=decltype(co_await t);}',
 'noexcept':'template<class T> void f(T t){noexcept(co_await t);}',
 'await-empty':'template<class T> void f(T t){co_await;}',
 'yield-empty':'template<class T> void f(T t){co_yield;}',
 'await-unbound':'template<class T> void f(T t){co_await missing;}',
 'yield-unbound':'template<class T> void f(T t){co_yield missing;}',
 'return-unbound':'template<class T> void f(T t){co_return missing;}',
 'await-fixed-error':'template<class T> void f(T t){co_await (1 + nullptr);}',
 'yield-fixed-error':'template<class T> void f(T t){co_yield (1 + nullptr);}',
 'return-fixed-error':'template<class T> void f(T t){co_return (1 + nullptr);}',
 'after-error':'template<class T> void f(T t){co_await t; missing=1;}',
 'direct-demand':'void f(){co_return;}',
 'template-demand':'template<class T> void f(T t){co_await t;} int main(){f(1);}',
 'member-demand':'template<class T> struct A{void f(T t){co_yield t;}}; int main(){A<int> a; a.f(1);}',
 'auto-demand':'template<class T> void f(T t){auto x=co_await t; co_return x;} int main(){f(1);}',
}
for name,source in negative.items():
 src=out/(name+'.cpp');src.write_text(source+'\n')
 run([compiler,'-c',src,'-o',out/'reject.o'],False)
print(f'{len(rows)} control commands passed')
