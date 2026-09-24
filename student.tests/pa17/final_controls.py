#!/usr/bin/env python3
"""Final PA17 audit: closure exception facts and adjacent ownership consumers."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
SAME = 'template<class,class>struct Same{static const bool value=false;};template<class T>struct Same<T,T>{static const bool value=true;};'
GOOD = {
 'parameter_size': 'int main(){auto f=[](int n)noexcept(sizeof(n)==4){return n;};static_assert(noexcept(f(7)),"n");return f(7)!=7;}',
 'parameter_shadow': 'long n;int main(){auto f=[](int n)noexcept(sizeof(n)==4){return n;};static_assert(noexcept(f(7)),"n");return f(7)!=7;}',
 'parameter_cv': SAME+'int main(){auto f=[](const int n)noexcept(Same<decltype(n),const int>::value){return n;};static_assert(noexcept(f(7)),"cv");return f(7)!=7;}',
 'parameter_array': SAME+'int main(){auto f=[](int n[9])noexcept(Same<decltype(n),int*>::value){return n[0];};int a[9]={7};static_assert(noexcept(f(a)),"array");return f(a)!=7;}',
 'parameter_function': SAME+'int step(){return 7;}int main(){auto f=[](int n())noexcept(Same<decltype(n),int(*)()>::value){return n();};static_assert(noexcept(f(step)),"function");return f(step)!=7;}',
 'template_parameter': 'template<class T>int run(){auto f=[](T n)noexcept(sizeof(n)==4){return n;};return noexcept(f(7));}int main(){return run<int>()!=1||run<long>()!=0;}',
 'template_parameter_cv': SAME+'template<class T>int run(){auto f=[](const T n)noexcept(Same<decltype(n),const T>::value){return n;};return noexcept(f(7));}int main(){return run<int>()!=1||run<long>()!=1;}',
 'parameter_pack': 'template<class...T>int run(T...n){auto f=[](T...x)noexcept(sizeof...(x)==2){return sizeof...(x);};return noexcept(f(n...));}int main(){return run()!=0||run(1,2L)!=1||run(1)!=0;}',
 'lexical_this': 'struct S{long n;int run(){auto f=[](int x)noexcept(sizeof(*this)==8){return x;};static_assert(noexcept(f(7)),"this");return f(7);}};int main(){S s;return s.run()!=7;}',
 'template_lexical_this': 'template<class T>struct S{T n;int run(){auto f=[](int x)noexcept(sizeof(*this)==8){return x;};return noexcept(f(7));}};int main(){S<int>a;S<long>b;return a.run()!=0||b.run()!=1;}',
 'nested_parameter': 'int main(){auto f=[]()->int{auto g=[](int n)noexcept(sizeof(n)==4){return n;};static_assert(noexcept(g(7)),"nested");return g(7);};return f()!=7;}',
 'fixed_lookup': 'constexpr bool test(long){return true;}template<class T>int run(){auto f=[]()noexcept(test(0)){return 7;};return noexcept(f());}constexpr bool test(int){return false;}int main(){return run<int>()!=1;}',
 'contextual_bool_true': 'struct B{constexpr explicit operator bool()const{return true;}};int main(){auto f=[]()noexcept(B()){return 7;};static_assert(noexcept(f()),"bool");return f()!=7;}',
 'contextual_bool_false': 'struct B{constexpr explicit operator bool()const{return false;}};int main(){auto f=[]()noexcept(B()){return 7;};static_assert(!noexcept(f()),"bool");return f()!=7;}',
 'pointer_adapter': 'int main(){auto f=[](int n)noexcept(sizeof(n)==4){return n+1;};int(*p)(int)=f;return p(6)!=7;}',
 'empty_copy_effects': 'int n;struct E{};E g(){++n;return E();}E f(){return E(g());}int main(){f();return n!=1;}',
 'empty_copy_template': 'int n;template<class>struct E{};E<int> g(){++n;return E<int>();}template<class T>E<T> f(){return E<T>(g());}int main(){f<int>();return n!=1;}',
 'closure_statics': 'int f(){auto a=[]()->int{static int n=0;return ++n;};auto b=[]()->int{static int n=0;return ++n;};return a()+b();}int main(){return f()!=2||f()!=4;}',
 'selected_partial_definition': 'template<class T>struct A{template<class U>int f(U);};template<class T>struct A<T*>{template<class U>int f(U);};template<class X>template<class Y>int A<X*>::f(Y n){return sizeof(X)+n;}int main(){A<long*>a;return a.f(3)!=11;}',
 'alias_forward_definition': 'template<class T>struct A;template<class T>using Alias=A<T>;Alias<int>*p;template<class T>struct A{static const int n=7;};int main(){return Alias<int>::n!=7;}',
 'dormant_member': 'template<class T>struct A{static int broken;int f(){return 7;}int bad(){return T::missing;}};template<class T>int A<T>::broken=T::missing;int main(){A<int>a;return a.f()!=7;}',
 'constexpr_then_emit': 'template<int N>constexpr int leaf(){return N;}template<int N>constexpr int f(){return leaf<N>();}static_assert(f<7>()==7,"c");int main(){return f<7>()!=7;}',
}
BAD = {
 'parameter_runtime_value': 'int main(){auto f=[](int n)noexcept(n>0){return n;};return f(7);}',
 'body_local_not_parameter': 'int main(){auto f=[]()noexcept(sizeof(n)==4){int n;};}',
 'unused_template_unknown': 'template<class T>void f(){auto g=[]()noexcept(missing()){return 7;};}int main(){}',
 'contextual_bool_missing': 'struct B{};int main(){auto f=[]()noexcept(B()){return 7;};return f();}',
 'contextual_bool_nonconstant': 'struct B{explicit operator bool()const{return true;}};int main(){auto f=[]()noexcept(B()){return 7;};return f();}',
 'contextual_bool_deleted': 'struct B{explicit operator bool()const=delete;};int main(){auto f=[]()noexcept(B()){return 7;};return f();}',
 'parameter_unavailable_outside': 'int main(){auto f=[](int n)noexcept(sizeof(n)==4){return n;};return n;}',
 'uncaptured_reference': 'int main(){const int n=7;auto f=[]()->const int&{return n;};return f();}',
}
def run(cc, work):
 work.mkdir(parents=True,exist_ok=True); rows=[]
 for good,cases in ((True,GOOD),(False,BAD)):
  for name,source in cases.items():
   src=work/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
   r=subprocess.run([cc,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True,timeout=30)
   row=dict(name=name,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),expected='native' if good else 'reject',compiler_exit=r.returncode,diagnostic=r.stderr,passed=(r.returncode==0 if good else r.returncode==1))
   if good and r.returncode==0:
    b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
    row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
    row['native_exit']=subprocess.run([exe],timeout=30).returncode if b.returncode==0 else None
    row['passed']=b.returncode==0 and row['native_exit']==0
   rows.append(row)
   print(name,'PASS' if row['passed'] else 'FAIL',r.stderr.strip(),flush=True)
 (work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
 return all(row['passed'] for row in rows)
if __name__=='__main__':sys.exit(0 if run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
