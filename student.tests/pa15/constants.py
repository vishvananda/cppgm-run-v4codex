#!/usr/bin/env python3
"""Exercise constant boundaries and the independent typed-query evaluator."""
import pathlib, subprocess, tempfile, sys
root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root/'dev/cppgm++'
cases = {
    'limits': (True, '''
static_assert((1 << 31) == (-2147483647-1), "sign bit");
static_assert((1LL << 63) == (-9223372036854775807LL-1), "sign bit");
static_assert((0xffffffffffffffffULL << 63) == 0x8000000000000000ULL, "modulo");
static_assert((0xffffffffU << 1) == 0xfffffffeU, "modulo");
static_assert((-7 % 3) == -1 && (-7 / 3) == -2, "truncation");
static_assert(true || ((-2147483647-1) % -1), "short circuit");
static_assert((false ? 1073741824 << 2 : 7) == 7, "conditional");
static_assert(short(42) + unsigned(3) == 45, "casts");
static_assert(char16_t(65535) == 65535 && wchar_t(12) == 12, "casts");
static_assert('abcd' == 0x61626364 && 'abcde' == 0x62636465, "multicharacter");
int main() { return 0; }
'''),
    'mod32': (False, 'static_assert((-2147483647-1) % -1 == 0, "overflow");'),
    'mod64': (False, 'static_assert((-9223372036854775807LL-1) % -1 == 0, "overflow");'),
    'shift32': (False, 'static_assert((1073741824 << 2) == 0, "overflow");'),
    'shift64': (False, 'static_assert((4611686018427387904LL << 2) == 0, "overflow");'),
    'negative_shift': (False, 'static_assert((-1 << 1) == -2, "negative");'),
    'shift_width': (False, 'static_assert((1U << 32) == 1, "width");'),
    'query_limits': (True, '''template<class T> struct A {
  int x[(sizeof(T) == 4 ? (1U << 31) >> 30 : 1)];
}; static_assert(sizeof(A<int>) == 8, "query"); int main() { return 0; }'''),
    'query_mod': (False, '''template<class T> struct A {
  int x[(sizeof(T) ? (-2147483647-1) % -1 : 1) + 1];
}; int main() { A<int> a; }'''),
    'query_shift': (False, '''template<class T> struct A {
  int x[(sizeof(T) ? (1073741824 << 2) : 1) + 1];
}; int main() { A<int> a; }'''),
}
with tempfile.TemporaryDirectory(prefix='pa15-constants-') as td:
    for name, (success, source) in cases.items():
        src, out = pathlib.Path(td)/f'{name}.cpp', pathlib.Path(td)/f'{name}.lowir'
        src.write_text(source)
        p = subprocess.run([str(compiler), '--emit-lowir','-O0','-o',str(out),str(src)], capture_output=True)
        assert (p.returncode == 0) == success, (name, p.returncode, p.stderr.decode())
        if success:
            text = out.read_text()
            assert not any(line.startswith('global ') and 'inline_hint' in line for line in text.splitlines())
        print(name, 'pass')
print(f'{len(cases)} constant groups passed')
