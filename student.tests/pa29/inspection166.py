#!/usr/bin/env python3
"""Trace audit controls through typed LowIR, MIR and direct ELF emission."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
adapter=pathlib.Path(sys.argv[2]).resolve();cc=root/'dev/cppgm++'
result=dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=[],files={},telemetry={})
def run(name,args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 result['checks'].append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=p.returncode==0,stderr=p.stderr.decode(errors='replace')))
 assert p.returncode==0,(name,p.returncode,p.stderr.decode())
 return p
def check(name,value):
 result['checks'].append(dict(name=name,passed=bool(value)))
 assert value,name
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for src in sorted((root/'student.tests/pa29/controls166').glob('*.cpp')):
 name=src.stem;obj=out/(name+'.o');low=out/(name+'.lowir');rt=out/(name+'.roundtrip');exe=out/name
 run(name+' object',[cc,'-std=c++14','-O0','-c',src,'-o',obj])
 stats=run(name+' stats',[cc,'-std=c++14','-O0','--stats','-c',src,'-o',out/'stats.o'])
 check(name+' telemetry byte equality',obj.read_bytes()==(out/'stats.o').read_bytes())
 result['telemetry'][name]=[json.loads(s) for s in stats.stderr.decode().splitlines() if s.startswith('{')]
 run(name+' typed IR',[cc,'-std=c++14','-c','--emit-lowir','--validate-lowir',src,'-o',low])
 run(name+' roundtrip',[root/'dev/lowir',low,'-o',rt]);check(name+' roundtrip bytes',low.read_bytes()==rt.read_bytes())
 ir_obj=out/(name+'.roundtrip.o');run(name+' IR object',[adapter,rt,ir_obj])
 run(name+' IR link',['g++',ir_obj,'-o',exe]);run(name+' IR execution',[exe])
 native=run(name+' disassembly',['objdump','-drC',obj]).stdout
 (out/(name+'.dis')).write_bytes(native)
 mirrored=run(name+' IR disassembly',['objdump','-drC',ir_obj]).stdout
 check(name+' encoding/relocation equality',native.splitlines()[2:]==mirrored.splitlines()[2:])
 mir=out/(name+'.mir');standalone=out/(name+'.native')
 run(name+' MIR',[root/'dev/lowir2native','--dump-machine-ir',mir,rt,'-o',standalone]);run(name+' standalone',[standalone])
 result['files'][name]={p.name:sha(p) for p in [src,obj,low,rt,ir_obj,mir]}
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n')
print('%d/%d checks passed'%(sum(r['passed'] for r in result['checks']),len(result['checks'])),flush=True)
