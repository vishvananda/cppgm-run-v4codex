#!/usr/bin/env python3
"""Inspect external LowIR, emitted MIR/bytes, exception boundaries and debug hints."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
adapter=pathlib.Path(sys.argv[2]).resolve();cc=root/'dev/cppgm++'
result=dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=[],images={},telemetry={})
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 assert (p.returncode==0)==ok,(args,p.returncode,p.stderr.decode())
 return p
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for name in ['assembly','assembly-lifetimes','assembly-thread']:
 src=root/'student.tests/pa29/controls163'/(name+'.cpp');obj=out/(name+'.o');low=out/(name+'.lowir');rt=out/(name+'.roundtrip');exe=out/name
 stats=run([cc,'--stats','-c',src,'-o',obj]);result['telemetry'][name]=[json.loads(s) for s in stats.stderr.decode().splitlines() if s.startswith('{')]
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low]);run([root/'dev/lowir',low,'-o',rt]);assert low.read_bytes()==rt.read_bytes()
 ir_obj=out/(name+'.roundtrip.o');run([adapter,rt,ir_obj])
 host=[src.parent/'thread-host.cpp','-pthread'] if name=='assembly-thread' else []
 run(['g++',ir_obj,*host,'-o',exe]);run([exe])
 native=run(['objdump','-drC',obj]).stdout.decode();(out/(name+'.dis')).write_text(native)
 if name=='assembly':
  assert 'pause' in native and 'nop' in native and 'mfence' in native and 'bswap' in native
  assert 'lock xadd' in native and 'lock cmpxchg' in native and 'xchg' in native
  mir=out/(name+'.mir');run([root/'dev/lowir2native','--dump-machine-ir',mir,rt,'-o',out/'native']);run([out/'native'])
  assert 'pause' in mir.read_text() and 'nop' in mir.read_text()
 if name=='assembly-lifetimes':
  hint=low.read_text().split('object=_Z4hintv',1)[1].split('\n}',1)[0]
  assert 'eh_' not in hint and 'pause' in hint
 result['checks'].append(name+': parsed LowIR object executes; native opcodes and EH boundary inspected')
 result['images'][name]=dict(object_sha256=sha(obj),roundtrip_object_sha256=sha(ir_obj),lowir_sha256=sha(low),same_object=obj.read_bytes()==ir_obj.read_bytes())
low=out/'hints.lowir';low.write_text('''function @entry() -> i32 [role=entry] !dbg(hints.cpp, 1, 1) {
block ^start:
  nop !dbg(hints.cpp, 2, 3)
  pause !dbg(hints.cpp, 3, 3)
  return i32 0 !dbg(hints.cpp, 4, 3)
}
''')
run([root/'dev/lowir',low,'-o',out/'hints.roundtrip'])
run([root/'dev/lowir2native','--dump-machine-ir',out/'hints.mir',low,'-o',out/'hints']);run([out/'hints'])
mir=(out/'hints.mir').read_text();assert 'nop !dbg(hints.cpp, 2, 3)' in mir and 'pause !dbg(hints.cpp, 3, 3)' in mir
run([adapter,low,out/'hints.o']);native=run(['objdump','-d',out/'hints.o']).stdout.decode();assert 'pause' in native and 'nop' in native
# PA8 lowir.md Debug Locations explicitly permits backends without DWARF.
# This inherited host writer has no .debug_line section; MIR is the required view.
result['checks'].append('explicit LowIR hint debug locations survive MIR; ELF contains both instructions')
for body in ['nop 1','%bad = pause','pause i32']:
 low=out/'invalid.lowir';low.write_text('function @entry() -> i32 [role=entry] {block ^start: '+body+'\nreturn i32 0}\n')
 run([root/'dev/lowir',low,'-o',out/'invalid.roundtrip'],False)
result['checks'].append('hint instructions reject operands, results and types')
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n');print('LowIR/MIR/object/debug inspection: PASS')
