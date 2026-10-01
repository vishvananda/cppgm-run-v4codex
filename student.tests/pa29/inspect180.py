#!/usr/bin/env python3
"""Source/LowIR/MIR/ELF equivalence and explicit inline CFG and budget controls."""
import pathlib,subprocess,sys,json,hashlib
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
controls=pathlib.Path(sys.argv[2]).resolve();cc=root/'dev/cppgm++';rows=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),cwd=root,text=True,capture_output=True,timeout=120)
 rows.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr));(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=sha(cc),rows=rows),indent=2)+'\n')
 assert (p.returncode==0)==ok,rows[-1]
 return p.stdout
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/inspection180-object.cpp',*objects,'-o',out/'ir-object'])
for name in ['static-effects','static-template','static-query','inline-basic','inline-branch','inline-loop','inline-nested','inline-object','inline-throw','inline-catch','inline-rethrow','inline-noreturn','inline-float']:
 src=controls/(name+'.cpp');ir=out/(name+'.lowir');obj=out/(name+'.o')
 run([cc,'--emit-ast',src,'-o',out/(name+'.ast')])
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',ir])
 run([root/'dev/lowir',ir,'-o',out/(name+'.roundtrip')]);assert ir.read_bytes()==(out/(name+'.roundtrip')).read_bytes()
 run([out/'ir-object',ir,out/(name+'.prepared.lowir'),out/(name+'.adapter.o')])
 assert ir.read_bytes()==(out/(name+'.prepared.lowir')).read_bytes()
 run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),ir])
 run(['g++',out/(name+'.adapter.o'),'-o',out/(name+'.adapter')]);run([out/(name+'.adapter')])
 run([cc,'-O0','--stats','-c',src,'-o',obj]);run([cc,'-O0','-c',src,'-o',out/(name+'.plain.o')]);assert obj.read_bytes()==(out/(name+'.plain.o')).read_bytes()
 # The reader publishes declarations before definitions, which can reorder
 # ELF symbol-table indices. Compare instructions and symbolic relocations,
 # and the complete symbol set; both paths already link and execute above.
 direct=run(['objdump','-dr',obj]).splitlines()[3:]
 adapter=run(['objdump','-dr',out/(name+'.adapter.o')]).splitlines()[3:]
 assert direct==adapter
 assert sorted(run(['nm',obj]).splitlines())==sorted(run(['nm',out/(name+'.adapter.o')]).splitlines())
 run(['nm','-C',obj]);run(['readelf','-rSW',obj]);run(['readelf','--debug-dump=frames',obj]);run(['objdump','-dr',obj])
# Both caller and callee phi predecessors must name split block exits.
fixtures={
'phi':'''function @twice(%x : i32) -> i32 [force_inline=yes] {
 block ^start:
  %c = cmp gt i32 %x, 0
  branch %c, ^positive, ^negative
 block ^positive:
  %y = binary mul i32 %x, 2 !dbg(callee.cpp, 8, 3)
  jump ^end
 block ^negative:
  jump ^end
 block ^end:
  %r = phi i32 [^positive: %y, ^negative: 0]
  return i32 %r
}
function @main() -> i32 [role=entry] {
 block ^start:
  branch 1, ^a, ^b
 block ^a:
  %x = call i32 @twice(3)
  jump ^join
 block ^b:
  %y = call i32 @twice(-1)
  jump ^join
 block ^join:
  %r = phi i32 [^a: %x, ^b: %y]
  %z = binary sub i32 %r, 6
  return i32 %z
}''',
'by-address':'''function @write(%p : ptr [pass=by_address]) -> i32 [force_inline=yes] {
 block ^entry:
  %x = load i32 %p
  store i32 8, %p
  return i32 %x
}
function @main() -> i32 [role=entry] {
 slot $v : i32
 block ^entry:
  store i32 4, $v
  %x = call i32 @write($v)
  %y = load i32 $v
  %sum = binary add i32 %x, %y
  %r = binary sub i32 %sum, 12
  return i32 %r
}''',
'no-return':'''function @forever() -> void [force_inline=yes, return=noreturn] {
 block ^loop:
  jump ^loop
}
function @test(%x : i32) -> i32 {
 block ^entry:
  %c = cmp lt i32 %x, 0
  branch %c, ^bad, ^good
 block ^bad:
  call void @forever()
  unreachable
 block ^good:
  return i32 %x
}
function @main() -> i32 [role=entry] {
 block ^entry:
  %x = call i32 @test(0)
  return i32 %x
}''',
'noinline':'''function @keep() -> i32 [force_inline=yes, no_inline=yes] {
 block ^entry:
  return i32 0
}
function @main() -> i32 [role=entry] {
 block ^entry:
  %x = call i32 @keep()
  return i32 %x
}''',
}
for name,text in fixtures.items():
 ir=out/(name+'.lowir');ir.write_text(text+'\n');prepared=out/(name+'.prepared.lowir');obj=out/(name+'.o')
 counts=run([out/'ir-object',ir,prepared,obj]);run([root/'dev/lowir',prepared,'-o',out/(name+'.roundtrip')])
 run(['g++',obj,'-o',out/name]);run([out/name]);run([root/'dev/lowir2native',ir,'-o',out/(name+'.native')]);run([out/(name+'.native')])
 if name=='phi':
  assert prepared.read_text().count('!dbg(callee.cpp, 8, 3)')==3
  run([root/'dev/lowir2native',ir,'--dump-machine-ir',out/'phi.mir'])
  assert 'callee.cpp' in (out/'phi.mir').read_text()
 assert ('call i32 @keep' in prepared.read_text()) if name=='noinline' else int(counts.split()[0])>0
# Mandatory-call eligibility has conservative fallbacks; input storage and output are
# bounded even when a short acyclic call graph denotes exponential expansion.
for name,text,diagnostic in [
 ('cycle','function @f() -> void [force_inline=yes] { block ^b: call void @f() return void }','recursive'),
 ('depth','\n'.join('function @f%d() -> void [force_inline=yes] {block ^b: %s return void}'%(i,('call void @f%d()'%(i+1)) if i<66 else '') for i in range(67)),'deep'),
 ('growth','\n'.join('function @f%d() -> void [force_inline=yes] {block ^b: %s return void}'%(i,('call void @f%d() call void @f%d()'%(i+1,i+1)) if i<22 else 'nop') for i in range(23)),'budget'),
 ('frame','function @f() -> ptr [force_inline=yes] {block ^b: %p = stack_alloc 8 return ptr %p} function @main() -> i32 [role=entry] {block ^b: %p = call ptr @f() return i32 0}','stack'),
]:
 ir=out/(name+'.lowir');ir.write_text(text+'\n');counts=run([out/'ir-object',ir,out/(name+'.prepared'),out/(name+'.o')]).split()
 assert int(counts[2])>0 and int(counts[1])<=int(counts[3])<=4194304 and int(counts[4])<=262144
 assert 'call ' in (out/(name+'.prepared')).read_text()
print('inspection passed',len(rows),flush=True)
