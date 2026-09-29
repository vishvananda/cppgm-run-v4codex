#!/usr/bin/env python3
"""PA21 RTTI formation, demand, identity and execution controls. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner

info = 'namespace std{class type_info{public:bool operator==(const type_info&)const;bool operator!=(const type_info&)const;};}'
base = 'struct B{virtual int f(){return 3;}};struct D:B{int f(){return 7;}};'
runner.GOOD = {
 'fundamental_distinct': info+'int main(){return typeid(int)==typeid(long)||typeid(double)!=typeid(double);}',
 'strip_top_cv_ref': info+'int main(){const volatile int x=3;return typeid(x)!=typeid(int)||typeid(const int&)!=typeid(int);}',
 'pointer_nested_cv': info+'int main(){return typeid(int*)==typeid(const int*)||typeid(int*const)!=typeid(int*);}',
 'array_no_decay': info+'int main(){int a[3];return typeid(a)!=typeid(int[3])||typeid(a)==typeid(int*);}',
 'function_no_decay': info+'int f(int);int main(){return typeid(f)!=typeid(int(int))||typeid(f)==typeid(int(*)(int));}',
 'scalar_unevaluated': info+'int calls;int f(){++calls;return 0;}int main(){return typeid(f())!=typeid(int)||calls;}',
 'template_body_unevaluated': info+'template<class T>int bad(){return T::missing;}int main(){return typeid(bad<int>())!=typeid(int);}',
 'static_class_unevaluated': info+'struct A{};template<class T>A bad(){return T::missing;}int main(){return typeid(bad<int>())!=typeid(A);}',
 'dynamic_reference': info+base+'int main(){D d;B&b=d;return typeid(b)!=typeid(D);}',
 'dynamic_evaluated_once': info+base+'int calls;template<class T>B&select(T&x){++calls;return x;}int main(){D d;return typeid(select(d))!=typeid(D)||calls!=1;}',
 'template_types': info+'template<class T>int f(){return typeid(T)==typeid(int);}int main(){return !f<int>()||f<long>()||!f<int>();}',
 'template_fixed_type': info+'template<class T>int f(){return typeid(int)==typeid(int);}int main(){return !f<int>()||!f<long>();}',
 'reference_alias': info+'int main(){const auto&x=typeid(int);return x!=typeid(int);}',
 'enum_identity': info+'enum E{a};enum F{b};int main(){return typeid(E)==typeid(F)||typeid(a)!=typeid(E);}',
 'incomplete_pointer': info+'struct A;int main(){return typeid(A*)!=typeid(A*)||typeid(A*)==typeid(void*);}',
 'no_eager_layout': info+'template<class T>struct A{typename T::missing x;};int main(){return typeid(A<int>*)!=typeid(A<int>*);}',
 'lambda_identity': info+'int main(){auto a=[](){return 1;};auto b=[](){return 1;};return typeid(a)==typeid(b)||typeid(a)!=typeid(a);}',
 'cast_success': base+'int main(){D d;B*p=&d;return dynamic_cast<D*>(p)!=&d;}',
 'cast_failure': base+'int main(){B b;return dynamic_cast<D*>(&b)!=0;}',
 'cast_null': base+'int main(){B*p=0;return dynamic_cast<D*>(p)!=0;}',
 'cast_cross_failure': base+'struct U{virtual int g(){return 0;}};int main(){D d;B*p=&d;return dynamic_cast<U*>(p)!=0;}',
 'cast_same_nonpolymorphic': 'struct A{};int main(){A a;return dynamic_cast<const A*>(&a)!=&a;}',
 'cast_up_nonpolymorphic': 'struct A{int x;};struct D:A{};int main(){D d;A*a=dynamic_cast<A*>(&d);return a!=static_cast<A*>(&d);}',
 'cast_template': base+'template<class T>T*cast(B*p){return dynamic_cast<T*>(p);}int main(){D d;B b;return cast<D>(&d)!=&d||cast<D>(&b)!=0;}',
 'noexcept_typeid_static': info+'int f();int main(){return !noexcept(typeid(f()));}',
 'noexcept_typeid_dynamic': info+base+'int main(){B*p=0;return noexcept(typeid(*p));}',
 'noexcept_cast_pointer': base+'int main(){B*p=0;return !noexcept(dynamic_cast<D*>(p));}',
 'member_pointer_rtti': info+'struct A{int f()const;int x;};struct B{int x;};int main(){return typeid(int A::*)==typeid(int B::*)||typeid(int(A::*)()const)==typeid(int(A::*)());}',
 'typeid_function_syntax': info+'int main(){return typeid(int())==typeid(int)||typeid(int(3))!=typeid(int);}',
 'explicit_comparison': info+'int main(){const auto&p=typeid(int);return !p.operator==(typeid(int))||(&p)->operator!=(typeid(int));}',
 'comparison_conversion': info+'int calls;struct A{operator const std::type_info&(){++calls;return typeid(int);}};int main(){A a;return !(typeid(int)==a)||calls!=1;}',
 'private_downcast': 'struct B{virtual int f(){return 1;}};struct D:private B{B*get(){return this;}};int main(){D d;B*p=d.get();return dynamic_cast<D*>(p)!=0;}',
 'private_cast_evaluated_once': 'int calls;struct B{virtual int f(){return 1;}};struct D:private B{B*get(){++calls;return this;}};int main(){D d;return dynamic_cast<D*>(d.get())!=0||calls!=1;}',
 'protected_base_chain': 'struct B{virtual int f(){return 1;}};struct C:protected B{B*get(){return this;}};struct D:C{};int main(){D d;B*p=d.get();return dynamic_cast<D*>(p)!=0||dynamic_cast<C*>(p)!=0;}',
 'public_base_inside_private_derived': 'struct B{virtual int f(){return 1;}};struct C:B{};struct D:private C{B*get(){return this;}C*mid(){return this;}};int main(){D d;return dynamic_cast<C*>(d.get())!=d.mid();}',
 'comparison_arrow_chain': info+'int calls;struct A{const std::type_info*operator->(){++calls;return &typeid(int);}};int main(){A a;return !a->operator==(typeid(int))||calls!=1;}',
 'comparison_implicit_this': 'namespace std{class type_info{public:bool operator==(const type_info&)const;bool same(const type_info&x)const{return operator==(x);}};}int main(){return !typeid(int).same(typeid(int))||typeid(int).same(typeid(long));}',
 'comparison_template_explicit': info+'template<class T>bool same(const T&a,const T&b){return a.operator==(b);}int main(){return !same(typeid(int),typeid(int))||same(typeid(int),typeid(long));}',
 'local_type_identity': info+'template<class T>const std::type_info& f(){struct A{};return typeid(A);}int main(){return f<int>()==f<long>()||f<int>()!=f<int>();}',
 'typeid_decltype_lvalue': info+'decltype(typeid(int)) f(){return typeid(int);}int main(){return f()!=typeid(int);}',
 'typeid_array_cv': info+'int main(){const int a[2]={};return typeid(a)!=typeid(int[2])||typeid(const int[2])!=typeid(int[2]);}',
 'function_template_identity': info+'template<class T>long f(T){return 1;}int main(){return typeid(f<int>)!=typeid(long(int))||typeid((f<long>))!=typeid(long(long));}',
 'function_template_type_query': info+'template<class T>long f(T){return 1;}decltype(typeid(f<int>)) g(){return typeid(long(int));}int main(){return g()!=typeid(f<int>);}',
 'rtti_template_member_dormant': info+'template<class T>struct A{virtual int f(){return T::missing;}};int main(){return typeid(A<int>)!=typeid(A<int>);}',
 'new_trivial_copy': 'struct A{int n;A(int x):n(x){}};template<class T>int f(const T&x){T*p=new T(x);int n=p->n;delete p;return n;}int main(){A a(7);return f(a)!=7;}',
}
runner.BAD = {
 'typeinfo_missing':'int main(){typeid(int);}',
 'typeinfo_global_only':'class type_info;int main(){typeid(int);}',
 'comparison_missing':'namespace std{class type_info{};}int main(){return typeid(int)==typeid(int);}',
 'comparison_private':'namespace std{class type_info{bool operator==(const type_info&)const;};}int main(){return typeid(int)==typeid(int);}',
 'comparison_deleted':'namespace std{class type_info{public:bool operator==(const type_info&)const=delete;};}int main(){return typeid(int)==typeid(int);}',
 'incomplete_object':info+'struct A;int main(){typeid(A);}',
 'cast_nonpolymorphic':'struct B{};struct D:B{};int main(){B b;dynamic_cast<D*>(&b);}',
 'cast_remove_const':base+'int main(){const D d;const B*p=&d;dynamic_cast<D*>(p);}',
 'cast_private_base':'struct B{};struct D:private B{};int main(){D d;dynamic_cast<B*>(&d);}',
 'cast_incomplete_target':base+'struct U;int main(){B b;dynamic_cast<U*>(&b);}',
 'dynamic_body_required':info+base+'template<class T>B&bad(){return T::missing;}int main(){return typeid(bad<int>())==typeid(B);}',
 'unresolved_function':info+'void f(int);void f(long);int main(){typeid(f);}',
 'query_unresolved_function':info+'void f(int);void f(long);decltype(typeid(f)) bad();int main(){}',
 'typeid_deleted_function':info+'void f()=delete;int main(){typeid(f);}',
 'typeid_nonstatic_function':info+'struct A{void f();};int main(){typeid(A::f);}',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
