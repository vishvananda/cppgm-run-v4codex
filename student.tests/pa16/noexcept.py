#!/usr/bin/env python3
"""Explicit exception-specification and expression-effect controls."""
from pathlib import Path
import subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[2]
CC=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
GOOD={
 'calls': 'void f()noexcept;void g();static_assert(noexcept(f()),"");static_assert(!noexcept(g()),"");',
 'bool_type': 'void f();void g(bool);void g(unsigned long)=delete;int main(){g(noexcept(f()));}void g(bool){}',
 'indirect': 'void f()noexcept{}void (*p)()=f;static_assert(!noexcept(p()),"");',
 'unevaluated': 'int f();static_assert(noexcept(noexcept(f())),"");static_assert(noexcept(sizeof(f())),"");',
 'no_short_circuit': 'int f();static_assert(!noexcept(false && f()),"");static_assert(!noexcept(true ? 1 : f()),"");',
 'default_arg': 'int bad();int f(int=bad())noexcept;static_assert(!noexcept(f()),"");static_assert(noexcept(f(1)),"");',
 'constructor': 'struct A{A()noexcept;};struct B{B();};static_assert(noexcept(A()),"");static_assert(!noexcept(B()),"");',
 'implicit_ctor': 'struct A{};struct B{A a;B()=default;};static_assert(noexcept(B()),"");',
 'ctor_default_throw': 'int bad();struct A{A(int=bad())noexcept;};struct B{A a;B()=default;};static_assert(!noexcept(B()),"");',
 'ctor_default_no_throw': 'int good()noexcept;struct A{A(int=good())noexcept;};struct B{A a;B()=default;};static_assert(noexcept(B()),"");',
 'ctor_second_base': 'struct A{A()noexcept;};struct B{B();};struct C:A,B{C()=default;};static_assert(!noexcept(C()),"");',
 'member_init': 'int bad();struct A{int a=bad();A()=default;};static_assert(!noexcept(A()),"");',
 'member_init_no_throw': 'int good()noexcept;struct A{int a=good();A()=default;};static_assert(noexcept(A()),"");',
 'temporary_destructor': 'struct A{A()noexcept;~A()noexcept(false);};static_assert(!noexcept(A()),"");',
 'callee_argument_destructor': 'struct A{A()noexcept;~A()noexcept(false);};void f(A const&)noexcept;static_assert(!noexcept(f(A())),"");',
 'dtor_base': 'struct A{~A()noexcept(false){}};struct B:A{~B(){}};static_assert(!noexcept(((B*)0)->~B()),"");',
 'operator': 'struct A{int operator+(int)const noexcept;};static_assert(noexcept(((A*)0)->operator+(1)),"");',
 'conversion': 'struct A{operator int()const noexcept;};struct B{operator int()const;};int f(int)noexcept;static_assert(noexcept(f(*((A*)0))),"");static_assert(!noexcept(f(*((B*)0))),"");',
 'dependent_query': 'void f(int)noexcept;void f(double);template<class T>constexpr bool q(){return noexcept(f(T()));}static_assert(q<int>(),"");static_assert(!q<double>(),"");',
 'dependent_spec': 'template<class T>void f()noexcept(sizeof(T)>1);static_assert(noexcept(f<int>()),"");static_assert(!noexcept(f<char>()),"");',
 'prototype_parameter': 'void f(int x)noexcept(sizeof(x)>1);static_assert(noexcept(f(1)),"");',
 'recursive_member': 'template<class T>struct A{void f()noexcept(sizeof(T)>0);};struct B{A<B> a;};static_assert(noexcept(((B*)0)->a.f()),"");',
 'late_member': 'struct A{void f()noexcept(k());static constexpr bool k(){return true;}};static_assert(noexcept(((A*)0)->f()),"");',
 'unused_body': 'template<class T>struct A{void f()noexcept(sizeof(T)>0){typename T::missing x;}};static_assert(noexcept(((A<int>*)0)->f()),"");',
 'redeclaration': 'void f()noexcept(sizeof(int)>0);void f()noexcept(true);static_assert(noexcept(f()),"");',
 'direct_specialized': 'template<class T>void f()noexcept;static_assert(noexcept(f<int>()),"");',
 'contextual_conversion': 'struct A{constexpr A(){}constexpr explicit operator bool()const{return true;}};void f()noexcept(A());static_assert(noexcept(f()),"");',
 'contextual_false': 'struct A{constexpr A(){}constexpr explicit operator bool()const{return false;}};void f()noexcept(A());static_assert(!noexcept(f()),"");',
 'aggregate_omitted_ctor': 'struct A{A();};struct B{A a;};static_assert(!noexcept(B{}),"");',
 'aggregate_safe_ctor': 'struct A{A()noexcept;};struct B{A a;};static_assert(noexcept(B{}),"");',
 'nested_aggregate_omitted': 'struct A{A();};struct B{A a[2];};struct C{B b;};static_assert(!noexcept(C{}),"");',
 'nsdmi_conversion': 'struct A{A()noexcept;operator int();};struct B{int n=A();B()=default;};static_assert(!noexcept(B()),"");',
 'nsdmi_aggregate': 'struct A{A();};struct B{A a;};struct C{B b{};C()=default;};static_assert(!noexcept(C()),"");',
 'placement_allocation': 'void*operator new(unsigned long,void*)noexcept;struct A{A()noexcept;~A()noexcept(false);};static_assert(noexcept(new ((void*)0) A()),"");',
 'placement_throwing_ctor': 'void*operator new(unsigned long,void*)noexcept;struct A{A();};static_assert(!noexcept(new ((void*)0) A()),"");',
 'delete_safe': 'struct A{~A()noexcept;};static_assert(noexcept(delete (A*)0),"");',
 'delete_throwing': 'struct A{~A()noexcept(false);};static_assert(!noexcept(delete (A*)0),"");',
 'dependent_allocation': 'void*operator new(unsigned long,void*)noexcept;struct A{A()noexcept;~A()noexcept(false);};template<class T>constexpr bool f(){return noexcept(new ((void*)0) T());}static_assert(f<A>(),"");',
 'inherited_ctor': 'struct A{A(int)noexcept;};struct B:A{using A::A;};static_assert(noexcept(B(1)),"");',
 'inherited_ctor_member': 'struct A{A(int)noexcept;};struct M{M();};struct B:A{using A::A;M m;};static_assert(!noexcept(B(1)),"");',
 'inherited_deferred_ctor': 'template<class T>struct A{A(int)noexcept(sizeof(T)>0);};struct B:A<int>{using A<int>::A;};static_assert(noexcept(B(1)),"");',
}
BAD={'invalid_operand':'static_assert(noexcept(unknown()),"");',
 'nonconstant_spec':'int f();void g()noexcept(f());',
 'conflicting_spec':'void f()noexcept;void f()noexcept(false);'}
BAD.update({'conflicting_deferred':'struct A{void f()noexcept(sizeof(int)>0);};void A::f()noexcept(false){}',
 'invalid_unused_member_spec':'struct A{void f()noexcept(unknown());};',
 'prototype_runtime_value':'void f(int x)noexcept(x);'})
BAD.update({'nonconstexpr_conversion_receiver':'struct A{A(){}constexpr explicit operator bool()const{return true;}};void f()noexcept(A());',
 'effectful_conversion_receiver':'int x;struct A{constexpr A(){++x;}constexpr explicit operator bool()const{return true;}};void f()noexcept(A());'})
failed=[]
with tempfile.TemporaryDirectory(prefix='pa16-noexcept-') as td:
 for name,source in {**GOOD,**BAD}.items():
  if name in GOOD and 'int main()' not in source:source+='int main(){return 0;}'
  src=Path(td)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=20)
  okay=(r.returncode==0)==(name in GOOD)
  if okay and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True);okay=r.returncode==0
   if okay:r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name,'PASS' if okay else 'FAIL',r.returncode,r.stderr.strip(),flush=True)
  if not okay:failed.append(name)
print(f'{len(GOOD)} native, {len(BAD)} rejection controls; failures: {failed}')
sys.exit(bool(failed))
