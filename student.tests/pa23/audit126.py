#!/usr/bin/env python3
"""Final audit: interactions across PA23 semantic and program ABI owners."""
from pathlib import Path
import json
import sys
import audit124 as harness

value = 'struct V{int x;V():x(3){}};struct W{int y;W():y(5){}};struct A:virtual V,virtual W{};'
result = 'struct R{long x,y,z;R(long n):x(n),y(n+1),z(n+2){}};'
cases = {
    'value-result-secondary-call': [value+result+'struct P{virtual int pad(){return 1;}};struct I{virtual R read(A a){return R(a.x+a.y);}};struct D:P,I{R read(A a){return R(a.x+a.y+1);}};int main(){A a;a.x=7;a.y=11;D d;I&i=d;R r=i.read(a);return r.x!=19||r.z!=21;}'],
    'value-result-member-call': [value+result+'struct P{virtual int pad(){return 1;}};struct I{virtual R read(A a){return R(a.x+a.y);}};struct D:P,I{R read(A a){return R(a.x+a.y+1);}};int main(){A a;a.x=7;a.y=11;D d;R(D::*p)(A)=&I::read;R r=(d.*p)(a);return r.x!=19||r.z!=21;}'],
    'value-result-indirect-call': [value+result+'R read(A a){return R(a.x+a.y);}int main(){A a;a.x=7;a.y=11;R(*p)(A)=&read;R r=p(a);return r.x!=18||r.z!=20;}'],
    'value-derived-projections': [value+'struct B:A{long pad;};int read(B b){A&a=b;return b.x+b.y+a.x+a.y;}int main(){B b;b.x=7;b.y=11;return read(b)!=36;}'],
    'covariant-member-result': ['struct P{virtual void pad(){}};struct V{virtual V*self(){return this;}};struct D:P,virtual V{bool empty;D():empty(false){}D*self(){return empty?0:this;}};int main(){D d;V*(V::*v)()=&V::self;D*(D::*p)()=&D::self;if((d.*v)()!=static_cast<V*>(&d)||(d.*p)()!=&d)return 1;d.empty=true;return (d.*v)()!=0||(d.*p)()!=0;}'],
    'construction-order-shared': ['int log;struct V{V(){log=log*10+1;}};struct W{W(){log=log*10+2;}};struct A:virtual V,virtual W{A(){log=log*10+3;}};struct B:virtual W,virtual V{B(){log=log*10+4;}};struct D:A,B{D(){log=log*10+5;}};int main(){D d;return log!=12345;}'],
    'template-shared-construction': ['template<int N>struct V{int x;V():x(N){}virtual int f(){return x;}};template<int N>struct A:virtual V<N>{int f(){return this->x+1;}int dormant(){return N.missing;}};template<int N>struct B:virtual V<N>{};template<int N>struct D:A<N>,B<N>{};template<int N>int read(D<N>d){V<N>&v=d;return v.f();}int main(){D<6>d;return read(d)!=7;}'],
    'null-shared-projection': ['struct V{int x;virtual int f(){return x;}};struct A:virtual V{};struct P{virtual int p(){return 1;}};struct D:P,A{};V*project(A*p){return p;}P*cross(A*p){return dynamic_cast<P*>(p);}int main(){D d;A*a=&d;return project(0)!=0||cross(0)!=0||project(a)!=static_cast<V*>(&d)||cross(a)!=static_cast<P*>(&d);}'],
}
h = value+result+'struct I{virtual R read(A);};struct P{virtual int pad(){return 1;}};struct D:P,I{R read(A);};'
cases['external-value-result-thunk'] = [h+'R I::read(A a){return R(a.x+a.y);}R D::read(A a){return R(a.x+a.y+1);}', h+'int main(){D d;A a;a.x=7;a.y=11;R(D::*p)(A)=&I::read;R r=(d.*p)(a);return r.x!=19||r.z!=21;}']
h = 'namespace std{class type_info{public:bool operator==(const type_info&)const;};}extern int bad;struct V{virtual int f();};struct A:virtual V{A();int f();};struct P{virtual int pad();};struct D:P,A{D();int f();};'
cases['external-construction-rtti'] = [h+'int bad;int V::f(){return 1;}A::A(){V&v=*this;if(v.f()!=2||!(typeid(v)==typeid(A))||dynamic_cast<void*>(&v)!=this)++bad;}int A::f(){return 2;}', h+'int P::pad(){return 4;}D::D(){}int D::f(){return 3;}int main(){D d;V&v=d;return bad||v.f()!=3||!(typeid(v)==typeid(D));}']
for name, sources in list(cases.items()):
    if len(sources) > 1:
        cases[name+'-reversed'] = list(reversed(sources))

if __name__ == '__main__':
    harness.cases = cases
    cc, work = [Path(p).resolve() for p in sys.argv[1:3]]
    data = harness.check(cc, work)
    print(json.dumps(data, indent=2))
    sys.exit(not all(row['passed'] for row in data['cases']))
