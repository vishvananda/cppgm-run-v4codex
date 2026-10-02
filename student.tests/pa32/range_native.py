#!/usr/bin/env python3
"""Fill calls preserve six GP/two FP arguments, snapshots and late addresses."""
import pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 r=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 assert r.returncode==0,(args,r.returncode,r.stderr);return r
text='''function @fill(%p : ptr, %n : i64, %value : ptr, %a : i64, %b : i64, %d : i64, %x : f64, %y : f64) -> i64 [no_inline=yes] {
block ^entry:
 %sum = binary add i64 %a, %b
 %fp = binary add f64 %x, %y
 %last = index i8 %p, %n
 jump ^head
block ^head:
 %ptr = phi ptr [^entry: %p, ^body: %next]
 %more = cmp ne ptr %ptr, %last
 branch %more, ^body, ^done
block ^body:
 %byte = load u8 %value
 store u8 %byte, %ptr
 %next = index i8 %ptr, 1
 jump ^head
block ^done:
 %integer = convert fptosi i64 f64 %fp
 %r0 = binary add i64 %sum, %integer
 %r1 = binary add i64 %r0, %d
 %r2 = binary add i64 %r1, %n
 %eq = cmp eq ptr %ptr, %last
 %r3 = binary add i64 %r2, %eq
 return i64 %r3
}
function @main() -> i32 [role=entry] {
 slot $bytes : obj<64x8>
 block ^entry:
 %p = addr $bytes
 store u8 73, %p
 %r = call i64 @fill(%p, 64, %p, 11, 13, 17, 1.25, 2.75)
 %c = cmp eq i64 %r, 110
 %r0 = call i64 @fill(nullptr, 0, nullptr, 11, 13, 17, 1.25, 2.75)
 %c0 = cmp eq i64 %r0, 46
 %ok = binary and i64 %c, %c0
 %bad = cmp eq i64 %ok, 0
 %result = convert trunc i32 i64 %bad
 return i32 %result
}'''
with tempfile.TemporaryDirectory(prefix='pa32-range-native-') as temp:
 d=pathlib.Path(temp);src=d/'input.lowir';src.write_text(text)
 for level in range(4):
  ir=d/'out.lowir';run(root/'dev/lowiropt',f'-O{level}','-o',ir,src)
  for path in ['original','replay','native']:
   exe=d/f'{path}{level}'
   if path=='native':run(root/'dev/lowir2native','-o',exe,ir)
   else:run(root/'dev/cppgm++',f'-O{level}' if path=='original' else '-O0','-o',exe,src if path=='original' else ir)
   run(exe)
print('range native ABI: six GP/two FP live inputs, zero/64-byte ranges, four levels x three paths PASS')
