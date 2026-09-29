#!/usr/bin/env python3
"""Omitted aggregate members, empty copies and parameter boundary controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
cases={
 'omitted_order':'int trace;struct M{M(){trace=trace*10+2;}~M(){trace=trace*10+3;}};struct A{int n;M m;};int f(){trace=1;return 7;}int main(){{A a={f()};if(a.n!=7||trace!=12)return 1;}return trace!=123;}',
 'omitted_throw':'int live;struct M{M(){++live;}~M(){--live;}};struct X{X(){throw 7;}};struct A{int n;M m;X x;};int main(){try{A a={1};}catch(int n){return n!=7||live;}return 1;}',
 'omitted_argument_throw':'int live;struct G{G(){throw 7;}};struct M{M(){++live;}~M(){--live;}};struct X{X(const G& g=G()){};};struct A{int n;M m;X x;};int main(){try{A a={1};}catch(int n){return n!=7||live;}return 1;}',
 'omitted_temporary_cleanup_throw':'int live;struct G{~G()noexcept(false){throw 7;}};struct M{M(const G&g=G()){++live;}~M(){--live;}};struct A{int n;M m;};int main(){try{A a={1};}catch(int n){return n!=7||live;}return 1;}',
 'noexcept_parameter_catch':'int live;struct P{P(){++live;}P(const P&){++live;}~P(){--live;}};int f(P p)noexcept{try{throw 7;}catch(int n){return n!=7||live!=2;}}int main(){P p;return f(p)||live!=1;}',
 'empty_copy_lifetime':'int drops;struct I{~I(){++drops;}};template<class T>struct Pair{T a,b;Pair(const T&x):a(x),b(x){}};int main(){I i;{Pair<I> p(i);}return drops!=2;}',
 'throwing_return_transfer':'int live;struct P{P(){++live;}P(P&&){throw 7;}~P(){--live;}};struct A{int n;P p;};A f(bool b){A a={1};A c={2};return b?static_cast<A&&>(a):static_cast<A&&>(c);}int main(){try{A a=f(true);}catch(int n){return n!=7||live;}return 1;}',
}
for label,member in {
 'throw':'struct M{M(){++live;}~M(){--live;}};struct X{X(){throw 7;}};struct A{int n;M m;X x;};',
 'argument_throw':'struct G{G(){throw 7;}};struct M{M(){++live;}~M(){--live;}};struct X{X(const G&g=G()){};};struct A{int n;M m;X x;};',
 'temporary_cleanup_throw':'struct G{~G()noexcept(false){throw 7;}};struct M{M(const G&g=G()){++live;}~M(){--live;}};struct A{int n;M m;};',
}.items():
 cases['prvalue_omitted_'+label]='int live;'+member+'A make(){return A{1};}int main(){try{A a=make();}catch(int n){return n!=7||live;}return 1;}'
cases['prvalue_shared_plain_constructor']='int live;struct M{M*self;M():self(this){++live;}~M(){--live;}};struct A{int n;M m;};A f(){return A{7};}A g(){return A{9};}int main(){{A a=f();A b=g();if(a.n!=7||b.n!=9||a.m.self!=&a.m||b.m.self!=&b.m||live!=2)return 1;}return live;}'
cases['prvalue_default_argument_identity']='int live,drops;struct G{G(){++live;}~G(){--live;++drops;}};struct M{int seen;M(const G&g=G()):seen(live){}};struct A{int n;M m;};A f(){return A{7};}A g(){return A{9};}int main(){A a=f();A b=g();return a.n!=7||b.n!=9||a.m.seen!=1||b.m.seen!=1||live||drops!=2;}'
name='200-hidden-eh-reference-prvalue-template-member-temp-cleanup'
cases[name]=(ROOT/'pa21/tests/general'/(name+'.t')).read_text()
rows=[]
for name,source in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=WORK/name
 commands=[]
 for cmd in ([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
   r=dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
  except subprocess.TimeoutExpired:r=dict(argv=list(map(str,cmd)),exit=124,stderr='timeout')
  commands.append(r)
  if r['exit']:break
 passed=len(commands)==4 and not commands[-1]['exit']
 rows.append(dict(name=name,source=source,commands=commands,passed=passed))
 print(name,'PASS' if passed else 'FAIL',r['stderr'].strip(),flush=True)
(WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(not all(r['passed'] for r in rows))
