#!/usr/bin/env python3
"""Explicit legacy and reference lifetime traits, independent of course fixtures."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-159/controls').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++'; rows=[]
def run(name,args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=90,cwd=root)
 good=(p.returncode==0)==ok
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=good,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 if not good: print(name,p.returncode,p.stderr.decode(errors='replace'),flush=True)
 return good
for src in sorted((root/'student.tests/pa29/controls159').glob('*.cpp')):
 reject=src.stem.endswith('-reject');obj=out/(src.stem+'.o');exe=out/src.stem
 if run(src.stem+' compile',[cc,'-c',src,'-o',obj],not reject) and not reject:
  if run(src.stem+' link',['g++',obj,'-o',exe]):run(src.stem+' run',[exe])
for trait,n in [('__has_trivial_constructor',1),('__has_nothrow_copy',1),('__has_trivial_copy',1),('__has_trivial_assign',1),('__has_nothrow_assign',1),('__has_nothrow_constructor',1),('__reference_binds_to_temporary',2),('__reference_constructs_from_temporary',2),('__reference_converts_from_temporary',2)]:
 for arity in [n-1,n+1]:
  src=out/'arity.cpp';src.write_text('static_assert('+trait+'('+','.join(['int']*arity)+'),"bad arity");\n')
  run(trait+' arity '+str(arity),[cc,'-c',src,'-o',out/'arity.o'],False)
src=root/'student.tests/pa29/controls159/reference-runtime.cpp';low=out/'dependent.lowir';rt=out/'roundtrip.lowir';exe=out/'native'
if run('typed LowIR',[cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low]):
 if run('LowIR reader',[root/'dev/lowir',low,'-o',rt]):
  rows.append(dict(name='roundtrip bytes',passed=low.read_bytes()==rt.read_bytes()))
  if run('native MIR',[root/'dev/lowir2native','--dump-machine-ir',out/'dependent.mir',rt,'-o',exe]):run('LowIR execute',[exe])
if run('telemetry',[cc,'-c','--stats',src,'-o',out/'stats.o']):rows.append(dict(name='telemetry bytes',passed=(out/'reference-runtime.o').read_bytes()==(out/'stats.o').read_bytes()))
result=dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows)
(out/'results.json').write_text(json.dumps(result,indent=2)+'\n')
print('%d/%d controls passed'%(sum(r['passed'] for r in rows),len(rows)))
sys.exit(any(not r['passed'] for r in rows))
