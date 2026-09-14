#!/usr/bin/env python3
"""Independent PA15 interactions and volatile constant-read reducers.

N3485 5.19 [expr.const]/2 requires a non-volatile glvalue for constant
lvalue-to-rvalue conversion. Address formation and unevaluated operands do not
perform that conversion. Run with an optional frozen compiler argument.
"""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
CC = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT / 'dev/cppgm++'
FUNCTION = 'constexpr int f(bool read,volatile int n){return read?n:7;}\n'
REFERENCE = 'const int n=3;const volatile int&r=n;\n'
GOOD = {
    'short_circuit_parameter': FUNCTION + 'static_assert(f(false,3)==7,"");int main(){return 0;}',
    'unevaluated_parameter': 'constexpr int f(volatile int n){return sizeof(n);}static_assert(f(3)==4,"");int main(){return 0;}',
    'short_circuit_reference': REFERENCE + 'template<int>struct C{};C<false?r:7> c;int main(){return 0;}',
    'unevaluated_reference': REFERENCE + 'template<int>struct C{};C<sizeof(r)> c;int main(){return 0;}',
    'reference_address': REFERENCE + 'constexpr const volatile int*p=&r;int main(){return p!=&n;}',
    'array_reference_address': REFERENCE + 'int main(){constexpr const volatile int*p[1]={&r};return p[0]!=&n;}',
    'volatile_array_decay': 'volatile int a[2];int main(){constexpr volatile int*p[1]={a};return p[0]!=a;}',
    'nonvolatile_reference': 'const int n=3;const int&r=n;template<int N>struct C{static const int value=N;};static_assert(C<r>::value==3,"");int main(){return 0;}',
    'volatile_initializer': 'int main(){constexpr volatile int n=3;return n-3;}',
    'runtime_reference': REFERENCE + 'int main(){return r-3;}',
    'static_reference_read': REFERENCE + 'int x=r;int main(){return x-3;}',
    'local_static_reference_read': 'int main(){const int n=3;const volatile int&r=n;static int x=r;return x-3;}',
    'local_reference_read': 'int main(){const int n=3;const volatile int&r=n;return r-3;}',
    'selected_pack_constant': '''template<int...N>constexpr int f(){return sizeof...(N);}
template<>constexpr int f<2,3>(){return 9;}template<int N>struct C{static const int value=N;};
static_assert(C<f<2,3>()>::value==9,"");static_assert(C<f<1>()>::value==1,"");int main(){return 0;}''',
    'nested_pack_arrays': '''template<int...N>int sum(){constexpr int a[]={N...};int s=0;
for(unsigned i=0;i<sizeof...(N);++i)s+=a[i];return s;}
int main(){return sum<1,2,3>()+sum<4,5>()-15;}''',
    'selected_class_constant': '''template<class T>struct C;template<>struct C<int>{
static constexpr int f(int n){return n+7;}};template<int N>struct V{static const int value=N;};
static_assert(V<C<int>::f(3)>::value==10,"");int main(){return 0;}''',
    'vtable_pack_demand': '''template<class...T>struct C{virtual int f(){return sizeof...(T);}int unused(){return T::missing;}};
int main(){C<int,char> c;C<int,char>*p=&c;return p->f()-2;}''',
}
BAD = {
    'read_parameter_ast': FUNCTION + 'static_assert(f(true,3)==3,"");int main(){return 0;}',
    'read_parameter_query': FUNCTION + 'template<int>struct C{};C<f(true,3)> c;int main(){return 0;}',
    'read_reference_ast': REFERENCE + 'static_assert(r==3,"");int main(){return 0;}',
    'read_reference_query': REFERENCE + 'template<int>struct C{};C<r> c;int main(){return 0;}',
    'read_reference_cast': REFERENCE + 'template<int>struct C{};C<static_cast<int>(r)> c;int main(){return 0;}',
    'read_reference_array': REFERENCE + 'int main(){constexpr int a[1]={r};return 0;}',
    'read_reference_nested': REFERENCE + 'int main(){constexpr int a[1][1]={{r}};return 0;}',
    'read_reference_call': REFERENCE + 'constexpr int f(){return r;}constexpr int x=f();int main(){return 0;}',
    'volatile_binding_query': 'const volatile int n=3;template<int>struct C{};C<n> c;int main(){return 0;}',
    'volatile_member_query': 'template<class T>struct C{static const volatile int n=3;};template<int>struct V{};V<C<int>::n> v;int main(){return 0;}',
    'selected_body_rejection': 'template<class T>struct C;template<>struct C<int>{int unused(){static_assert(false,"");return 0;}};int main(){return 0;}',
}
VOLATILE_LOADS = {'volatile_initializer', 'runtime_reference', 'static_reference_read',
                  'local_static_reference_read', 'local_reference_read'}
failed = []
with tempfile.TemporaryDirectory(prefix='pa15-final-', dir=os.environ.get('RALPH_ARTIFACT_DIR')) as td:
    for name, source in {**GOOD, **BAD}.items():
        src = Path(td) / (name + '.cpp')
        ir, exe = src.with_suffix('.lowir'), src.with_suffix('.exe')
        src.write_text(source)
        r = subprocess.run([CC, '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, src],
                           capture_output=True, text=True, timeout=20)
        okay = (r.returncode == 0) == (name in GOOD)
        if okay and name in VOLATILE_LOADS:
            okay = 'load volatile i32' in ir.read_text()
        if okay and name in GOOD:
            r = subprocess.run([ROOT / 'dev/lowir2native-ref', '-O0', '-o', exe, ir],
                               capture_output=True, text=True, timeout=20)
            okay = r.returncode == 0
            if okay:
                r = subprocess.run([exe], capture_output=True, text=True, timeout=10)
                okay = r.returncode == 0
        print(name, 'PASS' if okay else 'FAIL', r.returncode, r.stderr.strip(), flush=True)
        if not okay:
            failed.append(name)
assert not failed, failed
print(f'{len(GOOD)} native and {len(BAD)} rejection final-audit controls passed')
