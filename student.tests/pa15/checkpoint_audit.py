#!/usr/bin/env python3
"""Cross-handoff PA15 ownership reducers; execute source-generated LowIR."""
from pathlib import Path
import subprocess, tempfile, sys
ROOT = Path(__file__).resolve().parents[2]
CC = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT/'dev/cppgm++'
GOOD = {
 'pack_adl': '''template<class...>struct L{};
namespace N{struct X{};int pick(L<X>*){return 7;}}
int main(){L<N::X>* p=0;return pick(p)-7;}''',
 'explicit_type_pack': '''template<class...T>int f(T...){return 1;}
template<>int f<int,char>(int,char){return 7;}
int main(){return f(1,'a')+f<int,char>(2,'b')-14;}''',
 'explicit_value_pack': '''template<int...I>int f(){return sizeof...(I);}
template<>int f<1,2>(){return 7;}
int main(){return f<1,2>()+f<3>()-8;}''',
 'deduced_type_pack': '''template<class...T>int f(T...){return 1;}
template<>int f(int,char){return 7;}
int main(){return f(1,'a')-7;}''',
 'target_reference_pack': '''template<class...T>int f(T...){return sizeof...(T);}
int main(){int(*p)(int&,long&)=f;int i=0;long j=0;return p(i,j)-2;}''',
 'target_pack_prefix': """template<class...T>int f(T...){return sizeof...(T);}
int main(){int(*p)(int&,char)=f<int&>;int i=0;return p(i,'a')-2;}""",
 'target_pack_return': """template<class R,class...T>R f(T...){return sizeof...(T);}
int main(){long(*p)(int&,long&)=f;int i=0;long j=0;return p(i,j)-2;}""",
 'empty_specialization': """template<class...T>int f(T...){return 1;}
template<>int f<>(){return 7;}int main(){return f()-7;}""",
 'value_pack_adl_exclusion': """namespace N{enum class E{v};}
template<N::E...>struct L{};int pick(L<N::E::v>*){return 7;}
namespace N{int pick(L<E::v>*){return 8;}}
int main(){L<N::E::v>* p=0;return pick(p)-7;}""",
 'local_pack_abi': """template<class...T>struct L{int x;};
template<class T>int use(T* p){return p->x;}
int main(){struct X{};L<X> v;v.x=7;return use(&v)-7;}""",
 'pack_repeated_deduction': '''template<class...>struct L{};
template<class...T>int f(L<T...>,L<T...>){return sizeof...(T);}
int main(){return f(L<int,char>(),L<int,char>())-2;}''',
}
BAD = {
 'class_pack_redeclaration': 'template<class...T>struct C;template<class T>struct C{};',
 'class_scalar_redeclaration': 'template<class T>struct C;template<class...T>struct C{};',

 'specialization_short_prefix': 'template<class...T>int f(T...);template<>int f<int>(){return 0;}',
 'target_short_prefix': 'template<class...T>int f(T...);int(*p)()=f<int>;',
 'target_wrong_return': 'template<class...T>int f(T...);long(*p)(int)=f;',
 'specialization_wrong_reference': 'template<class...T>int f(T...);template<>int f<int&>(int){return 0;}',

 'pack_deduction_conflict': '''template<class...>struct L{};
template<class...T>int f(L<T...>,L<T...>){return sizeof...(T);}
int main(){return f(L<int>(),L<char>());}''',
}
failures = []
with tempfile.TemporaryDirectory(prefix='pa15-audit-') as tmp:
 for name, source in {**GOOD, **BAD}.items():
  src=Path(tmp)/(name+'.cpp');ir=src.with_suffix('.lowir');exe=Path(tmp)/name
  src.write_text(source)
  r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,src],capture_output=True,text=True)
  okay = (r.returncode == 0) == (name in GOOD)
  if okay and name in GOOD:
   r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
   okay = r.returncode == 0
   if okay:
    r=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=r.returncode==0
  print(name, 'PASS' if okay else 'FAIL', r.returncode, r.stderr.strip(),flush=True)
  if not okay:failures.append(name)
assert not failures, failures
print(f'{len(GOOD)} native and {len(BAD)} rejection audit controls passed')
