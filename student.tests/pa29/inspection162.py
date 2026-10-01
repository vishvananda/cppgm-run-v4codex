#!/usr/bin/env python3
"""Audit fixes through explicit LowIR, native encoding and telemetry views."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
adapter=pathlib.Path(sys.argv[2]).resolve();cc=root/'dev/cppgm++'
result={'compiler_sha256':hashlib.sha256(cc.read_bytes()).hexdigest(),'checks':[],'files':{}}
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 assert p.returncode==0,(args,p.returncode,p.stderr.decode());return p
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for name in ['atomic-qualifiers','atomic-bool','atomic-reference-lifetime','atomic-reference-recipes','atomic-generic-template','atomic-padded-align-generic','atomic-scalar-alignment']:
 src=root/'student.tests/pa29/controls162'/(name+'.cpp');obj=out/(name+'.o');low=out/(name+'.lowir');rt=out/(name+'.roundtrip');exe=out/name
 run([cc,'-c',src,'-o',obj]);stats=run([cc,'-c','--stats',src,'-o',out/'stats.o'])
 assert obj.read_bytes()==(out/'stats.o').read_bytes()
 run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low]);run([root/'dev/lowir',low,'-o',rt]);assert low.read_bytes()==rt.read_bytes()
 ir_obj=out/(name+'.roundtrip.o');run([adapter,rt,ir_obj]);run(['g++',ir_obj,'-latomic','-o',exe]);run([exe])
 dis=out/(name+'.disassembly');dis.write_bytes(run(['objdump','-drC',obj]).stdout)
 text=low.read_text();native=dis.read_text()
 if 'alignment' in name or 'generic' in name:
  assert '__atomic_load' in native and '__atomic_store' in native
  assert 'cmpxchg16b' not in native
  result['checks'].append(name+': recorded alignment selects runtime ABI without cmpxchg16b')
 else:
  mir=out/(name+'.mir');standalone=out/(name+'.native')
  run([root/'dev/lowir2native','--dump-machine-ir',mir,rt,'-o',standalone]);run([standalone])
  result['files'][mir.name]=sha(mir)
  assert 'atomic_load' in text
 result['checks'].append(name+': telemetry equality, LowIR validation/roundtrip and linked execution')
 result['files'].update({p.name:sha(p) for p in [obj,low,rt,ir_obj,dis]})
 result.setdefault('telemetry',{})[name]=[json.loads(s) for s in stats.stderr.decode().splitlines() if s.startswith('{')]
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n')
print('audit LowIR/native/telemetry inspection: PASS',flush=True)
