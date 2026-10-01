#!/usr/bin/env python3
"""Trace aggregate facts through LowIR, native code, debug and PA9 adapters."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';result=dict(binary_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=[],telemetry={},inputs={})
def run(name,args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 result['checks'].append(dict(name=name,status=p.returncode,passed=p.returncode==0,stderr=p.stderr.decode(errors='replace')))
 assert not p.returncode,(name,p.stderr.decode());return p
def check(name,value):
 result['checks'].append(dict(name=name,passed=bool(value)));assert value,name
for src in sorted((root/'student.tests/pa29/controls168').glob('*.cpp')):
 if src.name.startswith('bad-'):continue
 name=src.stem;low=out/(name+'.lowir');roundtrip=out/(name+'.roundtrip');obj=out/(name+'.o');exe=out/name
 result['inputs'][name]=hashlib.sha256(src.read_bytes()).hexdigest()
 run(name+' compile',[cc,'-std=c++14','-O0','-c',src,'-o',obj])
 p=run(name+' stats',[cc,'-std=c++14','-O0','-c','--stats',src,'-o',out/'stats.o'])
 check(name+' telemetry equality',obj.read_bytes()==(out/'stats.o').read_bytes())
 result['telemetry'][name]=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]
 run(name+' LowIR',[cc,'-std=c++14','-c','--emit-lowir','--validate-lowir',src,'-o',low])
 run(name+' roundtrip',[root/'dev/lowir',low,'-o',roundtrip]);check(name+' lossless IR',low.read_bytes()==roundtrip.read_bytes())
 mir=out/(name+'.mir');run(name+' native',[root/'dev/lowir2native','--dump-machine-ir',mir,roundtrip,'-o',exe]);run(name+' native execution',[exe])
 symbols=run(name+' symbols',['nm',obj]).stdout.decode();(out/(name+'.symbols')).write_text(symbols)
 dis=run(name+' instructions',['objdump','-drC',obj]).stdout;(out/(name+'.dis')).write_bytes(dis)
 check(name+' MIR has main','function @main' in mir.read_text())
 if name=='queries':check('designator ABI retained','di1bLi2E' in symbols)
for src in sorted((root/'pa29/tests/compile').glob('600-designated-*.t')):
 obj=out/(src.stem+'.o');exe=out/src.stem
 run(src.stem+' fixture object',[cc,'-c',src,'-o',obj]);run(src.stem+' fixture link',['g++',obj,'-o',exe]);run(src.stem+' fixture execution',[exe])
adapter=out/'abi-designator'
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run('ABI adapter build',['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/abi-designator168.cpp',*objects,'-o',adapter])
facts=run('designator graph/fact roundtrip',[adapter]).stdout;(out/'designator.abi').write_bytes(facts)
run('standalone PA9 designator ABI',[root/'dev/abimangle',out/'designator.abi','-o',out/'designator.name'])
check('designator ABI spelling',(out/'designator.name').read_text().strip()=='DTtlT_di6memberLi3EEE')
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n');print(str(len(result['checks']))+' checks passed')
