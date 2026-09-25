#!/usr/bin/env python3
"""Discarded source forms, selected operators and volatile reads: CC WORK."""
from pathlib import Path
import json,re,sys
import ordering_controls as runner
P='volatile int v=7;int calls;volatile int&get(){++calls;return v;}struct X{volatile int&operator[](int){++calls;return v;}};'
runner.GOOD={
 'reference_cast': 'int v=7,calls;template<class T>T& get(T&x){++calls;return x;}int main(){(void)get(v);return calls!=1||v!=7;}',
 'reference_statement': 'int v=7,calls;template<class T>T& get(T&x){++calls;return x;}int main(){get(v);return calls!=1||v!=7;}',
 'volatile_reference_cast': P+'void probe(){(void)get();}int main(){probe();return calls!=1;}',
 'volatile_reference_statement': P+'void probe(){get();}int main(){probe();return calls!=1;}',
 'volatile_id': P+'void probe(){(void)v;}int main(){probe();return calls;}',
 'volatile_xvalue': P+'void probe(){(void)static_cast<volatile int&&>(v);}int main(){probe();return calls;}',
 'volatile_xvalue_member': 'struct A{volatile int v;};A a={7};void probe(){(void)static_cast<A&&>(a).v;}int main(){probe();return a.v!=7;}',
 'volatile_conditional_overload': P+'void probe(bool b){X x;(void)(b?x[0]:v);}int main(){probe(false);probe(true);return calls!=1;}',
 'volatile_conditional_call': P+'void probe(bool b){(void)(b?get():v);}int main(){probe(false);probe(true);return calls!=1;}',
 'volatile_conditional_builtin': P+'void probe(bool b){volatile int*p=&v;(void)(b?*p:v);}int main(){probe(false);probe(true);return calls;}',
 'volatile_conditional_comma': P+'void probe(bool b){X x;(void)(b?(++calls,x[0]):v);}int main(){probe(false);probe(true);return calls!=2;}',
 'volatile_comma': P+'void probe(){(void)(++calls,v);}int main(){probe();return calls!=1;}',
 'volatile_overload': P+'void probe(){X x;(void)x[0];}int main(){probe();return calls!=1;}',
 'volatile_member_pointer': 'struct A{volatile int n;};A a={7};void probe(){volatile int A::*p=&A::n;(void)(a.*p);}int main(){probe();return a.n!=7;}',
 'volatile_pointer_member': 'struct A{volatile int n;};A a={7};void probe(){A*q=&a;volatile int A::*p=&A::n;(void)(q->*p);}int main(){probe();return a.n!=7;}',
 'volatile_member_pointer_xvalue': 'struct A{volatile int n;};A a={7};void probe(){volatile int A::*p=&A::n;(void)(static_cast<A&&>(a).*p);}int main(){probe();return a.n!=7;}',
 'volatile_template_conditional': P+'template<class T>void probe(T&x,bool b){(void)(b?x[0]:v);}int main(){X x;probe(x,false);probe(x,true);return calls!=1;}',
 'volatile_template_fixed': P+'template<class T>void probe(T){X x;(void)(true?x[0]:v);}int main(){probe(0);probe(0L);return calls!=2;}',
}
CLASS='int copies,dtors;struct A{A(){}A(volatile A&){++copies;}~A(){++dtors;}};'
runner.GOOD.update({
 'class_cast':CLASS+'int main(){volatile A a;(void)a;return copies!=1||dtors!=1;}',
 'class_statement':CLASS+'int main(){volatile A a;a;return copies!=1||dtors!=1;}',
 'class_parentheses':CLASS+'int main(){volatile A a;((a));return copies!=1||dtors!=1;}',
 'class_comma':CLASS+'int main(){volatile A a;(a,1);return copies!=1||dtors!=1;}',
 'class_void_functional':CLASS+'int main(){volatile A a;(void(a));return copies!=1||dtors!=1;}',
 'class_conditional':CLASS+'int main(){volatile A a,b;(void)(true?a:b);return copies!=1||dtors!=1;}',
 'class_conditional_call':CLASS+'volatile A&get(volatile A&a){return a;}int main(){volatile A a,b;(void)(true?get(a):b);return copies||dtors;}',
 'class_xvalue':CLASS+'struct B{A a;};int main(){volatile B b;(void)static_cast<volatile B&&>(b).a;return copies||dtors;}',
 'class_template_dependent':CLASS+'template<class T>void f(T&x){(void)x;}int main(){volatile A a;f(a);f(a);return copies!=2||dtors!=2;}',
 'class_template_fixed':CLASS+'volatile A a;template<class T>void f(T){(void)a;}int main(){f(1);f(1L);return copies!=2||dtors!=2;}',
 'class_template_comma':CLASS+'volatile A a;template<class T>void f(T){(a,1);}int main(){f(1);f(1L);return copies!=2||dtors!=2;}',
 'class_template_parentheses':CLASS+'volatile A a;template<class T>void f(T){((a));}int main(){f(1);f(1L);return copies!=2||dtors!=2;}',
 'class_unevaluated':CLASS+'int main(){volatile A a;static_assert(sizeof(((void)a,1))==sizeof(int),"");return copies||dtors;}',
 'class_default_copy_argument':'int defaults,copies,dtors;int f(){return ++defaults;}struct A{A(){}A(volatile A&,int n=f()){copies+=n;}~A(){++dtors;}};int main(){volatile A a;(void)a;return copies!=1||dtors!=1||defaults!=1;}',
})
runner.GOOD.update({
 'query_discard_sfinae':'struct A{};template<class T>auto f(T*p)->decltype((void)*p,int()){return 1;}int f(...){return 2;}int main(){volatile A a;return f(&a)!=2;}',
 'query_discard_valid':CLASS+'template<class T>auto f(T*p)->decltype((void)*p,int()){return 1;}int f(...){return 2;}int main(){volatile A a;return f(&a)!=1||copies||dtors;}',
 'query_discard_comma':'struct A{};template<class T>auto f(T*p)->decltype((*p,1)){return 1;}int f(...){return 2;}int main(){volatile A a;return f(&a)!=2;}',
 'query_discard_call':'struct A{};template<class T>T&get(T&);template<class T>auto f(T*p)->decltype((void)get(*p),int()){return 1;}int f(...){return 2;}int main(){volatile A a;return f(&a)!=1;}',
 'query_discard_overload':'struct A{};struct X{volatile A&operator[](int);};template<class T>auto f(T*p)->decltype((void)(*p)[0],int()){return 1;}int f(...){return 2;}int main(){X x;return f(&x)!=1;}',
 'query_discard_conditional':'struct A{};template<class T>auto f(T*p)->decltype((void)(true?*p:*p),int()){return 1;}int f(...){return 2;}int main(){volatile A a;return f(&a)!=2;}',
 'query_discard_mixed':'struct A{};template<class T>T&get(T&);template<class T>auto f(T*p)->decltype((void)(true?get(*p):*p),int()){return 1;}int f(...){return 2;}int main(){volatile A a;return f(&a)!=1;}',
})
runner.GOOD.update({
 'class_array_element':CLASS+'int main(){volatile A a[2];(void)a[0];return copies!=1||dtors!=1;}',
 'class_indirection':CLASS+'int main(){volatile A a;volatile A*p=&a;(void)*p;return copies!=1||dtors!=1;}',
 'class_member':CLASS+'struct B{A a;};int main(){volatile B b;(void)b.a;return copies!=1||dtors!=1;}',
 'class_nonvolatile':CLASS+'int main(){A a;(void)a;return copies||dtors;}',
 'class_constvolatile':'int copies;struct A{A(){}A(const volatile A&){++copies;}};int main(){const volatile A a;(void)a;return copies!=1;}',
 'class_full_expression_order':CLASS+'int seen;void inspect(){seen=dtors;}int main(){volatile A a;((void)a,inspect());return copies!=1||dtors!=1||seen;}',
 'class_template_unevaluated':'template<class T>struct A{A(){}A(volatile A&){T::missing();}};template<class T>auto f(T*p)->decltype((void)*p,int()){return 1;}int main(){volatile A<int>a;return f(&a)!=1;}',
 'class_template_query_deleted':'template<class T>struct A{A(){}A(volatile A&)=delete;};template<class T>auto f(T*p)->decltype((void)*p,int()){return 1;}int f(...){return 2;}int main(){volatile A<int>a;return f(&a)!=2;}',
})
# Source throw/catch execution belongs to PA21. PA18 checks emitted cleanup
# edges and executes successful construction/destruction under the supplied backend.
COUNTS={'volatile_reference_cast':0,'volatile_reference_statement':0,'volatile_id':1,'volatile_xvalue':0,'volatile_xvalue_member':0,'volatile_conditional_overload':0,'volatile_conditional_call':0,'volatile_conditional_builtin':2,'volatile_conditional_comma':0,'volatile_comma':1,'volatile_overload':0,'volatile_member_pointer':1,'volatile_pointer_member':1,'volatile_member_pointer_xvalue':0,'volatile_template_conditional':0,'volatile_template_fixed':0}
runner.BAD={
 'class_no_volatile_copy':'struct A{};int main(){volatile A a;(void)a;}',
 'class_deleted_copy':'struct A{A(){}A(volatile A&)=delete;};int main(){volatile A a;a;}',
 'class_private_copy':'class A{A(volatile A&);public:A(){}};int main(){volatile A a;(void)a;}',
 'class_explicit_copy':'struct A{A(){}explicit A(volatile A&){}};int main(){volatile A a;(void)a;}',
 'class_unevaluated_bad':'struct A{};int main(){volatile A a;using T=decltype((void)a);}',
}
if __name__=='__main__':
 cc=Path(sys.argv[1]).resolve();w=Path(sys.argv[2]);ok=runner.run(cc,w);rows=[]
 for name,expected in COUNTS.items():
  ir=(w/(name+'.lowir')).read_text()
  bodies=re.findall(r'^function @probe[^\n]*\{\n(.*?)^}',ir,re.S|re.M)
  count=sum(len(re.findall(r'load volatile i32',b)) for b in bodies)
  passed=bool(bodies) and count==expected
  rows.append(dict(name=name,expected_volatile_reads=expected,actual_volatile_reads=count,passed=passed))
  print(name,'reads',count,'expected',expected,'PASS' if passed else 'FAIL',flush=True);ok &= passed
 (w/'inspections.json').write_text(json.dumps(rows,indent=2)+'\n');sys.exit(0 if ok else 1)
