#!/usr/bin/env python3
"""Typed range, lookup, evaluation and lifetime controls. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
runner.GOOD = {
 'array_modify':'int main(){int a[3]={1,2,3};for(auto&x:a) x*=2;return a[0]!=2||a[2]!=6;}',
 'array_forward':'int main(){int a[2]={1,2};for(auto&&x:a) ++x;return a[0]!=2||a[1]!=3;}',
 'nested_arrays':'int main(){int a[2][2]={{1,2},{3,4}};int n=0;for(auto&row:a)for(const auto&v:row)n+=v;return n!=10;}',
 'array_break_continue':'int main(){int a[5]={1,2,3,4,5};int n=0;for(int x:a){if(x==2)continue;if(x==4)break;n+=x;}return n!=4;}',
 'array_list':'int main(){int n=0;for(const auto&x:{1L,2L,3L})n+=x;return n!=6;}',
 'array_volatile':'int main(){volatile int a[2]={3,4};int n=0;for(auto&x:a){n+=x;++x;}return n!=7||a[0]!=4||a[1]!=5;}',
 'template_array':'template<class T,int N>int sum(T(&a)[N]){int n=0;for(auto&&x:a)n+=x;return n;}int main(){int a[2]={3,4};long b[3]={1,2,3};return sum(a)!=7||sum(b)!=6;}',
 'template_fixed_array':'template<class T>int sum(){int a[2]={3,4};int n=0;for(int x:a)n+=x;return n;}int main(){return sum<int>()!=7||sum<long>()!=7;}',
 'lambda_array':'int main(){auto f=[](){int a[2]={3,4};int n=0;for(auto x:a)n+=x;return n;};return f()!=7;}',
 'single_array_evaluation':'int calls;int a[2]={3,4};int(&f())[2]{++calls;return a;}int main(){int n=0;for(auto&x:f())n+=x;return calls!=1||n!=7;}',
 'array_prvalue':'using A=int[2];int main(){int n=0;for(auto x:A{3,4})n+=x;return n!=7;}',
 'array_lvalue_xvalue':'using A=int[2];int main(){A a={3,4};for(auto&x:static_cast<A&&>(a))++x;return a[0]!=4||a[1]!=5;}',
}
RANGE='''int live;int destroyed;int begins;int ends;
struct R{int a[3];R(){++live;a[0]=1;a[1]=2;a[2]=3;}~R(){--live;++destroyed;}int*begin(){++begins;return a;}int*end(){++ends;return a+3;}};
'''
runner.GOOD.update({
 'range_lifetime':RANGE+'int main(){int n=0;for(auto x:R()){if(live!=1||destroyed)return 1;n+=x;}return n!=6||live||destroyed!=1||begins!=1||ends!=1;}',
 'range_break_lifetime':RANGE+'int main(){for(auto x:R()){if(x==2)break;}return live||destroyed!=1;}',
 'range_return_lifetime':RANGE+'int f(){for(auto x:R()){if(x==2)return live;}return 0;}int main(){return f()!=1||live||destroyed!=1;}',
 'range_goto_lifetime':RANGE+'int main(){for(auto x:R()){if(x==2)goto done;}done:return live||destroyed!=1;}',
 'range_continue_lifetime':RANGE+'int main(){int n=0;for(auto x:R()){if(live!=1)return 2;if(x==2)continue;n+=x;}return n!=4||live||destroyed!=1;}',
 'range_template_lifetime':RANGE+'template<class T>int f(){int n=0;for(auto x:T())n+=x;return n;}int main(){return f<R>()!=6||live||destroyed!=1;}',
 'range_member_single_eval':RANGE+'int calls;R&f(R&r){++calls;return r;}int main(){R r;int n=0;for(auto x:f(r))n+=x;return n!=6||calls!=1||begins!=1||ends!=1||destroyed;}',
 'adl_local_shadow':'namespace N{struct R{int a[2];};int*begin(R&r){return r.a;}int*end(R&r){return r.a+2;}}int main(){N::R r={{3,4}};int begin=1,end=2,n=0;for(auto x:r)n+=x;return n!=7||begin!=1||end!=2;}',
 'static_endpoints':'int a[2]={3,4};struct R{static int*begin(){return a;}static int*end(){return a+2;}};int main(){R r;int n=0;for(auto x:r)n+=x;return n!=7;}',
 'adl_template':'namespace N{template<class T>struct R{T a[2];};template<class T>T*begin(R<T>&r){return r.a;}template<class T>T*end(R<T>&r){return r.a+2;}}template<class T>int f(T&r){int n=0;for(auto x:r)n+=x;return n;}int main(){N::R<long> r={{3,4}};return f(r)!=7;}',
 'iteration_class_copy':'int alive,copies;struct V{int n;V(int n):n(n){++alive;}V(const V&o):n(o.n){++alive;++copies;}~V(){--alive;}};int main(){V a[2]={V(3),V(4)};int baseline=copies;for(V x:a){if(alive!=3)return 1;}return alive!=2||copies-baseline!=2;}',
})
ITER = """int copies;int living;int derefs;int increments;
struct I{int*p;I(int*p):p(p){++living;}I(const I&o):p(o.p){++living;++copies;}~I(){--living;}
int&operator*(){++derefs;return *p;}I&operator++(){++p;++increments;return *this;}bool operator!=(const I&o){return p!=o.p;}};
struct R{int a[2];I begin(){return I(a);}I end(){return I(a+2);}};
"""
runner.GOOD.update({
 'iterator_lifetime':ITER+'int main(){R r={{3,4}};int n=0;for(auto&x:r){if(living!=2)return 2;n+=x;}return n!=7||living||derefs!=2||increments!=2;}',
 'iterator_break':ITER+'int main(){R r={{3,4}};for(auto&x:r){break;}return living||derefs!=1||increments;}',
 'iterator_return':ITER+'int f(){R r={{3,4}};for(auto&x:r){return x;}return 0;}int main(){return f()!=3||living;}',
 'iterator_continue':ITER+'int main(){R r={{3,4}};for(auto&x:r){continue;}return living||derefs!=2||increments!=2;}',
 'iterator_reference_copy':ITER+'I*saved;struct S{int*a;const I&begin(){return *saved;}I end(){return I(a+2);}};int main(){int a[2]={3,4};I i(a);saved=&i;S s={a};int n=0;int baseline=copies;for(auto x:s)n+=x;return n!=7||living!=1||copies-baseline!=1||i.p!=a;}',
 'proxy_bool':'struct B{bool b;explicit operator bool()const{return b;}};struct I{int*p;int operator*(){return *p;}I&operator++(){++p;return *this;}B operator!=(I o){return B{p!=o.p};}};struct R{int a[2];I begin(){return I{a};}I end(){return I{a+2};}};int main(){R r={{3,4}};int n=0;for(int x:r)n+=x;return n!=7;}',
 'endpoint_defaults':'int calls;int f(){return ++calls;}struct R{int a[2];int*begin(int n=f()){return a;}int*end(int n=f()){return a+2;}};int main(){R r={{3,4}};int n=0;for(auto x:r)n+=x;return n!=7||calls!=2;}',
 'element_temporary_lifetime':'int alive;struct V{int n;V(int n):n(n){++alive;}~V(){--alive;}};struct I{int*p;V operator*(){return V(*p);}I&operator++(){++p;return *this;}bool operator!=(I o){return p!=o.p;}};struct R{int a[2];I begin(){return I{a};}I end(){return I{a+2};}};int main(){R r={{3,4}};int n=0;for(const auto&x:r){if(alive!=1)return 2;n+=x.n;}return n!=7||alive;}',
})
runner.BAD = {
 'list_mixed':'int main(){for(auto x:{1,2L}){}}',
 'list_empty':'int main(){for(auto x:{}){}}',
 'list_mutable_reference':'int main(){for(int&x:{1,2}){}}',
 'array_const_reference':'int main(){const int a[2]={1,2};for(int&x:a){}}',
 'no_adl_ordinary':'struct R{};namespace X{int*begin(R&);int*end(R&);}using namespace X;int main(){R r;for(int x:r){}}',
 'member_hides_adl':'namespace N{struct R{int begin;};int*begin(R&);int*end(R&);}int main(){N::R r;for(int x:r){}}',
 'mismatched_end':'struct R{int*begin();long*end();};int main(){R r;for(int x:r){}}',
 'private_begin':'class R{int*begin();public:int*end();};int main(){R r;for(int x:r){}}',
 'goto_range_body':'int main(){int a[2]={1,2};goto in;for(int x:a){in: return x;}}',
}
if __name__=='__main__':sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
