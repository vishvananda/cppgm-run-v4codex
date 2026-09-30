#!/usr/bin/env python3
"""Lifecycle context and hidden by-value ABI across separate and merged TUs."""
from pathlib import Path
import json,sys
import audit124 as harness
h='struct V{int x;V();~V();virtual int f();};struct A:virtual V{A();~A();int f();};struct P{long guard;};struct D:P,A{D();~D();int f();};'
cases={'external-lifecycle':[h+'int bad;V::V():x(7){}V::~V(){}int V::f(){return 1;}A::A(){V&v=*this;if(v.f()!=2||v.x!=7)++bad;}A::~A(){V&v=*this;if(v.f()!=2)++bad;}int A::f(){return 2;}',h+'extern int bad;D::D(){}D::~D(){}int D::f(){return 3;}int main(){{D d;V&v=d;if(v.f()!=3||v.x!=7)return 1;}return bad;}']}
h='struct V{int x;V():x(3){}};struct A:virtual V{};'
cases['value-abi']=[h+'int read(A a){return a.x;}',h+'int read(A);int main(){A a;a.x=7;return read(a)!=7;}']
cases['opaque-reference']=['struct A;int read(A&);int relay(A&a){return read(a);}',h+'int relay(A&);int read(A&a){return a.x;}struct P{long guard;};struct D:A,P{};int main(){D d;d.x=7;return relay(d)!=7;}']
cases['opaque-value-declaration']=['struct A;int read(A);'+h+'int read(A a){return a.x;}',h+'int read(A);int main(){A a;a.x=7;return read(a)!=7;}']
h='struct A{virtual int f(){return 1;}};struct D:A{int f(){return 7;}};'
cases['member-pointer-abi']=[h+'int(A::*get())(){return &A::f;}',h+'int(A::*get())();int main(){D d;return (d.*get())()!=7;}']
h='struct V{virtual int f(){return 1;}};struct A:virtual V{};struct D:A{virtual int g(){return 7;}};'
cases['dispatch-before-lifecycle']=[h+'int call(D&d){return d.g();}',h+'int call(D&);int main(){D d;return call(d)!=7;}']
for name,sources in list(cases.items()):cases[name+'-reversed']=list(reversed(sources))
if __name__=='__main__':
 harness.cases=cases
 cc,work=map(lambda x:Path(x).resolve(),sys.argv[1:3]);result=harness.check(cc,work)
 print(json.dumps(result,indent=2));sys.exit(not all(x['passed'] for x in result['cases']))
