#!/usr/bin/env python3
"""PA16 audit reducers and cross-owner controls; run explicitly."""
from pathlib import Path
import json, os, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT/'dev/cppgm++'
WORK = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa16-audit/controls'
WORK.mkdir(parents=True, exist_ok=True)
GOOD = {
 # [expr]/12 permits excess precision. Check the chosen x87 model against
 # native execution; do not impose the unmandated single-rounding result.
 'double_round': 'constexpr double b=1.0/9007199254740992.0+1.0/73786976294838206464.0;constexpr double x=1.0+b;static_assert(x==1.0 || x==1.00000000000000022204, "permitted precision");int main(){volatile double y=b;return (1.0+y)==x?0:1;}',
 'double_subtract': 'constexpr double b=1.0/9007199254740992.0+1.0/73786976294838206464.0;constexpr double x=-1.0-b;static_assert(x== -1.0 || x== -1.00000000000000022204, "permitted precision");int main(){volatile double y=b;return (-1.0-y)==x?0:1;}',
 'float_operations': 'constexpr float f(float a,float b){return ((a+b)*b-a)/b;}static_assert(f(2,4)==5.5f, "");int main(){volatile float a=2,b=4;return f(a,b)==5.5f?0:1;}',
 'long_double_operations': 'constexpr long double f(long double a,long double b){return ((a+b)*b-a)/b;}static_assert(f(2,4)==5.5L, "");int main(){volatile long double a=2,b=4;return f(a,b)==5.5L?0:1;}',
 'late_empty_ctor': 'struct A{constexpr A();constexpr explicit operator bool()const{return true;}};constexpr bool f(){return bool(A());}bool early=f();constexpr A::A(){}static_assert(f(), "late definition");int main(){return !early;}',
 'late_empty_template': 'struct A{constexpr A();constexpr explicit operator bool()const{return true;}};template<class T>constexpr bool f(){return bool(T());}bool early=f<A>();constexpr A::A(){}static_assert(f<A>(), "late definition");int main(){return !early;}',
 'late_scalar': 'constexpr int g();constexpr int f(){return g();}int early=f();constexpr int g(){return 7;}static_assert(f()==7, "late definition");int main(){return early!=7;}',
 'spec_then_body': 'template<class T>constexpr int f()noexcept(sizeof(T)>1){return 7;}static_assert(noexcept(f<int>()), "");static_assert(f<int>()==7, "");int main(){return f<int>()-7;}',
 'body_then_spec': 'template<class T>constexpr int f()noexcept(sizeof(T)>1){return 7;}static_assert(f<int>()==7, "");static_assert(noexcept(f<int>()), "");int main(){return f<int>()-7;}',
 'spec_default_body': 'template<class T>int f(T x=T())noexcept(sizeof(T)>1){return x;}static_assert(noexcept(f<int>()), "");int main(){return f<int>();}',
 'default_spec_body': 'template<class T>constexpr int f(T x=T())noexcept(sizeof(T)>1){return x;}static_assert(f<int>()==0, "");static_assert(noexcept(f<int>()), "");int main(){return f<int>();}',
 'spec_multiple': 'template<class T>constexpr int f(T x)noexcept(sizeof(T)>1){return x+1;}static_assert(noexcept(f(1)), "");static_assert(!noexcept(f(char(1))), "");static_assert(f(1)==2 && f(char(3))==4, "");int main(){return f(1)+f(char(3))-6;}',
 'array_strings': 'int main(){constexpr const char* a[]={"a","b"};constexpr const char*b[]={"a","b"};return a==b || a[1][0]!=98 || b[0][0]!=97;}',
 'omitted_exception': 'struct A{A();};struct B{A a;};template<class T>constexpr bool f(){return noexcept(T{});}static_assert(!f<B>(), "");int main(){return 0;}',
 'source_to_native': 'template<class T>constexpr T step(T x)noexcept(sizeof(T)>1){return x+T(0.5);}static_assert(noexcept(step(1.0)), "");static_assert(step(1.0)==1.5, "");int calls;int seed(){return ++calls;}double read(int i){static int n=seed();constexpr double a[]={step(1.0),step(2.0)};constexpr double b[]={step(1.0),step(2.0)};return a!=b?a[i]+b[i]+n:0;}int main(){return read(0)!=4 || read(1)!=6 || calls!=1;}',
 'depth_boundary': 'constexpr int f(int n){return n?1+f(n-1):0;}static_assert(f(511)==511, "");int main(){return 0;}',
}
BAD = {
 'implicit_float_overflow': 'constexpr float x=1.0e300;',
 'implicit_int_overflow': 'constexpr int x=1.0e30;',
 'implicit_unsigned_negative': 'constexpr unsigned x=-1.0;',
 'missing_empty_ctor': 'struct A{constexpr A();constexpr explicit operator bool()const{return true;}};constexpr bool f(){return bool(A());}static_assert(f(), "");',
 'late_effectful_ctor': 'int n;struct A{constexpr A();constexpr explicit operator bool()const{return true;}};constexpr bool f(){return bool(A());}bool early=f();constexpr A::A(){++n;}static_assert(f(), "");',
 'depth_exhaustion': 'constexpr int f(int n){return n?1+f(n-1):0;}static_assert(f(512)==512, "");',
}
rows = []
for name, source in {**GOOD, **BAD}.items():
    src=WORK/(name+'.cpp'); ir=src.with_suffix('.lowir'); exe=src.with_suffix('.exe')
    src.write_text(source)
    r=subprocess.run([CC,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,src],capture_output=True,text=True,timeout=30)
    row=dict(name=name,expected_success=name in GOOD,exit_code=r.returncode,stderr=r.stderr)
    okay=(r.returncode==0)==(name in GOOD)
    if okay and name in GOOD:
        if name=='source_to_native':
            irtext=ir.read_text()
            okay=irtext.count('copyobj')==2 and irtext.count('storage=readonly')==1
        r=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True,timeout=30)
        okay &= r.returncode==0; row['backend_exit']=r.returncode
        if r.returncode==0:
            r=subprocess.run([exe],capture_output=True,text=True,timeout=15)
            row['native_exit']=r.returncode; okay &= r.returncode==0
    row['passed']=okay; rows.append(row)
    print(name, 'PASS' if okay else 'FAIL', flush=True)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows), [r['name'] for r in rows if not r['passed']]
print(f'{len(GOOD)} native and {len(BAD)} rejection audit controls passed')
