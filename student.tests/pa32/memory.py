#!/usr/bin/env python3
"""Checked alias/byte/CFG reducers for the function-owned memory pass."""
import pathlib, random, subprocess, tempfile
ROOT=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=90)
 assert r.returncode==0,(args,r.returncode,r.stderr[-5000:])
 return r
functions=[];tests=[]
def case(body,expected,args='%p, %q, 1',params='%p : ptr, %q : ptr, %c : i64'):
 name='memory'+str(len(functions))
 functions.append(f'function @{name}({params}) -> i64 [binding=strong, no_inline=yes] {{\n{body}\n}}\n')
 tests.append((name,args,expected))
initial=bytes(range(1,65))
def value(offset,width,data=initial):return int.from_bytes(data[offset:offset+width],'little')
def signed(x):return x if x<2**63 else x-2**64
# Partial overlaps, complete stores, disjoint ranges, modulo offset addressing.
for width,ty in [(1,'u8'),(2,'u16'),(4,'u32'),(8,'i64')]:
 for offset in [0,1,4,7,8,15,16,24]:
  data=bytearray(initial);data[offset:offset+width]=(7).to_bytes(width,'little')
  case(f'''block ^entry:
 %a = index i8 %p, 8
 %b = index i8 %p, {offset}
 %before = load i64 %a
 store {ty} 7, %b
 %after = load i64 %a
 %sum = binary add i64 %before, %after
 return i64 %sum''',signed(value(8,8)+value(8,8,data)))
# Aliasing unknown parameters vs disjoint noalias projections.
for noalias in [False,True]:
 for alias in [False,True] if not noalias else [False]:
  for c in [0,1,7]:
   data=bytearray(initial)
   if c and alias:data[8:16]=(29).to_bytes(8,'little')
   annotation=' [alias=noalias]' if noalias else ''
   case('''block ^entry:
 %a = index i8 [projection=field] %p, 8
 %b = index i8 [projection=field] %q, 8
 %before = load i64 %a
 branch %c, ^yes, ^no
block ^yes: store i64 29, %b jump ^end
block ^no: jump ^end
block ^end:
 %after = load i64 %a
 %sum = binary add i64 %before, %after
 return i64 %sum''',signed(value(8,8)+value(8,8,data)),f'%p, {"%p" if alias else "%q"}, {c}',f'%p : ptr{annotation}, %q : ptr{annotation}, %c : i64')
# Conditional-address values; a write on an incoming edge kills the old fact.
for mutate in [False,True]:
 for c in [0,1,9]:
  for multi in [False,True]:
   expected=29 if mutate and c else value(8 if c else 0,8)
   case(f'''block ^entry:
 %other = index i8 %p, 8
 %a = load i64 %p
 %b = load i64 %other
 branch %c, ^yes, ^no
block ^yes:
 {'store i64 29, %other' if mutate else 'nop'}
 jump ^end
block ^no: jump ^end
block ^end:
 %selected = phi ptr [^yes: %other, ^no: %p]
 %v = load i64 %selected
 {'store i64 17, %selected' if multi else 'nop'}
 return i64 %v''',expected,f'%p, %q, {c}')
# Mutable values and addresses must retain snapshots.
case('''block ^entry:
 %a = load i64 %p
 %p = copy ptr %q
 %b = load i64 %p
 %sum = binary add i64 %a, %b
 return i64 %sum''',signed(2*value(0,8)))
case('''block ^entry:
 store i64 %c, %p
 %c = copy i64 99
 %a = load i64 %p
 return i64 %a''',7,'%p, %q, 7')
case('''block ^entry:
 %a = load i64 %p
 %a = copy i64 99
 %b = load i64 %p
 return i64 %b''',value(0,8))
# Repeated diamonds: both branch choices, a changed field, repeated loop
# execution, and a load with an independent live use.
for mutate in [False,True]:
 for c in [0,1,7]:
  for extra in [False,True]:
   expected=(value(8,8) if c else 13)+(31 if c and mutate else value(8,8) if c else 13)
   if extra:expected+=31 if mutate else value(8,8)
   case(f'''block ^entry: branch %c, ^left, ^right
block ^left:
 %pa = index i8 [projection=field] %p, 8
 %a = load i64 %pa
 jump ^join
block ^right:
 %b = binary add i64 %c, 13
 jump ^join
block ^join:
 %first = phi i64 [^left: %a, ^right: %b]
 {'%pm = index i8 [projection=field] %p, 8' if mutate else 'nop'}
 {'store i64 31, %pm' if mutate else 'nop'}
 branch %c, ^left2, ^right2
block ^left2:
 %pa2 = index i8 [projection=field] %p, 8
 %a2 = load i64 %pa2
 jump ^join2
block ^right2:
 %b2 = binary add i64 %c, 13
 jump ^join2
block ^join2:
 %second = phi i64 [^left2: %a2, ^right2: %b2]
 %sum = binary add i64 %first, %second
 {'%px = index i8 %p, 8 %x = load i64 %px %answer = binary add i64 %sum, %x' if extra else '%answer = copy i64 %sum'}
 return i64 %answer''',signed(expected),f'%p, %q, {c}')
# A matched load's parent has an independent live use. Its single load use
# does not suffice to retire that load when replacing just one result phi.
for c in [0,1]:
 case('''block ^entry: branch %c, ^a, ^b
block ^a:
 %x = load i64 %p
 %x3 = binary add i64 %x, 3
 jump ^m
block ^b: jump ^m
block ^m:
 %one = phi i64 [^a: %x3, ^b: 13]
 branch %c, ^a2, ^b2
block ^a2:
 %y = load i64 %p
 %y3 = binary add i64 %y, 3
 jump ^m2
block ^b2: jump ^m2
block ^m2:
 %two = phi i64 [^a2: %y3, ^b2: 13]
 %extra = phi i64 [^a2: %y3, ^b2: 17]
 %sum = binary add i64 %one, %two
 %answer = binary add i64 %sum, %extra
 return i64 %answer''',signed(3*(value(0,8)+3)) if c else 43,f'%p, %q, {c}')
# Loop backedges: first iteration cannot supply memory facts for the next.
for count in [0,1,2,7]:
 case('''block ^entry: store i64 0, %p jump ^head
block ^head:
 %i = phi i64 [^entry: 0, ^body: %next]
 %v = load i64 %p
 %more = cmp lt i64 %i, %c
 branch %more, ^body, ^exit
block ^body:
 %next = binary add i64 %i, 1
 store i64 %next, %p
 jump ^head
block ^exit: return i64 %v''',count,f'%p, %q, {count}')
# Adjacent transfers, legal and illegal coalescing. Same-object overlap must
# keep sequential copy semantics (including the cascading load).
for noalias in [False,True]:
 for alias in [False,True] if not noalias else [False]:
  for projection in ['',' [projection=field]']:
   for volatile in ['', ' volatile']:
    data=bytearray(initial)
    source=bytearray(initial)
    for at in [0,8,16]:data[at+8:at+16]=(data if alias else source)[at:at+8]
    expected=value(24,8,data)
    annotation=' [alias=noalias]' if noalias else ''
    body=['block ^entry:']
    for n,at in enumerate([0,8,16]):
     body += [f'%s{n} = index i8{projection} %p, {at}',f'%d{n} = index i8{projection} %q, {at+8}',f'%v{n} = load{volatile} i64 %s{n}',f'store{volatile} i64 %v{n}, %d{n}']
    body += ['%answer = load i64 %d2','return i64 %answer']
    case('\n'.join(body),expected,f'%p, {"%p" if alias else "%q"}, 1',f'%p : ptr{annotation}, %q : ptr{annotation}, %c : i64')
# Narrow store conversions, floating snapshots, signed zero and atomics.
case('''block ^entry:
 store u8 %c, %p
 %v = load u8 %p
 return i64 %v''',255,'%p, %q, -1')
case('''block ^entry:
 store f32 -0.0, %p
 %x = load f32 %p
 %y = load f32 %p
 %a = binary div f32 1.0, %y
 %negative = cmp lt f32 %a, 0.0
 return i64 %negative''',1)
case('''block ^entry:
 store i64 3, %p
 %a = load i64 %p
 atomic_store i64 19, %p, 5
 %b = load i64 %p
 return i64 %b''',19)
case('''block ^entry:
 store i64 3, %p
 %a = load i64 %p
 store volatile i64 19, %p
 %b = load i64 %p
 return i64 %b''',19)
main=['function @main() -> i64 [role=entry] {','slot $p : obj<64x8>','slot $q : obj<64x8>','block ^entry:','%p = addr $p','%q = addr $q','%total0 = const i64 0']
for n,(name,args,expected) in enumerate(tests):
 for label in ['p','q']:
  for offset in range(0,64,8):main += [f'%{label}{n}_{offset} = index i8 %{label}, {offset}',f'store i64 {value(offset,8)}, %{label}{n}_{offset}']
 main += [f'%r{n} = call i64 @{name}({args})',f'%bad{n} = cmp ne i64 %r{n}, {expected}',f'%total{n+1} = binary or i64 %total{n}, %bad{n}']
main += [f'return i64 %total{len(tests)}','}']
with tempfile.TemporaryDirectory(prefix='pa32-memory-') as directory:
 tmp=pathlib.Path(directory);source=tmp/'input.lowir';source.write_text(''.join(functions)+'\n'.join(main))
 for level in range(4):
  optimized=tmp/f'o{level}.lowir'
  run(ROOT/'dev/lowiropt',f'-O{level}','-o',optimized,source)
  run(ROOT/'dev/lowir','-o',tmp/'validated.lowir',optimized)
  run(ROOT/'dev/lowir2native','-o',tmp/'native',optimized);run(tmp/'native')
  run(ROOT/'dev/cppgm++','-O0','-o',tmp/'object',optimized);run(tmp/'object')
  run(ROOT/'dev/cppgm++',f'-O{level}','-o',tmp/'direct',source);run(tmp/'direct')
print(f'PA32 memory: PASS ({len(tests)} checked cases x 4 levels x 3 paths)')
