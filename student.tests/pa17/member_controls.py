#!/usr/bin/env python3
"""Member-template ownership and explicit-instantiation controls; run explicitly."""
from pathlib import Path
import json, re, subprocess, sys
import entity_controls as harness

harness.GOOD = {
    'qualified_head': '''struct C { typedef int Value; template<class T> Value get(T); };
template<class U> C::Value C::get(U x) { Value y=x; return y; }
int main(){ C c; return c.get(7)-7; }''',
    'cv_ref_overloads': '''struct C { template<class T> int f(T) & { return 1; }
template<class T> int f(T) const & { return 2; }
template<class T> int f(T) && { return 3; } };
int main(){ C c; const C d={}; return c.f(0)+d.f(0)+C().f(0)-6; }''',
    'static_template': '''struct C { template<class T> static int f(T); };
template<class U> int C::f(U x){return x+2;}
int main(){return C::f(5)-7;}''',
    'reference_field': '''struct C { int n; template<class R=int&> R operator*(){return n;} };
int main(){C c={4};*c=9;return c.n-9;}''',
    'constructor_template': '''struct C { int n; template<class T> explicit C(T x):n(x){} };
int main(){C c(9); C d(c);return d.n-9;}''',
    'constructor_pack': '''struct C { int n; template<class...T> C(T...x):n(sizeof...(x)){} };
int main(){C c(1,2,3);C d;return c.n+d.n-3;}''',
    'late_member_definition': '''struct C{template<class T> int f(T);};
int call(){C c;return c.f(6);}
template<class U> int C::f(U x){return x+1;}
int main(){return call()-7;}''',
    'specialized_member_body': '''struct C{template<class T> int f(T);};
template<> int C::f<int>(int);
template<class T> int C::f(T){static_assert(sizeof(T)==1,"primary");return 1;}
template<> int C::f<int>(int x){return x+3;}
int main(){C c;return c.f(4)+c.f('a')-8;}''',
    'explicit_function_deduction': '''template<class T>T f(T x){return x+2;}
extern template int f(int);template int f(int);
int main(){return f(5)-7;}''',
    'explicit_unused_member': '''struct C{template<class T>int f(T x){return x;}};
template int C::f(int);int main(){return 0;}''',
    'extern_then_definition_after_call': '''template<class T>int f(T x){return x+1;}
extern template int f<int>(int);int main(){return f(6)-7;}
template int f<int>(int);''',
    'extern_inline_member': '''template<class T>struct C{int f(){return 7;}};
extern template struct C<int>;int main(){C<int> c;return c.f()-7;}''',
    'explicit_ctor': '''template<class T>struct C{int n;C();};
template<class T>C<T>::C():n(9){} extern template C<int>::C();
template C<int>::C();int main(){C<int> c;return c.n-9;}''',
    'class_member_emission': '''template<class T>struct C{int f();};
template<class T>int C<T>::f(){return 7;}
extern template struct C<int>;template struct C<int>;
int main(){C<int> c;return c.f()-7;}''',
    'private_instantiation_argument': '''class Secret{typedef int Hidden;};
template<class T>int f(T x){return x;} template int f<Secret::Hidden>(Secret::Hidden);
int main(){return f(7)-7;}''',
    'inline_namespace_instantiation': '''namespace A{inline namespace B{template<class T>int f(T x){return x;}}
template int f(int);}int main(){return A::f(7)-7;}''',
}
harness.BAD = {
    'duplicate_member_definition': 'struct C{template<class T>int f(T){return 1;}};template<class U>int C::f(U){return 2;}',
    'private_member_call': 'class C{template<class T>int f(T){return 0;}};int main(){C c;return c.f(1);}',
    'explicit_builtin_operator': 'template<class T>int operator+(T,T);extern template int operator+<int>(int,int);',
    'duplicate_explicit_definition': 'template<class T>int f(T){return 0;}template int f(int);template int f(int);',
    'extern_after_definition': 'template<class T>int f(T){return 0;}template int f(int);extern template int f(int);',
    'no_matching_instantiation': 'template<class T>T f(T){return 0;}template int f<double>(int);',
    'explicit_constructor_copy_init': 'struct C{template<class T>explicit C(T){}};int main(){C c=1;}',
    'unqualified_instantiation_namespace': 'namespace A{template<class T>int f(T){return 0;}}using A::f;template int f(int);',
    'instantiation_does_not_exempt_definition': 'class Secret{typedef int Hidden;};template<class T>struct C{typedef typename T::Hidden value;};template<class T>void f(typename C<T>::value){} template void f<Secret>(C<Secret>::value);',
}
LOWIR = {
    'extern_free_suppression': ('''template<class T>int suppressed(T){static_assert(sizeof(T)==1,"dormant");return 1;}
extern template int suppressed(int);int main(){return suppressed(0);}''', r'^declare function @suppressed\(', r'^function @suppressed\('),
    'extern_member_suppression': ('''template<class T>struct C{int dormant();};
template<class T>int C<T>::dormant(){return sizeof(typename T::missing);}
extern template struct C<int>;int main(){C<int> c;return c.dormant();}''', r'^declare function @dormant\(', r'^function @dormant\('),
    'explicit_free_root': ('''template<class T>T retained(T x){return x;}
template int retained(int);int main(){return 0;}''', r'^function @retained\([^\n]*object_root=yes', r'^declare function @retained\('),
}
if __name__ == '__main__':
    cc = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else harness.ROOT/'dev/cppgm++'
    work = Path(sys.argv[2]) if len(sys.argv)>2 else Path('/tmp/pa17-member-controls')
    passed = harness.run(cc,work)
    rows = []
    for name,(source,required,forbidden) in LOWIR.items():
        src=work/(name+'.cpp');src.write_text(source);ir=work/(name+'.lowir')
        p=subprocess.run([str(cc),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)],capture_output=True,text=True)
        text=ir.read_text() if p.returncode==0 else ''
        good=p.returncode==0 and bool(re.search(required,text,re.M)) and not re.search(forbidden,text,re.M)
        rows.append(dict(name=name,source=source,required=required,forbidden=forbidden,compiler_exit=p.returncode,diagnostic=p.stderr,passed=bool(good)))
        passed &= bool(good);print(name,'PASS' if good else 'FAIL',p.stderr.strip())
    (work/'lowir-results.json').write_text(json.dumps(rows,indent=2)+'\n')
    sys.exit(0 if passed else 1)
