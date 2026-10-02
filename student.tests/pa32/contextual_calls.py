#!/usr/bin/env python3
"""Contextual no-unwind proofs, immutable readonly facts and region joins."""
import pathlib,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[2]
def run(*args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
 assert p.returncode==0,(args,p.returncode,p.stdout,p.stderr)
 return p
with tempfile.TemporaryDirectory(prefix='pa32-context-') as directory:
 t=pathlib.Path(directory)
 src=t/'context.lowir'
 src.write_text('''declare function @length(%p : ptr) -> i64 [unwind=no, effects=readonly, object=cppgm_builtin_strlen]
function @throwing() -> void { block ^entry: throw i64 13 }
global @text [binding=internal, storage=readonly] = { i8 65 i8 66 i8 67 i8 0 }
global @writable [binding=internal] = { i8 65 i8 0 }
global @weak [binding=weak, storage=readonly] = { i8 65 i8 0 }
global @unnulled [binding=internal, storage=readonly] = { i8 65 }
function @bounded(%p : ptr, %n : u8) -> i64 [binding=weak] {
 block ^entry: eh_try ^landing
 %bad = cmp uge u8 %n, 4
 branch %bad, ^throw, ^load
 block ^throw: call void @throwing() return i64 -1
 block ^load: %a = index i64 %p, %n %v = load i64 %a eh_end return i64 %v
 block ^landing: eh_catch_all, 1 return i64 -2
}
function @string_length(%p : ptr) -> i64 [binding=weak] {
 block ^entry: eh_try ^landing
 %n = call i64 @length(%p)
 %bad = cmp eq i64 %n, -1
 branch %bad, ^throw, ^good
 block ^throw: call void @throwing() return i64 -1
 block ^good: eh_end return i64 %n
 block ^landing: eh_catch_all, 1 return i64 -2
}
function @mutable(%p : ptr, %n : u8) -> i64 {
 block ^entry: %n = copy u8 7 %v = call i64 @bounded(%p, %n) return i64 %v
}
function @unknown(%p : ptr, %n : u8) -> i64 {
 block ^entry: %v = call i64 @bounded(%p, %n) return i64 %v
}
function @known(%p : ptr) -> i64 {
 block ^entry: %v = call i64 @bounded(%p, 258) return i64 %v
}
function @string_known() -> i64 {
 block ^entry: %a = addr @text %v = call i64 @string_length(%a) return i64 %v
}
function @string_writable() -> i64 {
 block ^entry: %a = addr @writable %v = call i64 @string_length(%a) return i64 %v
}
function @string_weak() -> i64 {
 block ^entry: %a = addr @weak %v = call i64 @string_length(%a) return i64 %v
}
function @string_unnulled() -> i64 {
 block ^entry: %a = addr @unnulled %v = call i64 @string_length(%a) return i64 %v
}
function @main() -> i64 [role=entry] {
 slot $array : obj<32x8>
 block ^entry: %a = index i64 $array, 2 store i64 71, %a
 %ptr = addr $array %v = call i64 @known(%ptr) %s = call i64 @string_known()
 %b = cmp ne i64 %v, 71 %c = cmp ne i64 %s, 3
 %r = binary or i64 %b, %c return i64 %r
}
''')
 for level in range(4):
  out=t/'optimized.lowir';run(root/'dev/lowiropt',f'-O{level}','-o',out,src)
  run(root/'dev/lowir','-o',t/'valid',out)
  run(root/'dev/lowir2native','-o',t/'native',out);run(t/'native')
  if level:
   text=out.read_text()
   for name in ['known','string_known']:
    body=text.split('function @'+name+'(')[1].split('}')[0]
    assert 'call ' not in body and 'eh_try' not in body,(name,body)
   for name,callee in [('unknown','bounded'),('string_writable','string_length'),('string_weak','string_length'),('string_unnulled','string_length')]:
    body=text.split('function @'+name+'(')[1].split('}')[0]
    assert '@'+callee+'(' in body,(name,body)
 # Real unwinding and source region-state correction, with direct/O0 replay
 # objects compared at each level, including meaningful debug metadata.
 cpp=t/'nested.cpp';cpp.write_text((root/'student.tests/pa32/call-nested-cleanup.cpp').read_text())
 for level in range(4):
  run(root/'dev/cppgm++','--emit-lowir','-O0','-gline-tables-only','-o',t/'nested.lowir',cpp)
  for source,label in [(cpp,'direct'),(t/'nested.lowir','replay')]:
   run(root/'dev/cppgm++',f'-O{level}','-gline-tables-only','-c',source,'-o',t/(label+'.o'))
   run('g++',t/(label+'.o'),'-o',t/label);run(t/label)
  assert (t/'direct.o').read_bytes()==(t/'replay.o').read_bytes(),level
 # Forward-merge source order and phi predecessor repair across a handler.
 src.write_text('''function @main() -> i64 [role=entry] {
 block ^entry: eh_try ^landing jump ^later
 block ^other: return i64 99
 block ^later: %a = const i64 4 eh_end jump ^merge
 block ^landing: eh_catch_all, 1 %b = const i64 7 jump ^merge
 block ^merge: %v = phi i64 [^later: %a, ^landing: %b]
 %bad = cmp ne i64 %v, 4 return i64 %bad
}
''')
 for level in range(4):
  run(root/'dev/lowiropt',f'-O{level}','-o',t/'merge.lowir',src)
  run(root/'dev/lowir','-o',t/'valid',t/'merge.lowir')
  run(root/'dev/lowir2native','-o',t/'merge',t/'merge.lowir');run(t/'merge')
print('PA32 contextual calls: PASS (known/unknown/readonly/boundary guards; direct/replay/native; nested source EH and debug objects; phi repair)')
