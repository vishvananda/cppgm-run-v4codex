#!/usr/bin/env python3
"""Final audit: closure storage across implicit range operations. Run CC WORK."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner

runner.GOOD = {
 'captured_array': 'int main(){int a[2]={3,4};auto f=[&](){int sum=0;for(auto x:a)sum+=x;return sum;};return f()!=7;}',
 'captured_array_parens': 'int main(){int a[2]={3,4};auto f=[&a](){int sum=0;for(auto x:((a)))sum+=x;return sum;};return f()!=7;}',
 'captured_array_mutation': 'int main(){int a[2]={3,4};auto f=[&a](){for(auto&x:a)++x;};f();return a[0]!=4||a[1]!=5;}',
 'captured_const_array': 'int main(){const int a[2]={3,4};auto f=[&](){int sum=0;for(const auto&x:a)sum+=x;return sum;};return f()!=7;}',
 'captured_volatile_array': 'int main(){volatile int a[2]={3,4};auto f=[&](){int sum=0;for(auto&x:a){sum+=x;++x;}return sum;};return f()!=7||a[0]!=4||a[1]!=5;}',
 'captured_array_alias': 'int main(){int a[2]={3,4};auto&r=a;auto f=[&r](){int sum=0;for(auto&x:r)sum+=x;return sum;};return f()!=7;}',
 'captured_nested_arrays': 'int main(){int a[2][2]={{1,2},{3,4}};auto f=[&](){int sum=0;for(auto&r:a)for(auto x:r)sum+=x;return sum;};return f()!=10;}',
 'nested_capture_range': 'int main(){int a[2]={3,4};auto f=[&](){auto g=[&](){int sum=0;for(auto x:a)sum+=x;return sum;};return g();};return f()!=7;}',
 'local_inner_capture': 'int main(){auto f=[](){int a[2]={3,4};auto g=[&](){int sum=0;for(auto x:a)sum+=x;return sum;};return g();};int(*p)()=f;return f()!=7||p()!=7;}',
 'captured_range_parameter': 'template<class T,int N>int sum(T(&a)[N]){auto f=[&](){int s=0;for(auto x:a)s+=x;return s;};return f();}int main(){int a[2]={3,4};long b[3]={2,3,4};return sum(a)!=7||sum(b)!=9||sum(a)!=7;}',
 'range_this_array': 'struct S{int a[2];int sum(){return [this](){int s=0;for(auto x:a)s+=x;return s;}();}};int main(){S a={{3,4}},b={{5,6}};return a.sum()!=7||b.sum()!=11;}',
 'range_capture_in_body': 'int main(){int a[2]={3,4},s=0;for(auto&x:a){auto f=[&](){s+=x;++x;};f();}return s!=7||a[0]!=4||a[1]!=5;}',
 'range_static': 'int main(){static int a[2]={3,4};auto f=[](){int s=0;for(auto x:a)s+=x;return s;};return f()!=7;}',
 'range_braced_capture': 'int main(){int a=3,b=4;auto f=[&](){int s=0;for(auto x:{a,b})s+=x;return s;};return f()!=7;}',
 'captured_class_array_lifetime': 'int live;struct V{int n;V(int n):n(n){++live;}V(const V&v):n(v.n){++live;}~V(){--live;}};int main(){{V a[2]={3,4};auto f=[&](){int s=0;for(auto x:a){if(live!=3)return 100;s+=x.n;}return s;};if(f()!=7||live!=2)return 1;}return live;}',
 'captured_range_break': 'int live;struct R{int a[2];R():a{3,4}{++live;}~R(){--live;}int*begin(){return a;}int*end(){return a+2;}};int main(){{R r;auto f=[&](){for(auto&x:r){++x;break;}return live;};if(f()!=1||r.a[0]!=4||r.a[1]!=4)return 1;}return live;}',
 'captured_range_return': 'int live;struct V{int n;V(int n):n(n){++live;}V(const V&v):n(v.n){++live;}~V(){--live;}};int main(){{V a[2]={3,4};auto f=[&](){for(auto x:a)return x.n;return 0;};if(f()!=3||live!=2)return 1;}return live;}',
 'captured_range_continue': 'int main(){int a[3]={2,3,4};auto f=[&](){int s=0;for(auto x:a){if(x==3)continue;s+=x;}return s;};return f()!=6;}',
}
member = 'struct R{int a[2];int*begin(){return a;}int*end(){return a+2;}};'
adl = 'namespace N{struct R{int a[2];};int*begin(R&r){return r.a;}int*end(R&r){return r.a+2;}};'
for name, decl, ty in [('member',member,'R'),('adl',adl,'N::R')]:
 runner.GOOD['captured_'+name] = decl + 'int main(){'+ty+' r={{3,4}};auto f=[&](){int s=0;for(auto&x:r){s+=x;++x;}return s;};return f()!=7||r.a[0]!=4||r.a[1]!=5;}'
 runner.GOOD['nested_'+name] = decl + 'int main(){'+ty+' r={{3,4}};auto f=[&](){auto g=[&](){int s=0;for(auto x:r)s+=x;return s;};return g();};return f()!=7;}'
 runner.GOOD['dependent_'+name] = decl + 'template<class T>int sum(T&r){auto f=[&](){int s=0;for(auto x:r)s+=x;return s;};return f();}int main(){'+ty+' a={{3,4}},b={{5,6}};return sum(a)!=7||sum(b)!=11;}'
 runner.GOOD['fixed_'+name] = decl + 'template<class T>int sum(){'+ty+' r={{3,4}};auto f=[&](){int s=0;for(auto x:r)s+=x;return s;};return f();}int main(){return sum<int>()!=7||sum<long>()!=7;}'
runner.BAD = {
 'uncaptured_array': 'int main(){int a[2]={3,4};auto f=[](){for(auto x:a){};};}',
 'uncaptured_const_array': 'int main(){const int a[2]={3,4};auto f=[](){for(auto x:a){};};}',
 'uncaptured_range': member+'int main(){R r={{3,4}};auto f=[](){for(auto x:r){};};}',
}
if __name__ == '__main__':
 sys.exit(0 if runner.run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
