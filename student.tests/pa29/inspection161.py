#!/usr/bin/env python3
"""Inspect the same typed LowIR used for native output, and canonical atomic ABI."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-161/inspection').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';result={'compiler_sha256':hashlib.sha256(cc.read_bytes()).hexdigest(),'checks':[],'files':{}}
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,cwd=root,timeout=90)
 assert p.returncode==0,(str(args[0]),p.returncode,p.stderr.decode());return p
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
adapter=out/'ir-object'
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/ir-object161.cpp',*objects,'-o',adapter])
for name in ['atomic-runtime','atomic-padding','atomic-fallback','atomic-query','atomic-identity']:
 src=root/'student.tests/pa29/controls161'/(name+'.cpp');obj=out/(name+'.o');low=out/(name+'.lowir');rt=out/(name+'.roundtrip');exe=out/name
 run([cc,'-c',src,'-o',obj]);stats=run([cc,'-c','--stats',src,'-o',out/'stats.o'])
 assert obj.read_bytes()==(out/'stats.o').read_bytes()
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low]);run([root/'dev/lowir',low,'-o',rt]);assert low.read_bytes()==rt.read_bytes()
 # The external LowIR adapter emits a host object; libatomic links generic fallbacks.
 ir_obj=out/(name+'.roundtrip.o')
 run([adapter,rt,ir_obj]);
 run(['g++',ir_obj,'-latomic','-o',exe]);run([exe])
 files=[obj,low,rt,ir_obj]
 if name!='atomic-fallback':
  mir=out/(name+'.mir');standalone=out/(name+'.standalone')
  run([root/'dev/lowir2native','--dump-machine-ir',mir,rt,'-o',standalone]);run([standalone]);files.append(mir)
 result['checks'].append(name+': telemetry byte equality, LowIR validation/roundtrip, object/native execution')
 result['files'][name]={p.name:sha(p) for p in files}
 if name=='atomic-runtime':
  text=low.read_text();assert all(op in text for op in ['atomic_load','atomic_store','atomic_exchange','atomic_compare_exchange','atomic_add_fetch','atomic_thread_fence','atomic_signal_fence'])
  dis=run(['objdump','-drC',obj]).stdout.decode();assert 'lock cmpxchg' in dis and 'lock xadd' in dis and 'cmpxchg16b' in dis
  result['checks'].append('atomic operations survive serialization and native lock instructions')
 if name=='atomic-identity':
  symbols=run(['nm',obj]).stdout.decode();assert '_Z8distinctU7_Atomici' in symbols and '_Z8overloadU7_Atomici' in symbols and '_Z8overloadi' in symbols
  result['checks'].append('distinct scalar and atomic parameter ABI identities')
result['phase_counters']=[json.loads(s) for s in stats.stderr.decode().splitlines() if s.startswith('{')]
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n');print('typed LowIR, native locks, ABI and telemetry checks: PASS')
