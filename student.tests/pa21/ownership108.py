#!/usr/bin/env python3
"""Explicit result ownership and nonthrowing boundary execution controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
setup='extern "C" void exit(int);namespace std{typedef void(*H)();H set_terminate(H)noexcept;}void pass(){exit(0);}void fail(){exit(9);}void boom(){throw 7;}'
cases={
 'implicit_destructor_terminates':setup+'struct G{~G(){boom();}};int main(){std::set_terminate(pass);try{G g;}catch(...){return 1;}return 2;}',
 'explicit_destructor_terminates':setup+'struct G{~G()noexcept{boom();}};int main(){std::set_terminate(pass);try{G g;}catch(...){return 1;}return 2;}',
 'noexcept_function':setup+'void f()noexcept{boom();}int main(){std::set_terminate(pass);try{f();}catch(...){return 1;}return 2;}',
 'noexcept_true_template':setup+'template<bool B>void f()noexcept(B){boom();}int main(){std::set_terminate(pass);try{f<true>();}catch(...){return 1;}return 2;}',
 'throwing_destructor_escapes':setup+'struct G{~G()noexcept(false){boom();}};int main(){std::set_terminate(fail);try{G g;}catch(int n){return n!=7;}return 2;}',
 'noexcept_false_template':setup+'template<bool B>void f()noexcept(B){boom();}int main(){std::set_terminate(fail);try{f<false>();}catch(int n){return n!=7;}return 2;}',
 'noexcept_catches_inside':setup+'int f()noexcept{try{boom();}catch(int n){return n;}return 2;}int main(){std::set_terminate(fail);return f()!=7;}',
 'noexcept_catch_miss':setup+'void f()noexcept{try{boom();}catch(long){}}int main(){std::set_terminate(pass);try{f();}catch(...){return 1;}return 2;}',
 'noexcept_handler_escape':setup+'void f()noexcept{try{boom();}catch(int){boom();}}int main(){std::set_terminate(pass);try{f();}catch(...){return 1;}return 2;}',
 'noexcept_nested_handler':setup+'void f()noexcept{try{boom();}catch(int){try{boom();}catch(long){}}}int main(){std::set_terminate(pass);try{f();}catch(...){return 1;}return 2;}',
 'noexcept_indirect_call':setup+'void f(void(*p)())noexcept{p();}int main(){std::set_terminate(pass);try{f(boom);}catch(...){return 1;}return 2;}',
 'noexcept_local_default':setup+'struct G{G(){boom();}};void f()noexcept{G g;}int main(){std::set_terminate(pass);try{f();}catch(...){return 1;}return 2;}',
 'noexcept_local_cleanup':setup+'struct G{~G()noexcept(false){boom();}};void f()noexcept{G g;}int main(){std::set_terminate(pass);try{f();}catch(...){return 1;}return 2;}',
 'noexcept_constructor':setup+'struct G{G()noexcept{boom();}};int main(){std::set_terminate(pass);try{G g;}catch(...){return 1;}return 2;}',
 'noexcept_subobject_constructor':setup+'struct M{M(){boom();}};struct G{M m;G()noexcept{}};int main(){std::set_terminate(pass);try{G g;}catch(...){return 1;}return 2;}',
 'noexcept_subobject_destructor':setup+'struct M{~M()noexcept(false){boom();}};struct G{M m;~G()noexcept{}};int main(){std::set_terminate(pass);try{G g;}catch(...){return 1;}return 2;}',
 'noexcept_parameter_destructor':setup+'struct M{~M()noexcept(false){boom();}};void f(M m)noexcept{}int main(){std::set_terminate(pass);try{f(M());}catch(...){return 1;}return 2;}',
}
cases.update({
 'nrvo_observable': 'int live,copies;struct R{R(){++live;}R(const R&){++live;++copies;}~R(){--live;}};R f(){R r;return r;}int main(){{R r=f();if(live!=1||copies)return 1;}return live;}',
 'nrvo_self_pointer':'struct R{int n;int*p;R():n(7),p(&n){}R(const R&r):n(r.n),p(&n){}~R(){n=0;}};R f(){R r;return r;}int main(){R r=f();return r.p!=&r.n||*r.p!=7;}',
 'nrvo_throw_before_return':'int live;struct R{R(){++live;}~R(){--live;}};R f(){R r;throw 7;return r;}int main(){try{R r=f();}catch(int n){return n!=7||live;}return 1;}',
 'nrvo_throw_later_cleanup':'int live,trace;struct R{R(){++live;}~R(){--live;trace=trace*10+1;}};struct G{~G()noexcept(false){trace=trace*10+2;throw 7;}};R f(){R r;G g;return r;}int main(){try{R r=f();}catch(int n){return n!=7||live||trace!=21;}return 1;}',
 'nrvo_throw_earlier_cleanup':'int live,trace;struct R{R(){++live;}~R(){--live;trace=trace*10+1;}};struct G{~G()noexcept(false){trace=trace*10+2;throw 7;}};R f(){G g;R r;return r;}int main(){try{R r=f();}catch(int n){return n!=7||live||trace!=21;}return 1;}',
 'nrvo_multiple_returns':'int live,copies;struct R{R(){++live;}R(const R&){++live;++copies;}~R(){--live;}};R f(bool yes){R r;if(yes)return r;return r;}int main(){{R a=f(true);R b=f(false);if(live!=2||copies)return 1;}return live;}',
 'nrvo_handler_return':'int live,copies;struct R{R(){++live;}R(const R&){++live;++copies;}~R(){--live;}};R f(){R r;try{throw 7;}catch(int){return r;}}int main(){{R a=f();if(live!=1||copies)return 1;}return live;}',
 'nrvo_throw_caught_inside':'int live;struct R{R(){++live;}~R(){--live;}};R f(){R r;try{throw 7;}catch(int){}return r;}int main(){{R a=f();if(live!=1)return 1;}return live;}',
 'nrvo_const_object':'int live,copies;struct R{R(){++live;}R(const R&){++live;++copies;}~R(){--live;}};R f(){const R r;return r;}int main(){{R r=f();if(live!=1||copies)return 1;}return live;}',
 'nrvo_polymorphic':'int live,copies;struct R{R(){++live;}R(const R&){++live;++copies;}virtual ~R(){--live;}virtual int f(){return 7;}};R f(){R r;return r;}int main(){{R r=f();if(live!=1||copies||r.f()!=7)return 1;}return live;}',
})
for name in ('100-nested-class-template-local-class-argument','200-class-value-argument-transfers-caller-cleanup','200-hidden-eh-const-ref-bound-temp-dtor','200-empty-aggregate-return-through-switch'):
 cases[name]=(ROOT/'pa21/tests/general'/ (name+'.t')).read_text()
cases.update({
 'temporary_dereferenced_this':'int count;struct G{int n;G(int v):n(v){}struct Call{G&g;Call(G&x):g(x){}void operator()(){count+=g.n;}};void f(){Call(*this)();}};int main(){G g(7);g.f();return count!=7;}',
 'aggregate_empty_class_member':'int count;struct M{M(){++count;}};struct A{M m;int n;};int main(){A a{};return count!=1||a.n;}',
 'aggregate_empty_pointer_member':'struct A{int*p;int(*f)(int);};int main(){A a{};return a.p!=nullptr||a.f!=nullptr;}',
 'aggregate_empty_data_member_pointer':'struct C{int n;};struct A{int C::*p;};int main(){A a{};return a.p!=nullptr;}',
 'aggregate_empty_volatile':'struct A{volatile int n;};int main(){A a{};return a.n;}',
 'aggregate_empty_nested':'struct B{int*p;int n;};struct A{B b;long n;};int main(){A a{};return a.b.p!=nullptr||a.b.n||a.n;}',
})
for count in (1,3,8,9,32):
 for fail in sorted(set((1,2 if count>1 else 1,count))):
  source='int next,live,expected,bad;int fail='+str(fail)+';struct M{int n;M():n(++next){if(n==fail)throw n;++live;}~M(){--live;if(n!=expected--)bad=1;}};'
  source+='struct A{M elements['+str(count)+'];};int main(){expected=fail-1;try{A a{};}catch(int n){return n!=fail||live||bad||expected;}return 2;}'
  cases['aggregate_array_%s_fail_%s'%(count,fail)]=source
cases.update({
 'aggregate_member_prefix':'int live,trace;struct M{int n;M(int n):n(n){if(n==3)throw n;++live;}~M(){--live;trace=trace*10+n;}};struct A{M a,b,c;};int main(){try{A a={M(1),M(2),M(3)};}catch(int n){return n!=3||live||trace!=21;}return 1;}',
 'aggregate_lvalue_copy_prefix':'int live,copies,trace;struct M{int n;M(int n):n(n){++live;}M(const M&m):n(m.n){if(++copies==3)throw 7;++live;}~M(){--live;trace=trace*10+n;}};struct A{M a,b,c;};int main(){M x(1),y(2),z(3);try{A a={x,y,z};}catch(int n){return n!=7||live!=3||trace!=21;}return 1;}',
 'aggregate_nested_prefix':'int live,trace;struct M{int n;M(int n):n(n){if(n==4)throw n;++live;}~M(){--live;trace=trace*10+n;}};struct B{M a,b;};struct A{B a,b;};int main(){try{A a={{M(1),M(2)},{M(3),M(4)}};}catch(int n){return n!=4||live||trace!=321;}return 1;}',
})
rows=[]
def run(cmd):
 try:
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
  return dict(argv=list(map(str,cmd)),exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
 except subprocess.TimeoutExpired:return dict(argv=list(map(str,cmd)),exit=124,stderr='timeout')
for name,source in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');obj=src.with_suffix('.o');exe=WORK/name
 commands=[]
 for cmd in ([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],[ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir],['g++','-no-pie',obj,'-o',exe],[exe]):
  commands.append(run(cmd))
  if commands[-1]['exit']:break
 passed=len(commands)==4 and commands[-1]['exit']==0
 rows.append(dict(name=name,source=source,commands=commands,passed=passed))
 print(name,'PASS' if passed else 'FAIL',commands[-1]['exit'],commands[-1]['stderr'].strip(),flush=True)
 (WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in rows) else 1)
