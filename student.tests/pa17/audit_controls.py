#!/usr/bin/env python3
"""Checkpoint audit reducers and interaction controls; run explicitly."""
from pathlib import Path
import hashlib, json, sys
import entity_controls as harness

# N3485 13.3.3 [over.match.best]/1-2 requires a no-worse conversion for
# every argument before either the non-template or partial-order tie-break.
harness.GOOD = {
    'constructor_equal_conversions': '''struct C{int n;C(int):n(1){}
template<class T>C(T):n(2){}};int main(){C c(1);return c.n-1;}''',
    'constructor_template_better_conversion': '''struct C{int n;C(long):n(1){}
template<class T>C(T):n(2){}};int main(){C c(1);return c.n-2;}''',
    'constructor_nontemplate_better_conversions': '''struct C{int n;C(int,int):n(1){}
template<class T>C(long,T):n(2){}};int main(){C c(1,1);return c.n-1;}''',
    'ordinary_alias_redeclaration': 'using A=int;using A=int;int main(){A n=0;return n;}',
    'template_alias_redeclaration': '''template<class T>using A=T*;
template<class U>using A=U*;int main(){int n=0;A<int> p=&n;return *p;}''',
    'alias_access_from_own_definition': '''class Secret{typedef int Hidden;
template<class T>using A=typename T::Hidden;
public:int f(){A<Secret> n=7;return n;}};int main(){Secret s;return s.f()-7;}''',
    'explicit_private_member_names': '''class Secret{typedef int Hidden;
public:template<class T>void f(T);};template<class T>void Secret::f(T){}
template void Secret::f<Secret::Hidden>(Secret::Hidden);int main(){return 0;}''',
    'qualified_member_own_context': '''class Secret{typedef int Hidden;
public:void f(Hidden);};void Secret::f(Hidden){}int main(){Secret s;s.f(1);return 0;}''',
    'alias_tuple_signature': '''template<class R,class T,class U>using F=R(T,U);
int f(char a,long b){return a+b;}int main(){F<int,char,long>*p=f;return p(3,4)-7;}''',
    'alias_tuple_empty_pack': '''template<class R,class...T>using F=R(T...);
int f(){return 7;}int main(){F<int>*p=f;return p()-7;}''',
    'selected_owner_tuple_and_access': '''template<class X,class Y>struct Pair{};
template<class T>struct A;template<class X,class Y>struct A<Pair<X,Y>>{
typedef X value;static value f(Y);};template<class R,class S>
typename A<Pair<R,S>>::value A<Pair<R,S>>::f(S n){return sizeof(R)+n;}
int main(){return A<Pair<int,char>>::f(3)+A<Pair<char,int>>::f(2)-10;}''',
    'partial_repeated_use': '''template<class T>struct A;template<class T>struct A<T*>{int f();};
template<class U>int A<U*>::f(){return sizeof(U);}
int main(){A<int*>a;A<char*>b;return a.f()+a.f()+b.f()-9;}''',
}
harness.BAD = {
    'crossed_constructor_conversions': '''struct C{C(int,long){}
template<class T>C(long,T){}};int main(){C c(1,1);}''',
    'crossed_constructor_reverse_order': '''struct C{template<class T>C(long,T){}
C(int,long){}};int main(){C c(1,1);}''',
    # [basic.scope.declarative]/4 and [temp]/5: a template's name cannot
    # also designate an ordinary alias, even if the aliased type is equal.
    'alias_template_then_alias': 'template<class T>using A=int;using A=int;',
    'alias_then_alias_template': 'using A=int;template<class T>using A=int;',
    # [temp.explicit]/12 exempts names in the instantiation; [class.access]/1
    # and [temp.res]/8 still check the alias definition in its own context.
    'explicit_member_does_not_privilege_alias': '''class Secret{typedef int Hidden;
public:template<class T>void f(T);};template<class T>using A=typename T::Hidden;
template<class T>void Secret::f(T){}template void Secret::f<int>(A<Secret>);''',
    'qualified_member_does_not_privilege_alias': '''class Secret{typedef int Hidden;
public:void f(int);};template<class T>using A=typename T::Hidden;
void Secret::f(A<Secret>){}''',
    'qualified_member_does_not_privilege_class': '''class Secret{typedef int Hidden;
public:void f(int);};template<class T>struct C{typedef typename T::Hidden type;};
void Secret::f(C<Secret>::type){}''',
    'recursive_alias_demand': '''template<class T>struct B;
template<class T>using A=typename B<T>::type;
template<class T>struct B{typedef A<T> type;};A<int> x;''',
}

if __name__ == '__main__':
    cc = Path(sys.argv[1]).resolve()
    work = Path(sys.argv[2])
    passed = harness.run(cc,work)
    rows = json.loads((work/'results.json').read_text())
    for row in rows:
        row['source'] = (harness.GOOD if row['expected']=='native' else harness.BAD)[row['name']]
        row['source_sha256'] = hashlib.sha256(row['source'].encode()).hexdigest()
    (work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
    sys.exit(0 if passed else 1)
