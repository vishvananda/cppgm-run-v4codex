#!/usr/bin/env python3
"""Defined semantic reducers and structural checks for PA19 oracle revisions."""
from pathlib import Path
import sys,re
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'pa18'))
import ordering_controls as runner
runner.GOOD={
 'array_identity':'template<class T>int use(T&a,T&b){a[0]=9;return b[0];}int main(){int a[]={1,2};int b[]={1,2};return use(a,b)!=1||a==b||a[1]!=2;}',
 'array_string':'template<class T,int N>int len(T(&)[N]){return N;}int main(){char a[]="abc";char b[]="abc";a[0]=0;return len(a)!=4||b[0]!=97||b[3]!=0;}',
 'array_short':'template<class T,int N>int len(T(&)[N]){return N;}int main(){unsigned short a[]={0};unsigned short b[]={0};a[0]=3;return len(a)!=1||b[0]!=0||a==b;}',
 'dormant_static':'int hits;int init(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=init();int main(){X<int>x;return hits;}',
 'dormant_invalid_static':'template<class T>struct X{static const int n;};template<class T>const int X<T>::n=T::missing;int main(){X<int>x;}',
 'demanded_static':'int hits;int init(){return ++hits;}template<class T>struct X{static const int n;};template<class T>const int X<T>::n=init();int main(){return X<int>::n!=1||hits!=1;}',
 'sizeof_static':'int hits;int init(){return ++hits;}template<class T>struct X{static const int n[3];};template<class T>const int X<T>::n[3]={init(),2,3};static_assert(sizeof(X<int>::n)==3*sizeof(int),"");int main(){return hits;}',
 'constant_reference_order':'struct X{int n;};extern const X& ref;int read(){return ref.n;}int early=read();const X object={7};const X&ref=object;int main(){return early!=7||&ref!=&object;}',
 'discarded_reference':'volatile int value=7;int calls;volatile int& get(){++calls;return value;}int main(){(void)get();return calls!=1;}',
 'discarded_id':'volatile int value=7;int main(){(void)value;return 0;}',
 'specialization_no_effect':'template<class T>int f(T){return 1;}template<>int f<int>(int){return 7;}template int f<int>(int);int main(){return f(0)!=7;}',
 'constant_member_object':'struct X{int n;constexpr X(int x):n(x){}};template<class>struct O{template<int N>static constexpr X v=X(N);};int read(){return O<int>::v<7>.n;}int early=read();int main(){return early!=7;}',
 'dependent_sizeof_conversion':'template<class T,int N>long f(){return sizeof(T)+N;}int main(){return f<char,2>()!=3||f<int,-1>()!=3;}',
}
runner.BAD={}
if __name__=='__main__':
 work=Path(sys.argv[2]);ok=runner.run(Path(sys.argv[1]).resolve(),work)
 if ok:
  for name in ('array_identity','array_string','array_short'):
   ir=(work/(name+'.lowir')).read_text();assert ir.count('copyobj ')==2 and 'storage=readonly' in ir
  a=(work/'discarded_reference.lowir').read_text();b=(work/'discarded_id.lowir').read_text()
  returned=re.search(r'(%\w+) = call ptr @get\(\)',a)[1]
  assert not re.search(r'load (?:volatile )?i32 '+re.escape(returned)+r'(?!\w)',a)
  assert re.search(r'load volatile i32 @\S*value',b)
  assert 'object_root=yes' not in (work/'specialization_no_effect.lowir').read_text()
  ref=(work/'constant_reference_order.lowir').read_text();assert re.search(r'global @\S*ref : ptr[^\n]*= addr @',ref)
  assert 'convert sext i64 i32 2' in (work/'dependent_sizeof_conversion.lowir').read_text()
 sys.exit(0 if ok else 1)
