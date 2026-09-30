#!/usr/bin/env python3
"""Virtual member pointer formation/storage/conversion/dispatch controls."""
from pathlib import Path
import json,sys
import layout123 as harness
h='struct A{virtual int a(){return 1;}};struct B{virtual int b(){return 2;}};struct D:A,B{int b(){return 7;}};'
cases={
 'secondary-conversion':h+'int main(){D d;int(D::*p)()=&B::b;return (d.*p)()!=7;}',
 'primary-logical-slot':h+'int main(){D d;int(D::*p)()=&D::b;return (d.*p)()!=7;}',
 'static-storage':h+'int(D::*p)()=&B::b;int main(){D d;return (d.*p)()!=7;}',
 'argument-storage':h+'int call(D&d,int(D::*p)()){return (d.*p)();}int main(){D d;return call(d,&B::b)!=7;}',
 'inverse-nonpoly-owner':'struct P{int x;};struct D:P{virtual int f(){return 7;}};int call(P&p,int(P::*f)()){return (p.*f)();}int main(){D d;int(D::*a)()=&D::f;int(P::*b)()=static_cast<int(P::*)()>(a);return call(d,b)!=7;}',
 'pure-virtual-address':'struct A{virtual int f()=0;};struct D:A{int f(){return 7;}};int(A::*p)()=&A::f;int main(){D d;return (d.*p)()!=7;}',
 'shared-virtual-owner':'struct V{virtual int f(){return 1;}};struct A:virtual V{};struct B:virtual V{};struct D:A,B{int f(){return 7;}};int main(){D d;int(V::*p)()=&V::f;return (d.*p)()!=7;}',
 'nonvirtual-virtual-mix':h+'struct E:D{int f(){return 5;}};int main(){E e;int(E::*p)()=&E::f;if((e.*p)()!=5)return 1;p=&B::b;return (e.*p)()!=7;}',
 'field-flow':h+'struct H{int(D::*p)();};int main(){D d;H h;h.p=&B::b;return (d.*h.p)()!=7;}',
 'null-conversion':h+'int main(){int(B::*b)()=nullptr;int(D::*d)()=b;return d!=nullptr;}',
 'constexpr-target':h+'constexpr int(D::*p)()=&B::b;int main(){D d;return (d.*p)()!=7;}',
 'virtual-value-argument':'struct V{int x;V():x(3){}};struct A:virtual V{};struct I{virtual int read(A a){return a.x;}};struct D:I{int read(A a){return a.x+1;}};int main(){A a;a.x=7;D d;int(I::*p)(A)=&I::read;return (d.*p)(a)!=8;}',
 'overloaded-slot':'struct A{virtual int f(int x){return x;}virtual int f(double){return 2;}};struct D:A{int f(double){return 7;}};int main(){D d;int(A::*p)(double)=&A::f;return (d.*p)(1.0)!=7;}',
}
if __name__=='__main__':
 harness.cases=cases;harness.unfinished={}
 cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);result=harness.check(cc,work)
 print(json.dumps(result,indent=2));sys.exit(not all(x['passed'] for x in result['cases']))
