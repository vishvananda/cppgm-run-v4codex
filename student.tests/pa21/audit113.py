#!/usr/bin/env python3
"""Independent PA21 composition controls, executed via the supplied object backend."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK = [Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True, exist_ok=True)
cases = {}
cases['evaluated_typeid_initializer_order'] = (
    'namespace std{class type_info;}struct B{virtual ~B(){}};'
    'struct A{int n;const std::type_info&r;};A*target;int observed;B b;'
    'B&read(){observed=target->n;return b;}'
    'void*operator new(unsigned long,void*p){return p;}'
    'int main(){alignas(A)char storage[sizeof(A)]={};target=(A*)storage;'
    'A*p=new(storage)A{7,typeid(read())};return p->n!=7||observed!=7;}')
cases['unevaluated_delete_no_body_demand'] = (
    'template<class T>struct U{~U(){T::missing();}};'
    'template<class T>bool check(T*p){return noexcept(delete p);}'
    'struct N{~N()noexcept{}};struct E{~E()noexcept(false){}};'
    'int main(){return !check((U<int>*)0)||!check((N*)0)||check((E*)0);}')
cases['dependent_new_array_result'] = (
    'template<class T>auto make()->decltype(new T[2]){return new T[2];}'
    'int main(){int*p=make<int>();p[0]=3;p[1]=7;int n=p[0]+p[1];delete[]p;return n!=10;}')
for depth in (1, 2, 9, 32):
    source = 'int live;struct M{M(){++live;}~M(){--live;}};'
    for i in range(depth):
        source += 'struct A%d{int n;%s m;};' % (i, 'M' if not i else 'A%d' % (i-1))
    source += 'A%d make(){return A%d{7};}int main(){{A%d a=make();if(a.n!=7||live!=1)return 1;}return live;}' % (depth-1, depth-1, depth-1)
    cases['nested_omitted_%d' % depth] = source
for count in (2, 9):
    for member_array in (False, True):
        fields = 'M a[%d];' % count if member_array else ''.join('M a%d;' % i for i in range(count))
        cases['deleting_%s_%d' % ('array' if member_array else 'fields', count)] = (
            'int live,drops;bool fail;struct M{M(){++live;}~M()noexcept(false){--live;++drops;if(fail){fail=false;throw 7;}}};'
            'struct B{virtual ~B()noexcept(false){}};struct D:B{' + fields + '};'
            'int main(){B*p=new D;fail=true;try{delete p;}catch(int n){return n!=7||live||drops!=%d;}return 1;}' % count)
for count in (1, 2, 8, 9, 64):
    prefix = ('int live,attempts,drops;struct M{M(){if(++attempts==%d)throw 7;++live;}~M(){--live;++drops;}};' % count)
    for mode, allocation in (('automatic', 'M a[%d];' % count), ('heap', 'M*p=new M[%d];delete[]p;' % count)):
        check = 'return n!=7||live||drops!=%d;' % (count-1)
        cases['source_handler_%s_%d' % (mode, count)] = prefix + 'int run(){try{' + allocation + '}catch(int n){' + check + '}return 1;}int main(){return run();}'
        cases['active_handler_%s_%d' % (mode, count)] = prefix + (
            'int guards;struct G{G(){++guards;}~G(){--guards;}};template<class T>int run(){G g;try{throw 2L;}catch(long){try{' + allocation +
            '}catch(int n){if(guards!=1)return 2;' + check + '}}return 1;}int main(){int r=run<int>();return r||guards;}')
cases['nested_omitted_throw'] = ('int live;struct M{M(){++live;}~M(){--live;}};struct X{X(){throw 7;}};'
    'struct A{M m;X x;};struct B{int n;A a;};B make(){return B{3};}'
    'int main(){try{B b=make();}catch(int n){return n!=7||live;}return 1;}')
cases['single_new_failure'] = ('int allocations;char storage[64];struct M{'
    'static void*operator new(unsigned long){++allocations;return storage;}'
    'static void operator delete(void*){--allocations;}M(){throw 7;}};'
    'int main(){try{M*p=new M;}catch(int n){return n!=7||allocations;}return 1;}')
for count in (1, 9):
    cases['heap_delete_failure_%d' % count] = ('int allocations,live,drops;bool fail;char storage[256];struct M{'
        'static void*operator new[](unsigned long){++allocations;return storage;}'
        'static void operator delete[](void*){--allocations;}M(){++live;}'
        '~M()noexcept(false){--live;++drops;if(fail){fail=false;throw 7;}}};'
        'int main(){M*p=new M[%d];fail=true;try{delete[]p;}catch(int n){return n!=7||allocations||live||drops!=%d;}return 1;}' % (count,count))
cases['single_delete_failure'] = ('int allocations,live;char storage[64];struct M{'
    'static void*operator new(unsigned long){++allocations;return storage;}'
    'static void operator delete(void*){--allocations;}M(){++live;}'
    '~M()noexcept(false){--live;throw 7;}};int main(){M*p=new M;try{delete p;}'
    'catch(int n){return n!=7||allocations||live;}return 1;}')
for array in (False, True):
    op = '[]' if array else ''
    cases['placement_new_failure_'+str(array)] = ('int allocations,argument_calls,seen;char storage[64];int arg(){++argument_calls;return 11;}'
        'struct M{static void*operator new'+op+'(unsigned long,int n){++allocations;seen=n;return storage;}'
        'static void operator delete'+op+'(void*,int n){--allocations;seen+=n;}M(){throw 7;}};'
        'int main(){try{M*p=new(arg())M'+('[2]' if array else '')+';}catch(int n){return n!=7||allocations||argument_calls!=1||seen!=22;}return 1;}')
    cases['unmatched_delete_'+str(array)] = ('int seen;char storage[64];struct M{'
        'static void*operator new'+op+'(unsigned long){return storage;}'
        'static void operator delete'+op+'(void*,int){++seen;}M(){throw 7;}};'
        'int main(){try{M*p=new M'+('[2]' if array else '')+';}catch(int n){return n!=7||seen;}return 1;}')
    for active in (False, True):
        body = ('try{M*p=new(arg())M'+('[9]' if array else '')+';}catch(int n){return n!=7||allocations||argument_calls!=1||seen!=22||live||defaults;}return 1;')
        if active: body = 'G g;try{throw 2L;}catch(long){'+body+'}return 3;'
        cases['placement_defaults_%s_%s' % (array,active)] = (
            'int allocations,argument_calls,seen,live,attempts,defaults,guards;char storage[256];'
            'int arg(){++argument_calls;return 11;}struct G{G(){++guards;}~G(){--guards;}};'
            'struct D{D(){++defaults;}~D(){--defaults;}};struct M{'
            'static void*operator new'+op+'(unsigned long,int n){++allocations;seen=n;return storage;}'
            'static void operator delete'+op+'(void*,int n){--allocations;seen+=n;}'
            'M(const D&d=D()){if(++attempts=='+('7' if array else '1')+')throw 7;++live;}~M(){--live;}};'
            'template<class T>int run(){'+body+'}int main(){int n=run<int>();return n||guards;}')
    cases['sized_delete_failure_'+str(array)] = ('int allocations,bytes,live,drops;bool fail;char storage[256];struct M{'
        'static void*operator new'+op+'(unsigned long n){++allocations;bytes=n;return storage;}'
        'static void operator delete'+op+'(void*,unsigned long n){--allocations;bytes-=n;}M(){++live;}'
        '~M()noexcept(false){--live;++drops;if(fail){fail=false;throw 7;}}};'
        'int main(){M*p=new M'+('[9]' if array else '')+';fail=true;try{delete'+op+' p;}'
        'catch(int n){return n!=7||allocations||bytes||live||drops!='+('9' if array else '1')+';}return 1;}')
rows = []
for name, source in cases.items():
    src = WORK / (name + '.cpp'); src.write_text(source)
    ir, obj, exe = src.with_suffix('.lowir'), src.with_suffix('.o'), WORK/name
    commands = []
    for cmd in ([CC, '--emit-lowir', '-O0', '--validate-lowir', '-o', ir, src],
                [ROOT/'dev/cppgm++-ref', '-c', '-O0', '-o', obj, ir],
                ['g++', '-no-pie', obj, '-o', exe], [exe]):
        try:
            p = subprocess.run(list(map(str, cmd)), capture_output=True, text=True, timeout=30)
            r = dict(argv=list(map(str, cmd)), exit=p.returncode, stdout=p.stdout, stderr=p.stderr)
        except subprocess.TimeoutExpired:
            r = dict(argv=list(map(str, cmd)), exit=124, stderr='timeout')
        commands.append(r)
        if r['exit']: break
    passed = len(commands) == 4 and not commands[-1]['exit']
    rows.append(dict(name=name, source=source, commands=commands, passed=passed))
    print(name, 'PASS' if passed else 'FAIL', r['stderr'].strip(), flush=True)
    (WORK/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(), rows=rows), indent=2)+'\n')
sys.exit(not all(r['passed'] for r in rows))
