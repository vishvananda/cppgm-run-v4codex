#!/usr/bin/env python3
"""Exercise typed phase boundaries, aliases, ABI effects and executable layout."""
import pathlib, subprocess, struct, tempfile
root=pathlib.Path(__file__).resolve().parents[2]
cc=root/'dev/lowir2native'
source='''
global @cell : i64 = 0
function @rotate(%a : i64, %b : i64, %c : i64, %d : i64, %e : i64, %f : i64, %g : i64) -> i64 {
 block ^entry:
  %x = binary sub i64 %a, %g
  %y = binary add i64 %x, %d
  return i64 %y
}
function @forward(%a : i64, %b : i64, %c : i64, %d : i64, %e : i64, %f : i64) -> i64 {
 block ^entry:
  %answer = call i64 @rotate(%b, %a, %d, %c, %f, %e, 5)
  return i64 %answer
}
function @main() -> i64 [role=entry] {
 slot $byte : i8
 slot $expected : i64
 block ^entry:
  store i8 -128, $byte
  %n = load i8 $byte
  %wide = copy i64 %n
  %bad0 = cmp ne i64 %wide, -128
  %small = copy i8 256
  %bad1 = cmp ne i64 %small, 0
  %swapped = unary bswap u16 4660
  %bad2 = cmp ne u16 %swapped, 13330
  %got = call i64 @forward(1, 9, 3, 4, 5, 6)
  %bad3 = cmp ne i64 %got, 7
  store i64 0, $expected
  %p = addr @cell
  %expect = addr $expected
  %changed = atomic_compare_exchange i64 %p, %expect, 42, 5, 2
  %unchanged = atomic_compare_exchange i64 %p, %expect, 9, 5, 2
  %seen = load i64 $expected
  %bad4 = cmp ne i64 %changed, 1
  %bad5 = cmp ne i64 %unchanged, 0
  %bad6 = cmp ne i64 %seen, 42
  %ab = binary or i64 %bad0, %bad1
  %cd = binary or i64 %bad2, %bad3
  %ef = binary or i64 %bad4, %bad5
  %abcd = binary or i64 %ab, %cd
  %efg = binary or i64 %ef, %bad6
  %bad = binary or i64 %abcd, %efg
  return i64 %bad
}
'''
with tempfile.TemporaryDirectory(prefix='pa24-integration-') as path:
 d=pathlib.Path(path); src=d/'abi.lowir'; src.write_text(source)
 exe=d/'abi'; mir=d/'abi.mir'
 p=subprocess.run([str(cc),'--stats','--dump-machine-ir',str(mir),'-o',str(exe),str(src)],text=True,capture_output=True,check=True)
 assert 'scratch_carried_reloads=' in p.stderr
 subprocess.run([str(exe)],check=True)
 exe2=d/'abi2'; subprocess.run([str(cc),'-o',str(exe2),str(src)],check=True)
 assert exe.read_bytes()==exe2.read_bytes(), 'MIR view changed executable output'
 image=exe.read_bytes(); assert image[:6]==b'\x7fELF\x02\x01'
 ph=struct.unpack_from('<Q',image,32)[0]; count=struct.unpack_from('<H',image,56)[0]
 flags=[struct.unpack_from('<I',image,ph+56*k+4)[0] for k in range(count)]
 assert flags==[5,6],flags
 a=d/'a.lowir'; a.write_text('function @read() -> i64 { block ^entry: %x = load i64 @g return i64 %x }\n')
 b=d/'b.lowir'; b.write_text('global @g : i64 = 3\nfunction @main() -> i64 { block ^entry: %x = call i64 @read() %bad = cmp ne i64 %x, 3 return i64 %bad }\n')
 subprocess.run([str(cc),'-o',str(exe),str(a),str(b)],check=True); subprocess.run([str(exe)],check=True)
 helper=d/'helper.lowir'; helper.write_text('function @helper() -> i64 !dbg("test.cpp", 7, 3) { block ^entry: return i64 0 !dbg("test.cpp", 8, 5) }\n')
 subprocess.run([str(cc),'--dump-machine-ir',str(mir),str(helper)],check=True)
 text=mir.read_text(); assert 'startup' not in text and '!dbg("test.cpp", 8, 5)' in text
 floating=d/'floating-data.lowir'
 floating.write_text('global @data = { f32 1 f64 -2 f32 snan }\nfunction @main() -> i64 { block ^entry: '
  '%p = addr @data %d = index i8 %p, 8 %s = index i8 %p, 16 '
  '%one = load i32 %p %minus_two = load i64 %d %nan = load u32 %s '
  '%a = cmp ne i32 %one, 1065353216 %b = cmp ne i64 %minus_two, -4611686018427387904 '
  '%c = cmp ne u32 %nan, 2139095041 %ab = binary or i64 %a, %b '
  '%bad = binary or i64 %ab, %c return i64 %bad }\n')
 subprocess.run([str(cc),'-o',str(exe),str(floating)],check=True); subprocess.run([str(exe)],check=True)
 # A switch can name one phi edge many times. Its transfer block must remain
 # shared; code growth tracks distinct CFG edges, not cases times phi values.
 fan=d/'fanout.lowir'
 rows=['function @main() -> i64 {','block ^entry:', '%which = const i64 13',
       'switch %which, ^other, '+', '.join(f'{k}:^join' for k in range(32)),
       'block ^other:', 'return i64 1', 'block ^join:']
 rows += [f'%p{k} = phi i64 [^entry: {k+1}]' for k in range(32)]
 prior='%p0'
 for k in range(1,32):
  rows.append(f'%s{k} = binary add i64 {prior}, %p{k}'); prior=f'%s{k}'
 rows += [f'%bad = cmp ne i64 {prior}, 528','return i64 %bad','}']
 fan.write_text('\n'.join(rows)+'\n')
 subprocess.run([str(cc),'-o',str(exe),'--dump-machine-ir',str(mir),str(fan)],check=True)
 subprocess.run([str(exe)],check=True)
 assert mir.read_text().count('block ^native') == 1, 'duplicate phi edge transfer blocks'
print('Scalar ABI, atomic updates, widths, multi-file fixups, debug views and ELF checks passed')
