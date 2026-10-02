#!/usr/bin/env python3
"""Private-store, readonly-call and exception/CFG conservative guards."""
import pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert p.returncode==0,(args,p.returncode,p.stderr);return p
with tempfile.TemporaryDirectory(prefix='pa32-context-guards-') as d:
 t=pathlib.Path(d)
 src=t/'guards.lowir'
 src.write_text('''declare function @observe(%p : ptr) -> void
function @write_only() -> i64 {
 slot $home : obj<16x8>
 block ^entry: %p = addr $home %q = index i8 %p, 8 store i64 7, %q return i64 0
}
function @escaping() -> i64 {
 slot $home : obj<16x8>
 block ^entry: %p = addr $home store i64 7, %p call void @observe(%p) return i64 0
}
function @volatile() -> i64 {
 slot $home : obj<16x8>
 block ^entry: %p = addr $home store volatile i64 7, %p return i64 0
}
function @outside() -> i64 {
 slot $home : obj<16x8>
 block ^entry: %p = index i8 $home, 16 store i64 7, %p return i64 0
}
function @mutable(%a : ptr) -> i64 {
 slot $home : obj<16x8>
 block ^entry: %p = addr $home %p = copy ptr %a store i64 7, %p return i64 0
}
function @floating(%x : f80) -> i64 {
 slot $home : obj<16x8>
 block ^entry: %p = addr $home store f32 %x, %p return i64 0
}
''')
 for level in range(1,4):
  out=t/'out.lowir';run(root/'dev/lowiropt',f'-O{level}','-o',out,src)
  run(root/'dev/lowir','-o',t/'valid',out)
  text=out.read_text()
  assert 'store' not in text.split('function @write_only')[1].split('}')[0]
  for name in ['escaping','volatile','outside','mutable']:
   assert 'store' in text.split('function @'+name)[1].split('}')[0],name
  floating=text.split('function @floating')[1].split('}')[0]
  assert 'store f32' in floating or 'convert fptrunc f32 f80' in floating,floating
 # The builtin tag alone is not sufficient: definition, ABI, effects, query,
 # active runtime role and direct-call override must agree with the proof.
 for metadata,result,param,decl in [
  ('unwind=no, effects=readonly','i64','ptr',True),
  ('effects=readonly','i64','ptr',True),
  ('unwind=no','i64','ptr',True),
  ('unwind=no, effects=readonly','i32','ptr',True),
  ('unwind=no, effects=readonly','i64','i64',True),
  ('unwind=no, effects=readonly','i64','ptr',False)]:
  declaration=('declare ' if decl else '')+f'function @length(%p : {param}) -> {result} [{metadata}, object=cppgm_builtin_strlen]'
  if not decl:declaration+=' { block ^entry: return i64 99 }'
  arg='@text' if param=='ptr' else '0'
  src.write_text(f'''global @text [binding=internal, storage=readonly] = {{ i32 0 }}
{declaration}
function @entry() -> {result} {{ block ^entry: %v = call {result} @length({arg}) return {result} %v }}
''')
  run(root/'dev/lowiropt','-O1','-o',t/'out.lowir',src)
  text=(t/'out.lowir').read_text().split('function @entry')[1]
  compatible=metadata=='unwind=no, effects=readonly' and result=='i64' and param=='ptr' and decl
  assert ('return i64 0' in text)==compatible,(declaration,text)
print('PA32 contextual guards: PASS (private writes, escape, volatile, out-of-bounds, mutable, float; builtin ABI/effects/definition guards)')
