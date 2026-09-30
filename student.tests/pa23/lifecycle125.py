#!/usr/bin/env python3
"""Explicit PA23 lifecycle controls; validated LowIR and checked hosted execution."""
from pathlib import Path
import json,sys
import layout123 as harness
cases={
 'construct-destroy-once':'int live;struct V{V()noexcept{++live;}~V()noexcept{--live;}virtual void f(){}};struct A:virtual V{};struct B:virtual V{};struct C:A,B{};struct D:C{};int main(){{D d;if(live!=1)return 1;}return live!=0;}',
 'construction-dispatch':'int log;struct V{V(){log=log*10+f();}virtual int f(){return 1;}~V(){log=log*10+f();}};struct A:virtual V{A(){log=log*10+f();}int f(){return 2;}~A(){log=log*10+f();}};struct P{long p;};struct D:P,A{D(){log=log*10+f();}int f(){return 3;}~D(){log=log*10+f();}};int main(){{D d;}return log!=123321;}',
 'construction-secondary-dispatch':'int seen;struct V{virtual int f(){return 1;}};struct A:virtual V{A(){V&v=*this;seen=seen*10+v.f();}int f(){return 2;}};struct B:virtual V{B(){V&v=*this;seen=seen*10+v.f();}int f(){return 3;}};struct D:A,B{int f(){return 4;}};int main(){D d;V&v=d;return seen!=23||v.f()!=4;}',
 'construction-rtti':'namespace std{class type_info{public:bool operator==(const type_info&)const;};}int bad;struct V{virtual int f(){return 1;}};struct A:virtual V{A(){V&v=*this;if(!(typeid(v)==typeid(A)))++bad;}};struct B:virtual V{};struct D:A,B{};int main(){D d;return bad;}',
 'nested-shared-vptr':'struct V{int x;V():x(7){}virtual int f(){return x;}};struct A:virtual V{int f(){return x+1;}};struct B:virtual A{};struct C:virtual A{};struct E:B,C{};struct D:E{long guard;D():guard(11){}};int main(){D d;V&v=d;return v.f()!=8||d.guard!=11;}',
 'virtual-nonvirtual-tail':'struct V{int x;V():x(7){}};struct A:virtual V{};struct P{long y;P():y(11){}};struct B:P,A{};struct D:virtual B{int z;D():z(13){}};int main(){D d;A&a=d;return a.x!=7||d.y!=11||d.z!=13;}',
 'copy-complete-from-base':'struct V{int x;V():x(3){}};struct A:virtual V{};struct P{long y;};struct D:A,P{long z;};int main(){D d;d.x=7;A&a=d;A copy(a);return copy.x!=7;}',
 'copy-base-skips-shared':'struct V{int x;V(int i=3):x(i){}};struct A:virtual V{};struct P{long y;};struct D:P,A{D(const A&a):V(9),A(a){}};int main(){A a;a.x=7;D d(a);return d.x!=9;}',
 'copy-nested-shared':'struct V{int x;V():x(3){}};struct A:virtual V{};struct B:virtual V{};struct D:A,B{};int main(){D d;d.x=7;D copy(d);return copy.x!=7;}',
 'assign-complete-base-view':'struct V{int x;V():x(3){}};struct A:virtual V{};struct P{long y;};struct D:A,P{long z;};int main(){D d,e;d.x=7;e.x=1;A&a=e;A&b=d;a=b;return e.x!=7;}',
 'delegating-base-entry':'struct V{int x;V(int v=3):x(v){}};struct A:virtual V{A():A(4){}A(int n):V(n){}};struct P{long p;};struct D:P,A{D():V(9){}};int main(){A a;D d;return a.x!=4||d.x!=9;}',
 'inherited-base-entry':'struct V{int x;V():x(3){}};struct A:virtual V{int y;A(int n):y(n){}};struct B:A{using A::A;};struct P{long p;};struct D:P,B{D():B(7){}};int main(){D d;return d.x!=3||d.y!=7;}',
 'byvalue-parameter':'struct V{int x;V():x(3){}};struct A:virtual V{};int read(A a){return a.x;}int main(){A a;a.x=7;return read(a)!=7;}',
}
value_base='struct V{int x;V():x(3){}};struct A:virtual V{};'
cases.update({
 'byvalue-indirect':value_base+'int read(A a){return a.x;}int main(){A a;a.x=7;int(*p)(A)=&read;return p(a)!=7;}',
 'byvalue-constructor':value_base+'struct B{int y;B(A a):y(a.x){}};int main(){A a;a.x=7;B b(a);return b.y!=7;}',
 'byvalue-conversion':value_base+'struct B{int y;B(A a):y(a.x){}};int main(){A a;a.x=7;B b=a;return b.y!=7;}',
 'byvalue-inherited':value_base+'struct B{int y;B(A a):y(a.x){}};struct C:B{using B::B;};int main(){A a;a.x=7;C c(a);return c.y!=7;}',
 'byvalue-virtual':value_base+'struct I{virtual int read(A a){return a.x;}};struct P{virtual void f(){}};struct D:P,I{int read(A a){return a.x+1;}};int main(){A a;a.x=7;D d;I&i=d;return i.read(a)!=8;}',
 'byvalue-member-pointer':value_base+'struct B{int read(A a){return a.x;}};int main(){A a;a.x=7;B b;int(B::*p)(A)=&B::read;return (b.*p)(a)!=7;}',
 'byvalue-multiple':value_base+'int read(A a,int n,A b){return a.x+n+b.x;}int main(){A a,b;a.x=7;b.x=11;return read(a,3,b)!=21;}',
 'byvalue-lifecycle-hidden':value_base+'struct B:virtual V{int y;B(A a):y(a.x){}};struct C:B{C(A a):V(),B(a){}};int main(){A a;a.x=7;C c(a);return c.y!=7||c.x!=3;}',
 'base-initializer-virtual-access':'struct V{int x;V():x(7){}};struct B{int y;B(int v):y(v){}};struct A:virtual V,B{A():B(x){}};struct P{long p;};struct D:P,A{};int main(){D d;return d.y!=7;}',
})
if __name__=='__main__':
 harness.cases=cases;harness.unfinished={}
 cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);result=harness.check(cc,work)
 print(json.dumps(result,indent=2));sys.exit(not all(x['passed'] for x in result['cases']))
