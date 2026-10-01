#!/usr/bin/env python3
"""Vector layout and wrapper demand through serialized LowIR and native output."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';result=dict(binary_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=[],telemetry={},inputs={})
def run(name,args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90)
 result['checks'].append(dict(name=name,status=p.returncode,passed=p.returncode==0,stderr=p.stderr.decode(errors='replace')))
 assert not p.returncode,(name,p.stderr.decode());return p
def check(name,value):
 result['checks'].append(dict(name=name,passed=bool(value)));assert value,name
for src in sorted((root/'student.tests/pa29/controls167').glob('*.cpp')):
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
 if name=='inline-demand':
  check('unused unsupported body omitted','unsupported' not in symbols and 'dormant' not in symbols)
  check('evaluated uses retained',all(s in symbols for s in ['used_leaf','used_branch','address_only']))
 if name=='vector-pointer-abi':check('PA9 vector ABI names',all(s in symbols for s in ['_Z6extentPKDv4_i','_Z6extentPKDv8_i','_Z6extentPKDv4_f']))
 if name=='layout':check('unused vector literal wrappers omitted',all(s not in symbols for s in ['full','partial','leading']))
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n');print(str(len(result['checks']))+' checks passed')
