#!/usr/bin/env python3
"""Explicit PA32 legality/transport tests; no course fixtures are changed."""
import pathlib, random, subprocess, tempfile
ROOT = pathlib.Path(__file__).resolve().parents[2]
OPT = ROOT / 'dev/lowiropt'
NATIVE = ROOT / 'dev/lowir2native'
DRIVER = ROOT / 'dev/cppgm++'
def run(*args, ok=True):
    r = subprocess.run([str(x) for x in args], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if ok and r.returncode:
        raise AssertionError((args, r.returncode, r.stderr.decode()[-2000:]))
    return r
rng = random.Random(207)
functions = []
def case(instructions, ty, value):
    name = 'case' + str(len(functions))
    functions.append(f'function @{name}() -> i64 {{\n block ^entry:\n{instructions}\n %bad = cmp ne {ty} %result, {value}\n return i64 %bad\n}}\n')
for ty, width in [('i8',8),('u8',8),('i16',16),('u16',16),('i32',32),('u32',32),('i64',64),('i128',128)]:
    mask = (1 << width)-1
    def signed(n): return n-(1 << width) if n & (1 << (width-1)) else n
    for op in ['add','sub','mul','and','or','xor','udiv','umod','div','mod','shl','shr','ushr']:
        for k in range(4):
            a, b = rng.getrandbits(width), rng.getrandbits(width) or 1
            if op in ('shl','shr','ushr'): b %= width
            x, y = signed(a), signed(b)
            if op == 'add': value = a+b
            elif op == 'sub': value = a-b
            elif op == 'mul': value = a*b
            elif op == 'and': value = a&b
            elif op == 'or': value = a|b
            elif op == 'xor': value = a^b
            elif op == 'udiv': value = a//b
            elif op == 'umod': value = a%b
            elif op in ('div','mod'):
                q = abs(x)//abs(y) * (-1 if (x<0) != (y<0) else 1)
                value = q if op == 'div' else x-q*y
            elif op == 'shl': value = a << b
            elif op == 'shr': value = x >> b
            else: value = a >> b
            case(f' %a = const {ty} {a}\n %b = copy {ty} {b}\n %result = binary {op} {ty} %a, %b',ty,value&mask)
    for op in ['eq','ne','lt','le','gt','ge','ult','ule','ugt','uge']:
        a,b = rng.getrandbits(width), rng.getrandbits(width)
        x,y = (a,b) if op.startswith('u') or op in ('eq','ne') else (signed(a),signed(b))
        pred = op.removeprefix('u')
        value = {'eq':x==y,'ne':x!=y,'lt':x<y,'le':x<=y,'gt':x>y,'ge':x>=y}[pred]
        case(f' %result = cmp {op} {ty} {a}, {b}','i64',int(value))
case(' %a = copy i8 255\n %result = copy i64 %a','i64',-1)
case(' %a = convert zext i128 i64 -1\n %b = binary add i128 %a, 2\n %result = binary ushr i128 %b, 64','i128',1)
case(' %x = const i64 3\n %y = copy i64 %x\n %x = const i64 4\n %result = binary add i64 %x, %y','i64',7)
case(' %x = const i128 1\n %saved = copy i128 %x\n %x = const i128 18446744073709551616\n %result = binary sub i128 %x, %saved','i128',18446744073709551615)
case(' %result = call u8 @reassociate(200)','u8',64)
case(' %x = const i64 1\n %a = binary add i64 %x, 10\n %x = const i64 2\n %result = binary add i64 %a, 20','i64',31)
functions.append('''function @reassociate(%x : u8) -> u8 {
 block ^entry:
  %a = binary add u8 %x, 100
  %b = binary add u8 %a, 20
  return u8 %b
}
function @phi_test() -> i64 {
 block ^entry:
  jump ^loop
 block ^loop:
  %a = phi i64 [^entry: 1, ^loop: %b]
  %b = phi i64 [^entry: 2, ^loop: %a]
  %i = phi i64 [^entry: 0, ^loop: %next]
  %next = binary add i64 %i, 1
  %again = cmp lt i64 %next, 4
  branch %again, ^loop, ^exit
 block ^exit:
  %ten = binary mul i64 %a, 10
  %result = binary add i64 %ten, %b
  %bad = cmp ne i64 %result, 21
  return i64 %bad
}
function @mutable_arg(%x : i64) -> i64 {
 block ^entry:
  %x = binary add i64 %x, 1
  %saved = copy i64 %x
  %x = const i64 12
  %result = binary sub i64 %x, %saved
  return i64 %result
}
function @mutable_argument_test() -> i64 {
 block ^entry:
  %result = call i64 @mutable_arg(5)
  %bad = cmp ne i64 %result, 6
  return i64 %bad
}
function @mutable_phi() -> i64 {
 block ^entry:
  jump ^loop
 block ^loop:
  %x = phi i64 [^entry: 1, ^body: %x]
  %i = phi i64 [^entry: 0, ^body: %next]
  %more = cmp lt i64 %i, 3
  branch %more, ^body, ^exit
 block ^body:
  %x = binary add i64 %x, 1
  %next = binary add i64 %i, 1
  jump ^loop
 block ^exit:
  %bad = cmp ne i64 %x, 4
  return i64 %bad
}
function @prune_test() -> i64 {
 block ^entry:
  branch 1, ^yes, ^no
 block ^yes:
  jump ^join
 block ^no:
  jump ^join
 block ^join:
  %a = phi i64 [^yes: 4, ^no: 9]
  %bad = cmp ne i64 %a, 4
  return i64 %bad
}
''')
main = ['function @main() -> i64 [role=entry] {',' block ^entry:', ' %sum0 = const i64 0']
names = ['case'+str(i) for i in range(len(functions)-1)]+['phi_test','mutable_argument_test','mutable_phi','prune_test']
for i,name in enumerate(names):
    main += [f' %r{i} = call i64 @{name}()', f' %sum{i+1} = binary or i64 %sum{i}, %r{i}']
main += [f' return i64 %sum{len(names)}','}']
with tempfile.TemporaryDirectory(prefix='pa32-local-') as tmp:
    tmp = pathlib.Path(tmp); source = tmp/'test.lowir'
    source.write_text(''.join(functions)+'\n'.join(main)+'\n')
    for level in range(4):
        opt = tmp/f'o{level}.lowir'; exe = tmp/f'o{level}'
        run(OPT,f'-O{level}','-o',opt,source)
        run(ROOT/'dev/lowir','-o',tmp/'validated.lowir',opt)
        run(NATIVE,'-o',exe,opt)
        run(exe)
    trap = tmp/'trap.lowir'
    trap.write_text('function @trap() -> i64 { block ^entry: %unused = binary div i64 1, 0 return i64 0 }')
    run(OPT,'-O1','-o',tmp/'trap-out.lowir',trap)
    assert 'binary div' in (tmp/'trap-out.lowir').read_text()
    for args in [[],['-O1'],['-o',str(tmp/'bad'),str(source)],['-O1','-O2','-o',str(tmp/'bad'),str(source)],['-O1','-o',str(tmp/'bad'),str(tmp/'absent')]]:
        assert run(OPT,*args,ok=False).returncode
    run(OPT,'--help')
    a,b=tmp/'a.cpp',tmp/'b.cpp'
    a.write_text('int f(int x){return x+1;}')
    b.write_text('int f(int);int main(){return f(2)==3?0:1;}')
    for flag in ['-g0','-gline-tables-only']:
        merged=tmp/'merged.lowir'
        run(DRIVER,'--emit-lowir',flag,'-O1','--validate-lowir','-o',merged,a,b)
        run(ROOT/'dev/lowir','-o',tmp/'merged-checked.lowir',merged)
        if flag != '-g0': assert '!dbg(' in merged.read_text()
        run(DRIVER,'-O0','-o',tmp/'merged',merged)
        run(tmp/'merged')
print(f'PA32 local legality: PASS ({len(names)} execution cases at O0/O1/O2/O3; trap and CLI checks)')
