#!/usr/bin/env python3
"""Explicit structural constexpr validity, completion and cv controls."""
from pathlib import Path
import subprocess, sys, tempfile
ROOT = Path(__file__).resolve().parents[2]
CC = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
GOOD = {
 'literal_aggregate': 'struct A{int x;};constexpr int f(A){return 1;}',
 'reference_nonliteral': 'struct A{A();~A();};constexpr int f(A const&){return 1;}',
 'pointer_incomplete': 'struct A;constexpr int f(A*){return 1;}',
 'literal_second_base': 'struct A{constexpr A(){}};struct B{constexpr B(){}};struct C:A,B{constexpr C():A(),B(){}};',
 'declared_constructor': 'struct A{int x;constexpr A();};',
 'member_initializer': 'struct A{int x=3;constexpr A(){};};',
 'defaulted_member_initializer': 'struct A{int x=3;constexpr A()=default;};',
 'implicit_default_literal': 'struct A{int x=3;};constexpr int f(A){return 1;}',
 'defaulted_copy': 'struct A{int x;constexpr A(int n):x(n){} constexpr A(A const&)=default;};',
 'empty_union': 'union U{constexpr U(){}};',
 'initialized_union': 'union U{int x;double y;constexpr U():x(3){}};',
 'unused_template_body': 'template<class T>struct A{T x;constexpr A(){}};static_assert(sizeof(A<int>)>0, "");',
 'dependent_return': 'struct A{A(){}~A(){}};template<class T>constexpr T f(){return T();}',
 'dependent_parameter': 'struct A{A(){}~A(){}};template<class T>constexpr int f(T){return 1;}',
 'static_nonliteral_owner': 'struct A{~A(){} static constexpr int f(){return 1;}};static_assert(A::f()==1, "");',
 'static_out_of_class': 'struct A{static constexpr int f();};constexpr int A::f(){return 3;}static_assert(A::f()==3, "");',
 'implicit_const': 'struct A{constexpr int f(){return 3;}};int main(){const A a{};return a.f()-3;}',
 'implicit_const_template': 'template<class T>struct A{constexpr int f(){return 3;}};int main(){const A<int> a{};return a.f()-3;}',
 'implicit_const_out_of_class': 'struct A{constexpr int f();};constexpr int A::f(){return 3;}int main(){const A a{};return a.f()-3;}',
 'constexpr_deleted': 'struct A{int x;constexpr A()=delete;};',
 'nonliteral_ctor_owner': 'struct A{int x;constexpr A():x(3){}~A(){}};',
 'unnamed_bitfield': 'struct A{int:2;int x;constexpr A():x(3){}};',
}
BAD = {
 'void': 'constexpr void f(){}',
 'nonliteral_parameter': 'struct A{A();};constexpr int f(A){return 1;}',
 'nonliteral_return': 'struct A{A(){}~A(){}};constexpr A f(){return A();}',
 'nonliteral_member_owner': 'struct A{~A(){} constexpr int f(){return 1;}};',
 'nonliteral_member_decl': 'struct A{~A(){} constexpr int f();};',
 'nonliteral_variable': 'struct A{constexpr A(){}~A(){}};constexpr A a;',
 'nonliteral_array_variable': 'struct A{constexpr A(){}~A(){}};constexpr A a[2]{};',
 'missing_member': 'struct A{int x;constexpr A(){}};',
 'missing_second_member': 'struct A{int x,y;constexpr A():x(1){}};',
 'missing_array_member': 'struct A{int x[2];constexpr A(){}};',
 'missing_out_of_class': 'struct A{int x;constexpr A();};constexpr A::A(){}',
 'defaulted_missing_member': 'struct A{int x;constexpr A()=default;};',
 'nonconstexpr_base': 'struct A{A(){}};struct B:A{constexpr B():A(){}};',
 'nonconstexpr_second_base': 'struct A{constexpr A(){}};struct B{B(){}};struct C:A,B{constexpr C():A(),B(){}};',
 'nonconstexpr_class_member': 'struct A{A(){}};struct B{A a;constexpr B():a(){}};',
 'nonconstexpr_delegation': 'struct A{A(int){}constexpr A():A(3){}};',
 'empty_union_initialization': 'union A{int x;constexpr A(){}};',
 'volatile_field_literal': 'struct A{volatile int x;};constexpr int f(A){return 1;}',
 'nonliteral_array_field': 'struct A{A();};struct B{A x[2];};constexpr int f(B){return 1;}',
 'nonliteral_second_base_parameter': 'struct A{constexpr A(){}};struct B{B(){}};struct C:A,B{C(){}};constexpr int f(C){return 1;}',
 'virtual_constexpr': 'struct A{virtual constexpr int f(){return 1;}};',
 'mutate_implicit_const': 'struct A{int x;constexpr int f(){return ++x;}};',
 'fixed_template_return': 'struct A{A();};template<class T>constexpr A f(){return A();}',
 'fixed_template_parameter': 'struct A{A();};template<class T>constexpr int f(A){return 1;}',
}
failed=[]
with tempfile.TemporaryDirectory(prefix='pa16-validity-') as td:
 for name, source in {**GOOD,**BAD}.items():
  if name in GOOD and 'int main()' not in source: source+='int main(){return 0;}'
  src=Path(td)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=20)
  okay=(r.returncode==0)==(name in GOOD)
  if okay and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
   okay=r.returncode==0
   if okay:
    r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name,'PASS' if okay else 'FAIL',r.returncode,r.stderr.strip(),flush=True)
  if not okay:failed.append(name)
print(f'{len(GOOD)} native, {len(BAD)} rejection controls; failures: {failed}')
sys.exit(bool(failed))
