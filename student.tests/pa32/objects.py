#!/usr/bin/env python3
"""Explicit aggregate-storage reducers; expected bytes/results are independent."""
import pathlib, random, subprocess, tempfile
ROOT = pathlib.Path(__file__).resolve().parents[2]

def run(*args):
    r = subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert r.returncode == 0,(args,r.returncode,r.stderr[-4000:])
    return r

functions, checks = [], []
def case(body, expected, params='', args='', slots=''):
    name = 'object_case'+str(len(checks))
    functions.append(f'function @{name}({params}) -> i64 [no_inline=yes] {{\n{slots}\n{body}\n}}')
    checks.append((name,args,expected))

rng = random.Random(211)
for ty,width,signed in [('i8',8,True),('u8',8,False),('i16',16,True),('u16',16,False),('i32',32,True),('u32',32,False),('i64',64,True)]:
    for branch in [0,1]:
        for depth in [1,5]:
            x = rng.randrange(-1000000,1000000)
            narrowed = x % (1<<width)
            if signed and narrowed >= 1<<(width-1): narrowed -= 1<<width
            slots = '\n'.join(f'slot $s{n} : obj<16x8>' for n in range(depth+1))
            body = f'''block ^entry:
 %a0 = addr $s0
 store {ty} %x, %a0
 %x = copy i64 99
 branch %c, ^yes, ^no
block ^yes:
 %high = index i8 [projection=field] %a0, 8
 store i64 7, %high
 jump ^join
block ^no:
 %other = index i8 [projection=field] %a0, 8
 store i64 11, %other
 jump ^join
block ^join:\n'''
            body += '\n'.join(f'%a{n+1} = addr $s{n+1}\ncopyobj 16x8 %a{n}, %a{n+1}' for n in range(depth))
            body += f'''\n%low = load {ty} %a{depth}
 %w = copy i64 %low
 %p = index i8 [projection=field] %a{depth}, 8
 %hi = load i64 %p
 %r = binary add i64 %w, %hi
 return i64 %r'''
            case(body,narrowed+(7 if branch else 11),'%x : i64, %c : i64',f'{x}, {branch}',slots)
# Exact-copy identity and padding bytes, initialized by an external byte source.
functions += ['global @raw = {i64 72623859790382856 i64 1230066625199609624 i64 2387509390608836392}']
for same in [False,True]:
    case(f'''block ^entry:
 %src = addr @raw
 %a = addr $a
 %b = addr $b
 copyobj 24x8 %src, %a
 %f = index i8 [projection=field] %a, 8
 store u32 123, %f
 copyobj 24x8 %a, {'%a' if same else '%b'}
 %out = index i8 [projection=field] {'%a' if same else '%b'}, 16
 %v = load i64 %out
 return i64 %v''',2387509390608836392,slots='slot $a : obj<24x8>\nslot $b : obj<24x8>')
# Unknown offsets, overlapping widths, mutable pointer definitions, and escape
# prevent splitting. They must remain observable through the original bytes.
for extra,expected in [
    ('%p = index i8 %a, %offset\nstore i8 3, %p',259),
    ('store u8 3, %a',259),
    ('%p = copy ptr %a\n%p = copy ptr %b\nstore i64 3, %p',256),
    ('store volatile u8 3, %a',259),
    ('call void @mutate(%a)',19)]:
    case(f'''block ^entry:
 %a = addr $a
 %b = addr $b
 store i64 256, %a
 store i64 0, %b
 {extra}
 %r = load i64 %a
 return i64 %r''',expected,'%offset : i64','0','slot $a : obj<8x8>\nslot $b : obj<8x8>')
functions += ['function @mutate(%p : ptr) -> void [no_inline=yes] {block ^entry: store i64 19, %p return void}']
for ty in ['f32','f64']:
    case(f'''block ^entry:
 %a = addr $a
 store {ty} -0.0, %a
 %v = load {ty} %a
 %r = binary div {ty} 1.0, %v
 %negative = cmp lt {ty} %r, 0.0
 return i64 %negative''',1,slots='slot $a : obj<8x8>')
    # Bulk floating representation is deliberately retained (NaN/sign/padding).
    case(f'''block ^entry:
 %a = addr $a
 %b = addr $b
 store {ty} -0.0, %a
 copyobj 8x8 %a, %b
 %v = load {ty} %b
 %r = binary div {ty} 1.0, %v
 %negative = cmp lt {ty} %r, 0.0
 return i64 %negative''',1,slots='slot $a : obj<8x8>\nslot $b : obj<8x8>')
for cond in [0,1]:
    case('''block ^entry:
 %a = addr $a
 zeroinit 16x8 %a
 branch %cond, ^yes, ^join
block ^yes:
 %p = index i8 [projection=field] %a, 8
 store i64 17, %p
 jump ^join
block ^join:
 %p2 = index i8 [projection=field] %a, 8
 %r = load i64 %p2
 return i64 %r''',17 if cond else 0,'%cond : i64',str(cond),'slot $a : obj<16x8>')
# Partial copies retain the source span; the destination may be decomposed.
case('''block ^entry:
 %a = addr $a
 %b = addr $b
 %part = index i8 [projection=field] %a, 8
 store i64 73, %part
 copyobj 8x8 %part, %b
 %r = load i64 %b
 return i64 %r''',73,slots='slot $a : obj<16x8>\nslot $b : obj<8x8>')
# A raw copy must preserve the complete initialized span, including holes in
# the local field census. Read the result through a distinct external consumer.
functions += ['''global @out = {zero 24}
function @write_bytes(%out : ptr) -> void [no_inline=yes] {
 slot $local : obj<24x8>
 block ^entry:
 %a = addr $local
 %raw = addr @raw
 copyobj 24x8 %raw, %a
 %field = index i8 [projection=field] %a, 8
 store u32 123, %field
 copyobj 24x8 %a, %out
 return void
}''']
case('''block ^entry:
 %out = addr @out
 call void @write_bytes(%out)
 %p = index i8 [projection=field] %out, 12
 %v = load u32 %p
 %r = copy i64 %v
 return i64 %r''',0x11121314)
main = ['function @main() -> i64 [role=entry] { block ^entry:', '%total0 = const i64 0']
for n,(name,args,expected) in enumerate(checks):
    main += [f'%v{n} = call i64 @{name}({args})',f'%bad{n} = cmp ne i64 %v{n}, {expected}',f'%total{n+1} = binary or i64 %total{n}, %bad{n}']
main += [f'return i64 %total{len(checks)}','}']
with tempfile.TemporaryDirectory(prefix='pa32-objects-') as directory:
    tmp = pathlib.Path(directory); source = tmp/'input.lowir'
    source.write_text('\n'.join(functions+main))
    for level in range(4):
        opt = tmp/f'o{level}.lowir'
        run(ROOT/'dev/lowiropt',f'-O{level}','-o',opt,source)
        run(ROOT/'dev/lowir','-o',tmp/'valid.lowir',opt)
        run(ROOT/'dev/lowir2native','-o',tmp/'native',opt); run(tmp/'native')
        run(ROOT/'dev/cppgm++','-O0','-o',tmp/'replay',opt); run(tmp/'replay')
        run(ROOT/'dev/cppgm++',f'-O{level}','-o',tmp/'direct',source); run(tmp/'direct')
        if level:
            text = opt.read_text()
            assert 'call void @mutate' in text and 'store volatile' in text
            assert 'copyobj 8x8' in text # conservative floating bulk copy
    # Complete-copy components propagate layouts once even for long chains.
    # Growth refusal is transactional for a wide, repeatedly copied object.
    for length in [100, 500, 1000]:
        chain = ['function @main() -> i64 [role=entry] {']
        chain += [f'slot $s{n} : obj<8x8>' for n in range(length+1)]
        chain += ['block ^entry:', '%a0 = addr $s0', 'store i64 61, %a0']
        for n in range(length):
            chain += [f'%a{n+1} = addr $s{n+1}', f'copyobj 8x8 %a{n}, %a{n+1}']
        chain += [f'%v = load i64 %a{length}', '%bad = cmp ne i64 %v, 61', 'return i64 %bad', '}']
        source.write_text('\n'.join(chain))
        run(ROOT/'dev/lowiropt','-O1','-o',tmp/'chain.lowir',source)
        assert 'copyobj' not in (tmp/'chain.lowir').read_text()
        run(ROOT/'dev/cppgm++','-O0','-o',tmp/'chain',tmp/'chain.lowir'); run(tmp/'chain')
    guard = ['function @main() -> i64 [role=entry] { slot $a : obj<64x8> block ^entry: %a = addr $a']
    for n in range(16):
        guard += [f'%p{n} = index i8 [projection=field] %a, {4*n}', f'store i32 {n}, %p{n}']
    guard += ['copyobj 64x8 %a, %a']*100
    guard += ['%v = load i32 %p15','%bad = cmp ne i32 %v, 15','return i64 %bad','}']
    source.write_text('\n'.join(guard))
    run(ROOT/'dev/lowiropt','-O1','-o',tmp/'guard.lowir',source)
    assert 'copyobj 64x8' in (tmp/'guard.lowir').read_text()
    run(ROOT/'dev/cppgm++','-O0','-o',tmp/'guard',tmp/'guard.lowir'); run(tmp/'guard')
    # Source exception/lifetime paths and serialized debug/object boundaries.
    cpp = tmp/'lifetime.cpp'
    cpp.write_text('''struct Pair { long a,b; };
long changed(int n) { Pair p={3,5}; try {p.a=11; if(n)throw n;p.b=17;} catch(int) {return p.a+p.b;}return p.a+p.b; }
Pair pick(int c) { Pair a={7,11},b={13,17}; if(c)return a;return b; }
int main(){Pair a=pick(0),b=pick(1);return changed(1)!=16||changed(0)!=28||a.b!=17||b.a!=7;}
''')
    for level in range(4):
        run(ROOT/'dev/cppgm++','-gline-tables-only',f'-O{level}','-o',tmp/'source',cpp); run(tmp/'source')
        run(ROOT/'dev/cppgm++','-gline-tables-only','--emit-lowir','-O0','-o',tmp/'source.lowir',cpp)
        run(ROOT/'dev/cppgm++','-gline-tables-only',f'-O{level}','-o',tmp/'source-replay',tmp/'source.lowir'); run(tmp/'source-replay')
print(f'PA32 objects: PASS ({len(checks)} execution cases x 4 levels x 3 paths; source EH/debug replay)')
