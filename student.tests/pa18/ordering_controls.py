#!/usr/bin/env python3
"""PA18 ordering/address controls; run CC WORK, checks real generated execution."""
from pathlib import Path
import hashlib, itertools, json, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
GOOD={
 'nondeduced_explicit':'template<class T>struct A{typedef int type;};template<class T>int f(typename A<T>::type){return 1;}template<class T>int f(T){return 2;}int main(){return f<int>(1)!=1;}',
 'address_nested_cv':'template<class T>int f(T*){return 1;}template<class T>int f(const T*){return 2;}int main(){int(*p)(const int*)=f;return p(0)!=2;}',
 'address_distinct_defaults':'template<class T>int f(T*,int=0){return 1;}template<class T>int f(T){return 2;}int main(){int(*p)(int*,int)=f;return p(0,1)!=1;}',
 'member_nonmember':'struct A{template<class T>int operator*(T*){return 1;}};template<class T,class U>int operator*(T&,U){return 2;}int main(){A a;int n;return a*&n!=1;}',
 'address_return':'int value;template<class T>T f(int){return 0;}template<class T>T* f(int){return &value;}int main(){int*(*p)(int)=f;return p(0)!=&value;}',
 'pointer_cv':'template<class T>int f(T*){return 1;}template<class T>int f(const T*){return 2;}int main(){int n;const int*p=&n;return f(p)!=2||f(&n)!=1;}',
 'nested_cv':'template<class T>struct A{};template<class T>int f(A<T>){return 1;}template<class T>int f(A<const T>){return 2;}int main(){return f(A<const int>())!=2||f(A<int>())!=1;}',
 'array_cv':'template<class T,int N>int f(T(&)[N]){return 1;}template<class T,int N>int f(const T(&)[N]){return 2;}int main(){int a[3];const int b[3]={};return f(a)!=1||f(b)!=2;}',
 'nested_array_cv':'template<class T,int M,int N>int f(T(&)[M][N]){return 1;}template<class T,int M,int N>int f(const T(&)[M][N]){return 2;}int main(){int a[2][3];const int b[2][3]={};return f(a)!=1||f(b)!=2;}',
 'reference_kind':'template<class T>int f(T&){return 1;}template<class T>int f(T&&){return 2;}int main(){int n;return f(n)!=1||f(1)!=2;}',
 'reference_cv':'template<class T>int f(T&){return 1;}template<class T>int f(const T&){return 2;}int main(){int n;const int k=1;return f(n)!=1||f(k)!=2;}',
 'default_ignored':'template<class T>int f(T*,int=0){return 1;}template<class T>int f(T){return 2;}int main(){int n;return f(&n)!=1;}',
 'default_fixed':'template<class T>int f(T,bool,bool=false){return 1;}template<class T,class U>int f(T,U){return 2;}int main(){return f(3,true)!=1;}',
 'constructor_default':'int chosen;struct A{template<class T>A(const T&){chosen=1;}template<class T>A(T&&,int=0){chosen=2;}};int main(){const int n=1;A a(n);return chosen!=1;}',
 'operator_default':'struct A{template<class T>int operator()(T*,int=0){return 1;}template<class T>int operator()(T){return 2;}};int main(){A a;int n;return a(&n)!=1;}',
 'query_operator_default':'struct A{template<class T>char operator()(T*,int=0);template<class T>long operator()(T);};int main(){A a;int n;return sizeof(a(&n))!=1;}',
 'member_lref':'struct A{template<class T>int operator=(T&){return 1;}template<class T>int operator=(T&&){return 2;}};int main(){A a;int n;return (a=n)!=1;}',
 'packs':'template<class...T>int f(T...){return 1;}template<class T,class...U>int f(T,U...){return 2;}template<class T,class U>int f(T,U){return 3;}int main(){return f()!=1||f(1)!=2||f(1,2)!=3||f(1,2,3)!=2;}',
 'pack_empty_tail':'template<class T>int f(T){return 1;}template<class T,class...U>int f(T,U...){return 2;}int main(){return f(1)!=1||f(1,2)!=2;}',

 'unused_head':'template<class T>int f(int){return 1;}template<class T,class U>int f(U){return 2;}int main(){return f<void>(1)!=1;}',
 'address_ordinary':'template<class T>int f(T){return 1;}int f(int){return 2;}int main(){int(*p)(int)=f;return p(0)!=2;}',
 'address_pointer':'template<class T>int f(T){return 1;}template<class T>int f(T*){return 2;}int main(){int(*p)(int*)=f;return p(0)!=2;}',

 'address_reference':'template<class T>int f(T){return 1;}template<class T>int f(T*){return 2;}int main(){int(&p)(int*)=f;return p(0)!=2;}',
 'address_pack':'template<class...T>int f(T...){return 1;}template<class T>int f(T){return 2;}int main(){int(*p)(int)=f;return p(0)!=2;}',
 'address_static':'struct A{template<class T>static int f(T){return 1;}static int f(int){return 2;}};int main(){int(*p)(int)=A::f;return p(0)!=2;}',
 'address_deleted_unselected':'template<class T>int f(T)=delete;int f(int){return 2;}int main(){int(*p)(int)=f;return p(0)!=2;}',
 'address_body_dormant':'template<class T>int f(T){return T::missing;}int f(int){return 2;}int main(){int(*p)(int)=f;return p(0)!=2;}',
 'explicit_specialization':'template<class T>int f(T){return 1;}template<class T>int f(T*){return 2;}template<>int f(int*){return 3;}int main(){int(*p)(int*)=f;return p(0)!=3;}',
}
# Exercise winner verification after a tied prefix in every declaration order.
for i,order in enumerate(itertools.permutations([
 'template<class T,class U>int f(T*,U){return 1;}',
 'template<class T,class U>int f(T,U*){return 2;}',
 'template<class T,class U>int f(T*,U*){return 3;}'])):
 GOOD['address_permutation_'+str(i)]=''.join(order)+'int main(){int(*p)(int*,int*)=f;return p(0,0)!=3;}'
 GOOD['call_permutation_'+str(i)]=''.join(order)+'int main(){int n;return f(&n,&n)!=3;}'
BAD={
 'crossed_reference_order':'template<class T,class U>int f(const T&,U*){return 1;}template<class T,class U>int f(T&,const U*){return 2;}int main(){const int n=0;const int*p=0;return f(n,p);}',

 'address_return_conflict':'template<class T>T f(T){return 1;}template<class T>int f(T){return 2;}int main(){int(*p)(int)=f;return p(0)!=2;}',
 'nondeduced_other':'template<class T>struct A{typedef int type;};template<class T>int f(T,typename A<T>::type){return 1;}template<class T,class U>int f(T,U){return 2;}int main(){return f(1,2)!=1;}',
 'incomparable_cv':'template<class T>int f(const T*){return 1;}template<class T>int f(volatile T*){return 2;}int main(){const volatile int*p=0;return f(p);}',
 'crossed_patterns':'template<class T,class U>int f(T*,U){return 1;}template<class T,class U>int f(T,U*){return 2;}int main(){int n;return f(&n,&n);}',
 'crossed_conversions':'template<class T>int f(T*,int){return 1;}template<class T>int f(const T*,long){return 2;}int main(){int n;return f(&n,0L);}',
 'default_not_ordering':'template<class T>int f(T,int=0){return 1;}template<class T>int f(T){return 2;}int main(){return f(1);}',
 'address_ambiguous':'template<class T,class U>int f(T*,U){return 1;}template<class T,class U>int f(T,U*){return 2;}int main(){int(*p)(int*,int*)=f;return p(0,0);}',
 'address_deleted_selected':'template<class T>int f(T){return 1;}int f(int)=delete;int main(){int(*p)(int)=f;return p(0);}',
 'address_nonstatic':'struct A{int f(int){return 1;}template<class T>int f(T){return 2;}};int main(){int(*p)(int)=A::f;return p(0);}',
 'address_return_only_call':'template<class T>T f(T){return 1;}template<class T>int f(T){return 2;}int main(){return f(1);}',
}
def run(cc,work):
 work.mkdir(parents=True,exist_ok=True);rows=[]
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
   rows.append(row);print(name,'PASS' if row['passed'] else 'FAIL',r.stderr.strip(),flush=True)
 (work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
 return all(r['passed'] for r in rows)
if __name__=='__main__':sys.exit(0 if run(Path(sys.argv[1]).resolve(),Path(sys.argv[2])) else 1)
