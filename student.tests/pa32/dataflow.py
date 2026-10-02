#!/usr/bin/env python3
"""Explicit execution/legality reducers for sparse slot and CFG optimization."""
import pathlib, random, subprocess, tempfile
ROOT = pathlib.Path(__file__).resolve().parents[2]
def run(*args):
    result = subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
    assert result.returncode == 0,(args,result.returncode,result.stderr[-4000:])
    return result
functions, calls = [], []
def case(body, expected, args='', parameters='', slots=''):
    name = 'test'+str(len(functions))
    functions.append(f'function @{name}({parameters}) -> i64 {{\n{slots}\n{body}\n}}\n')
    calls.append((name,args,expected))
rng = random.Random(208)
for ty,width,signed in [('i8',8,True),('u8',8,False),('i16',16,True),('u16',16,False),('i32',32,True),('u32',32,False),('i64',64,True)]:
    for _ in range(12):
        left,right = rng.randrange(-100000,100000),rng.randrange(-100000,100000)
        cond = rng.randrange(2)
        value = (left if cond else right) % (1<<width)
        if signed and value >= 1<<(width-1): value -= 1<<width
        case(f'''block ^entry:
 branch %condition, ^left, ^right
block ^left:
 store {ty} %left, $x
 %left = copy i64 55
 jump ^join
block ^right:
 store {ty} %right, $x
 %right = copy i64 66
 jump ^join
block ^join:
 %value = load {ty} $x
 %result = copy i64 %value
 return i64 %result''',value,f'{cond}, {left}, {right}',
        '%condition : i64, %left : i64, %right : i64',f'slot $x : {ty}')
for n in [0,1,2,3,7,17]:
    case('''block ^entry:
 store i64 1, $a
 store i64 2, $b
 store i64 0, $i
 jump ^loop
block ^loop:
 %i = load i64 $i
 %more = cmp lt i64 %i, %n
 branch %more, ^body, ^exit
block ^body:
 %a = load i64 $a
 %b = load i64 $b
 store i64 %a, $b
 store i64 %b, $a
 %next = binary add i64 %i, 1
 store i64 %next, $i
 jump ^loop
block ^exit:
 %x = load i64 $a
 %y = load i64 $b
 %ten = binary mul i64 %x, 10
 %result = binary add i64 %ten, %y
 return i64 %result''',21 if n%2 else 12,str(n),'%n : i64','slot $a : i64\nslot $b : i64\nslot $i : i64')
# Generated-name collision and a store that captures a mutable source.
case('''block ^entry:
 %opt_slot_1 = copy i64 7
 store i64 %opt_slot_1, $x
 %opt_slot_1 = copy i64 9
 jump ^next
block ^next:
 %v = load i64 $x
 return i64 %v''',7,slots='slot $x : i64')
# Byte writes to a wider slot must not be mistaken for whole-slot definitions.
case('''block ^entry:
 store i64 256, $x
 store u8 3, $x
 jump ^next
block ^next:
 %v = load i64 $x
 return i64 %v''',259,slots='slot $x : i64')
# Escaped and volatile storage stays addressable.
case('''block ^entry:
 store volatile i64 10, $x
 %address = addr $x
 store i64 19, %address
 jump ^next
block ^next:
 %v = load volatile i64 $x
 return i64 %v''',19,slots='slot $x : i64')
for cond in [0,1,2,-1]:
    case('''block ^entry:
 branch %c, ^left, ^right
block ^left:
 jump ^join
block ^right:
 jump ^join
block ^join:
 %v = phi i64 [^left: 11, ^right: 22]
 return i64 %v''',11 if cond else 22,str(cond),'%c : i64')
    case('''block ^entry:
 branch %c, ^left, ^right
block ^left:
 jump ^hop
block ^hop:
 jump ^join
block ^right:
 jump ^join
block ^join:
 %v = phi i64 [^hop: 37, ^right: 37]
 return i64 %v''',37,str(cond),'%c : i64')
main = ['function @main() -> i64 [role=entry] { block ^entry:', ' %total0 = const i64 0']
for n,(name,args,expected) in enumerate(calls):
    main += [f' %v{n} = call i64 @{name}({args})',f' %bad{n} = cmp ne i64 %v{n}, {expected}',f' %total{n+1} = binary or i64 %total{n}, %bad{n}']
main += [f' return i64 %total{len(calls)}','}']
with tempfile.TemporaryDirectory(prefix='pa32-dataflow-') as directory:
    tmp = pathlib.Path(directory); source = tmp/'input.lowir'
    source.write_text(''.join(functions)+'\n'.join(main))
    for level in range(4):
        optimized = tmp/f'o{level}.lowir'
        run(ROOT/'dev/lowiropt',f'-O{level}','-o',optimized,source)
        run(ROOT/'dev/lowir','-o',tmp/'validated.lowir',optimized)
        run(ROOT/'dev/lowir2native','-o',tmp/'native',optimized)
        run(tmp/'native')
        run(ROOT/'dev/cppgm++','-O0','-o',tmp/'object-exe',optimized)
        run(tmp/'object-exe')
    # Actual exception transfers: one immutable captured slot, a slot updated
    # after registration, and an ordinary merge after a handler definition.
    eh = tmp/'exception.cpp'
    eh.write_text('''int throws(int n) { if(n) throw n; return 5; }
int capture(int n) { int value=31; try { throws(n); } catch(int) {return value;} return 31; }
int changed(int n) { int value=1; try { value=17; throws(n); value=19; } catch(int) {return value;} return value; }
int merged(int n) { int value=0; try {throws(n);value=1;} catch(int) {value=2;} return value; }
int main() { return capture(1)!=31 || capture(0)!=31 || changed(1)!=17 || changed(0)!=19 || merged(1)!=2 || merged(0)!=1; }
''')
    for level in range(4):
        run(ROOT/'dev/cppgm++',f'-O{level}','-o',tmp/'eh',eh); run(tmp/'eh')
        run(ROOT/'dev/cppgm++','--emit-lowir','-O0','-o',tmp/'eh.lowir',eh)
        run(ROOT/'dev/cppgm++',f'-O{level}','-o',tmp/'eh-replay',tmp/'eh.lowir'); run(tmp/'eh-replay')
    # Uninitialized paths, exceptional roots with varying stores, extended
    # scalar types, and empty cycles must retain a valid conservative result.
    guards = '''declare function @maythrow() -> void
function @uninitialized(%c : i64) -> i64 {
 slot $x : i64
 block ^entry: branch %c, ^left, ^right
 block ^left: store i64 1, $x jump ^right
 block ^right: %r = load i64 $x return i64 %r
}
function @varying_handler() -> i64 {
 slot $x : i64
 block ^entry: store i64 1, $x eh_try ^handler call void @maythrow() store i64 2, $x call void @maythrow() eh_end return i64 0
 block ^handler: %r = load i64 $x return i64 %r
}
function @empty_cycle() -> void {
 block ^entry: jump ^a
 block ^a: jump ^b
 block ^b: jump ^a
}
'''
    source.write_text(guards)
    run(ROOT/'dev/lowiropt','-O1','-o',tmp/'guards.lowir',source)
    run(ROOT/'dev/lowir','-o',tmp/'validated.lowir',tmp/'guards.lowir')
    assert (tmp/'guards.lowir').read_text().count('load i64 $x') == 2
print(f'PA32 dataflow: PASS ({len(calls)} execution cases x 4 levels x 2 backends; EH direct/replay; conservative guards)')
