#!/usr/bin/env python3
"""Explicit typed call, growth, escape and exceptional-edge reducers for PA32."""
import pathlib, random, subprocess, tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
    p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
    assert p.returncode==0,(args,p.returncode,p.stderr[-3000:])
    return p
functions=[]; checks=[]
def add(body,expected,args='',params='',result='i64'):
    name='case'+str(len(checks)); functions.append(f'function @{name}({params}) -> {result} {{\n{body}\n}}')
    checks.append((name,result,args,expected))
rng=random.Random(209)
for ty,bits,signed in [('i8',8,True),('u8',8,False),('i16',16,True),('u16',16,False),('i32',32,True),('u32',32,False),('i64',64,True)]:
    for n in range(8):
        a,b=rng.randrange(-50000,50000),rng.randrange(-50000,50000); pick=rng.randrange(4)
        v=(a if pick else b)%(1<<bits)
        if signed and v>=1<<(bits-1): v-=1<<bits
        add(f'''block ^entry: branch %pick, ^left, ^right
block ^left: %a = copy {ty} %a return {ty} %a
block ^right: %b = copy {ty} %b return {ty} %b''',v,f'{pick}, {a}, {b}',f'%pick : i64, %a : {ty}, %b : {ty}',ty)
add('''slot $local : i64
block ^entry: store i64 %a, $local %a = copy i64 99
 %saved = load i64 $local return i64 %saved''',7,'7','%a : i64')
# Typed parameter copies must round to f32, not retain a wider actual.
add('''block ^entry: %v = convert fptosi i64 f32 %x return i64 %v''',16777216,'16777217.0','%x : f32')
for ty in ['f32','f64','f80']:
    add(f'''block ^entry: %inverse = binary div {ty} 1.0, %x
 %negative = cmp lt {ty} %inverse, 0.0 return i64 %negative''',1,'-0.0',f'%x : {ty}')
functions += ['''function @object_parameter(%v : obj<16x8>) -> i64 {
block ^entry: %a = load i64 %v store i64 999, %v return i64 %a
}
function @object_result(%condition : i64, %value : i64) -> obj<16x8> {
slot $home : obj<16x8>
block ^entry: store i64 %value, $home branch %condition, ^left, ^right
block ^left: return obj<16x8> $home
block ^right: store i64 23, $home return obj<16x8> $home
}''']
add('''slot $source : obj<16x8>
slot $result : obj<16x8>
block ^entry:
 zeroinit 16x8 $source
 store i64 17, $source
 %a = call i64 @object_parameter($source)
 %b = load i64 $source
 %obj = call obj<16x8> @object_result(0, 19)
 copyobj 16x8 %obj, $result
 %c = load i64 $result
 %ab = binary add i64 %a, %b
 %abc = binary add i64 %ab, %c
 return i64 %abc''',57)
# Original generated-looking names and a call result that aliases its argument.
add('''slot $inline1 : i64
block ^entry:
 %inline2 = copy i64 5
 %inline2 = call i64 @identity(%inline2)
 return i64 %inline2''',5)
functions += ['function @identity(%x : i64) -> i64 { block ^entry: return i64 %x }']
main=['function @main() -> i64 [role=entry] { block ^entry:', '%bad0 = const i64 0']
for n,(name,ty,args,expected) in enumerate(checks):
    main += [f'%v{n} = call {ty} @{name}({args})',f'%test{n} = cmp ne {ty} %v{n}, {expected}',f'%bad{n+1} = binary or i64 %bad{n}, %test{n}']
main += [f'return i64 %bad{len(checks)}','}']
with tempfile.TemporaryDirectory(prefix='pa32-calls-') as directory:
    tmp=pathlib.Path(directory); source=tmp/'input.lowir'; source.write_text('\n'.join(functions+main))
    for level in range(4):
        output=tmp/f'o{level}.lowir'
        run(ROOT/'dev/lowiropt',f'-O{level}','-o',output,source)
        run(ROOT/'dev/lowir','-o',tmp/'validated.lowir',output)
        run(ROOT/'dev/lowir2native','-o',tmp/'native',output); run(tmp/'native')
        run(ROOT/'dev/cppgm++','-O0','-o',tmp/'object',output);run(tmp/'object')
    # Source ABI, real unwinding, nested cleanup and actual mutable parameters.
    cpp=tmp/'eh.cpp';cpp.write_text('''int count;
struct Guard { int n; Guard(int x):n(x){} ~Guard(){ count+=n; } };
int leaf(int n){ if(n) throw n; return 7; }
int wrapper(int n){ return leaf(n)+1; }
int protected_call(int n) { Guard a(1); int result=0; try { Guard b(2); result=wrapper(n); }
 catch(int x) { Guard c(4); result=x+10; } return result; }
int safe(int n) noexcept { return n+1; }
int nested_safe(int n) { try { return safe(n); } catch(...) {return -1;} }
int main(){int a=protected_call(3); int b=protected_call(0);return a!=13||b!=8||count!=10||nested_safe(8)!=9;}
''')
    for level in range(4):
        run(ROOT/'dev/cppgm++',f'-O{level}','-gline-tables-only','-o',tmp/'eh',cpp);run(tmp/'eh')
        run(ROOT/'dev/cppgm++','--emit-lowir','-O0','-gline-tables-only','-o',tmp/'eh.lowir',cpp)
        run(ROOT/'dev/cppgm++',f'-O{level}','-gline-tables-only','-o',tmp/'replayed',tmp/'eh.lowir');run(tmp/'replayed')
    # Closed-world constants: internal/noinline is eligible, address escape,
    # exported aliases, conflicting actuals and parameter mutation are not.
    guards='''function @fixed(%mode : u8) -> i64 [binding=internal, no_inline=yes] {
 block ^entry: %v = copy i64 %mode return i64 %v }
function @mutable(%mode : i64) -> i64 [binding=internal, no_inline=yes] {
 block ^entry: %mode = binary add i64 %mode, 2 return i64 %mode }
function @differing(%mode : i64) -> i64 [binding=internal, no_inline=yes] {
 block ^entry: return i64 %mode }
function @escaped(%mode : i64) -> i64 [binding=internal, no_inline=yes] {
 block ^entry: return i64 %mode }
function @aliased(%mode : i64) -> i64 [binding=internal, no_inline=yes] {
 block ^entry: return i64 %mode }
alias object exported_alias = @aliased
function @main() -> i64 [role=entry] {
 block ^entry:
 %a = call i64 @fixed(257)
 %b = call i64 @mutable(3)
 %c = call i64 @differing(2)
 %d = call i64 @differing(4)
 %address = addr @escaped
 %e = call i64 %address(8) as (%x : i64) -> i64
 %f = call i64 @escaped(9)
 %g = call i64 @aliased(10)
 %s1 = binary add i64 %a, %b
 %s2 = binary add i64 %c, %d
 %s3 = binary add i64 %e, %f
 %s4 = binary add i64 %s1, %s2
 %s5 = binary add i64 %s3, %g
 %s6 = binary add i64 %s4, %s5
 %bad = cmp ne i64 %s6, 39
 return i64 %bad }
'''
    source.write_text(guards)
    for level in range(4):
        run(ROOT/'dev/lowiropt',f'-O{level}','-o',tmp/'guards.lowir',source)
        run(ROOT/'dev/lowir2native','-o',tmp/'guards',tmp/'guards.lowir');run(tmp/'guards')
        text=(tmp/'guards.lowir').read_text()
        if level>=2:
            fixed=text.split('function @fixed')[1].split('function @mutable')[0]
            assert 'return i64 1' in fixed
            for name in ['escaped','aliased','differing']:
                body=text.split('function @'+name)[1].split('}')[0]
                assert 'return i64 %mode' in body
    # Strongly recursive groups must not expand; nested acyclic fan-out must
    # exhaust a linear budget, terminate and keep structurally valid calls.
    source.write_text('''function @recursive(%x : i64) -> i64 { block ^entry:
 %r = call i64 @recursive(%x) return i64 %r }
'''+'\n'.join(f'''function @chain{n}(%x : i64) -> i64 {{ block ^entry:
 %a = call i64 @chain{n+1}(%x)
 %b = call i64 @chain{n+1}(%a) return i64 %b }}''' for n in range(80))+
'function @chain80(%x : i64) -> i64 { block ^entry: return i64 %x }')
    stats=run(ROOT/'dev/lowiropt','-O1','--stats','-o',tmp/'stress.lowir',source)
    run(ROOT/'dev/lowir','-o',tmp/'validated.lowir',tmp/'stress.lowir')
    assert (tmp/'stress.lowir').stat().st_size<100*source.stat().st_size
print(f'PA32 calls: PASS ({len(checks)} executable boundary cases x four levels x two backends; source EH/debug replay, constants/escape and growth guards)')
