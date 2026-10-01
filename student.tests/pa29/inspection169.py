#!/usr/bin/env python3
"""Trace block declarations/calls through serialized LowIR, MIR and ELF."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';result=dict(binary_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=[],telemetry={},inputs={})
def run(name,args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 result['checks'].append(dict(name=name,status=p.returncode,passed=p.returncode==0,stderr=p.stderr.decode(errors='replace')))
 assert not p.returncode,(name,p.stderr.decode());return p
def check(name,value):
 result['checks'].append(dict(name=name,passed=bool(value)));assert value,name
for src in sorted((root/'student.tests/pa29/controls169').glob('*.cpp')):
 if src.name.startswith('bad-') or src.name.endswith('.host.cpp'):continue
 name=src.stem;low=out/(name+'.lowir');roundtrip=out/(name+'.roundtrip');obj=out/(name+'.o');exe=out/name
 result['inputs'][name]=hashlib.sha256(src.read_bytes()).hexdigest()
 run(name+' compile',[cc,'-std=c++11','-O0','-c',src,'-o',obj])
 p=run(name+' stats',[cc,'-std=c++11','-O0','-c','--stats',src,'-o',out/'stats.o'])
 check(name+' telemetry equality',obj.read_bytes()==(out/'stats.o').read_bytes())
 result['telemetry'][name]=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]
 run(name+' LowIR',[cc,'-std=c++11','-c','--emit-lowir','--validate-lowir',src,'-o',low])
 run(name+' roundtrip',[root/'dev/lowir',low,'-o',roundtrip]);check(name+' lossless IR',low.read_bytes()==roundtrip.read_bytes())
 if not src.with_suffix('.host.cpp').exists():
  mir=out/(name+'.mir');run(name+' native',[root/'dev/lowir2native','--dump-machine-ir',mir,roundtrip,'-o',exe]);run(name+' native execution',[exe])
  check(name+' MIR has main','function @main' in mir.read_text())
 symbols=run(name+' symbols',['nm',obj]).stdout.decode();(out/(name+'.symbols')).write_text(symbols)
 dis=run(name+' instructions',['objdump','-drC',obj]).stdout;(out/(name+'.dis')).write_bytes(dis)
 if name=='abi':
  check('block ABI name retained','_Z8call_intU13block_pointerFiiEi' in symbols)
  check('block RTTI identity','_ZTIU13block_pointerFiiE' in symbols)
 if name=='invoke':
  check('explicit block invocation load','index i8' in low.read_text() and '16' in low.read_text())
for src in sorted((root/'pa29/tests/compile').glob('*block-pointer*.t')):
 obj=out/(src.stem+'.o');exe=out/src.stem
 run(src.stem+' fixture object',[cc,'-c',src,'-o',obj]);run(src.stem+' fixture link',['g++',obj,'-o',exe]);run(src.stem+' fixture execution',[exe])
adapter=out/'abi-block'
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run('ABI adapter build',['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/abi-block169.cpp',*objects,'-o',adapter])
facts=run('block graph/fact roundtrip',[adapter]).stdout;(out/'block.abi').write_bytes(facts)
run('standalone PA9 block ABI',[root/'dev/abimangle',out/'block.abi','-o',out/'block.name'])
check('block ABI spelling',(out/'block.name').read_text().strip()=='U13block_pointerFiiE')
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n');print(str(len(result['checks']))+' checks passed')
