#!/usr/bin/env python3
"""PA25 audit reducers: exact scalar facts and retained native definitions."""
import json, pathlib, subprocess, sys

root = pathlib.Path(__file__).resolve().parents[2]
compiler = pathlib.Path(sys.argv[1]).resolve()
out = pathlib.Path(sys.argv[2]).resolve()
out.mkdir(parents=True, exist_ok=True)
results = []

def command(args):
    try:
        p = subprocess.run(list(map(str, args)), capture_output=True, text=True, timeout=15)
        return p.returncode, p.stderr
    except subprocess.TimeoutExpired:
        return 'timeout', ''

def check(name, args, expected=0):
    code, diagnostic = command(args)
    ok = code == expected
    results.append(dict(name=name, passed=ok, actual=code, expected=expected, diagnostic=diagnostic))
    print(('PASS' if ok else 'FAIL'), name, code, flush=True)
    return ok

def source(name, text):
    p = out / name
    p.write_text(text + '\n')
    return p

reject = {
    'nan-int': 'constexpr int x=(int)__builtin_nan("");',
    'nan-wide': 'constexpr unsigned __int128 x=(unsigned __int128)__builtin_nanl("");',
    'narrow-float': 'float x{(U(1)<<100)+1};',
    'narrow-double': 'double x{(U(1)<<100)+1};',
    'narrow-long-double': 'long double x{(U(1)<<100)+1};',
    'fixed-enum-range': 'enum E:unsigned char{a=256};',
    'enum-overflow': 'enum E:U{a=~U(0),b};',
    'enum-no-type': 'enum E{a=-1,b=~U(0)};',
}
for name, text in reject.items():
    src = source(name+'.cc', 'using U=unsigned __int128;'+text+'int main(){return 0;}')
    check(name, [compiler, '-c', '-o', out/(name+'.obj'), src], 1)

view = source('builtin-view.cc', 'int main(){__builtin_nanl("");__builtin_inf();return 0;}')
rendered = out/'builtin-view.semantics'
if check('builtin-semantics', [compiler, '--emit-semantics', '-o', rendered, view]):
    lines = rendered.read_text().splitlines()
    results.append(dict(name='builtin-rendered-values', passed=any('nan' in s for s in lines) and any('inf' in s for s in lines)))

cases = {
    'enum-widen': 'enum E{a=5,b=(U(1)<<100)};static_assert(a==5 && b==(U(1)<<100),"values");int main(){return U(a)!=5 || U(b)!=(U(1)<<100);}',
    'enum-no-shrink': 'enum E{a=(U(1)<<100),b=5};int main(){return U(a)!=(U(1)<<100) || b!=5;}',
    'enum-signed': 'enum E{a=-1,b=(I(1)<<100),c=-7};int main(){return a!=-1 || I(b)!=(I(1)<<100) || c!=-7;}',
    'enum-declared-types': 'enum E{a=1L,b,c=sizeof(b)};static_assert(c==sizeof(long),"preceding type");int main(){return 0;}',
    'enum-increment': 'enum E{a=2147483647,b};static_assert(b==2147483648ULL,"increment");int main(){return b!=2147483648ULL;}',
    'enum-wide-increment': 'enum E{a=(U(1)<<127)-1,b};int main(){return U(b)!=(U(1)<<127);}',
    'exact-floating': 'float a{U(1)<<100};double b{U(1)<<100};long double c{(U(1)<<100)+(U(1)<<40)};int main(){return U(a)!=(U(1)<<100) || U(b)!=(U(1)<<100) || U(c)!=((U(1)<<100)+(U(1)<<40));}',
    'template-statement-wide': 'template<int N> U f(){enum E{small=N,large=(U(1)<<100)};return ({U x=large;x+small;});}int main(){return f<5>()!=((U(1)<<100)+5) || f<7>()!=((U(1)<<100)+7);}',
    'enum-template-positive': 'enum class E:U{a=U(1)<<100};template<E N>E f(){return N;}int main(){return f<E::a>()==E::a?0:1;}',
    'enum-template-negative': 'enum class E:I{a=-(I(1)<<100)};template<E N>E f(){return N;}int main(){return f<E::a>()==E::a?0:1;}',
    'enum-template-unsigned': 'enum class E:unsigned long{a=18446744073709551615UL};template<E N>E f(){return N;}int main(){return f<E::a>()==E::a?0:1;}',
}
for name, text in cases.items():
    src = source(name+'.cc', 'using U=unsigned __int128;using I=__int128;'+text)
    obj, exe = out/(name+'.obj'), out/(name+'.program')
    if check(name+'-direct', [compiler, '-o', exe, src]):
        check(name+'-run', [exe])
    if check(name+'-object', [compiler, '-c', '-o', obj, src]):
        if check(name+'-link', [compiler, '-o', exe, obj]):
            check(name+'-object-run', [exe])

# The Itanium contract encodes the enum type name and its exact numeric value.
for label, underlying, value in [('positive', 'unsigned __int128', 1<<100),
                                 ('negative', '__int128', -(1<<100)),
                                 ('unsigned', 'unsigned long', (1<<64)-1)]:
    name = 'enum-template-'+label
    ll = out/(name+'.lowir')
    expected = '_Z1fIL1E'+('n' if value < 0 else '')+str(abs(value))+'EES0_v'
    if check(name+'-abi', [compiler, '--emit-lowir', '-o', ll, out/(name+'.cc')]):
        ok = expected in ll.read_text()
        results.append(dict(name=name+'-symbol', passed=ok, expected=expected))
    facts = source(name+'.facts', f'let-arg V value named:E {value}\ntype template Box V')
    encoded = out/(name+'.mangled')
    if check(name+'-facts', [root/'dev/abimangle', '-o', encoded, facts]):
        expected_fact = '3BoxIL1E'+('n' if value < 0 else '')+str(abs(value))+'EE'
        results.append(dict(name=name+'-fact-symbol', passed=encoded.read_text().strip()==expected_fact, expected=expected_fact))
    common = (out/(name+'.cc')).read_text().split('int main()')[0]
    helper = source(name+'-host.cc', common+'template E f<E::a>();')
    user = source(name+'-use.cc', common+'extern template E f<E::a>();int main(){return f<E::a>()==E::a?0:1;}')
    obj, exe = out/(name+'-host.o'), out/(name+'-host.program')
    if check(name+'-host-object', ['g++', '-std=c++11', '-c', '-o', obj, helper]):
        if check(name+'-host-link', [compiler, '-o', exe, obj, user]):
            check(name+'-host-run', [exe])

foreign = {
    'weak-got': ('extern int absent(void);__attribute__((weak)) int choose(void){return absent();}',
                 'extern "C" int choose(){return 9;}int main(){return choose()-9;}', 0),
    'weak-alias': ('extern int target(void);__attribute__((weak)) int choose(void){return target();}int retained(void) __attribute__((alias("choose")));',
                   'extern "C" int choose(){return 9;}extern "C" int retained();extern "C" int target(){return 11;}int main(){return retained()-11;}', 0),
    'alias-needs-target': ('extern int absent(void);__attribute__((weak)) int choose(void){return absent();}int retained(void) __attribute__((alias("choose")));',
                          'extern "C" int choose(){return 9;}extern "C" int retained();int main(){return retained();}', 1),
    'shared-got': ('extern int target(void);__attribute__((weak)) int choose(void){return target();}int retained(void){return target();}',
                   'extern "C" int choose(){return 9;}extern "C" int retained();extern "C" int target(){return 11;}int main(){return retained()-11;}', 0),
}
for name, (helper, user, expected) in foreign.items():
    c = source(name+'.c', helper)
    cc = source(name+'.cc', user)
    for flags in ([], ['-fno-plt']):
        label = name+('-got' if flags else '-plt')
        obj, exe = out/(label+'.o'), out/(label+'.program')
        # Host compilation creates explicit foreign test input only.
        if not check(label+'-helper', ['cc', '-c', *flags, '-o', obj, c]):
            continue
        for index, inputs in enumerate(((obj, cc), (cc, obj))):
            tag = label+str(index)
            if check(tag+'-link', [compiler, '-o', exe, *inputs], expected) and not expected:
                check(tag+'-run', [exe])

(out/'results.json').write_text(json.dumps(results, indent=2)+'\n')
print(f'{sum(r["passed"] for r in results)}/{len(results)} checks passed')
sys.exit(any(not r['passed'] for r in results))
