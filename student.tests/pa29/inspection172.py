#!/usr/bin/env python3
"""Actual dependent fold ABI symbols, host peer and typed IR views."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';source=root/'student.tests/pa29/controls172/abi.cpp';rows=[]
def run(name,args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=90)
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=p.returncode==0,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 assert not p.returncode,rows[-1]
 return p.stdout.decode()
for label,binary in [('student',cc),('host','clang++')]:
 run(label+' object',[binary,'-std=c++11','-DABI_PEER','-c',source,'-o',out/(label+'.o')])
symbols={k:run(k+' symbols',['nm','--defined-only',out/(k+'.o')]) for k in ['student','host']}
assert {l.split()[-1] for l in symbols['student'].splitlines()}=={l.split()[-1] for l in symbols['host'].splitlines()}
run('host peer',['clang++','-std=c++11','-c',root/'student.tests/pa29/inspection172/abi-peer.cpp','-o',out/'peer.o'])
run('host link',['g++',out/'student.o',out/'peer.o','-o',out/'peer'])
run('host runtime',[out/'peer'])
# PA6/7 views intentionally run the pre-template semantic stages. The parser
# view and PA29's production LowIR/MIR surfaces own these template inputs.
for mode in ['--emit-ast','--emit-lowir']:
 run(mode,[cc,mode,root/'student.tests/pa29/controls172/side-effects.cpp','-o',out/(mode[7:]+'.txt')])
run('LowIR roundtrip',[root/'dev/lowir',out/'lowir.txt','-o',out/'roundtrip.lowir'])
assert (out/'lowir.txt').read_bytes()==(out/'roundtrip.lowir').read_bytes()
run('MIR and native',[root/'dev/lowir2native','--dump-machine-ir',out/'fold.mir',out/'roundtrip.lowir','-o',out/'native'])
run('native runtime',[out/'native'])
run('telemetry object',[cc,'-std=c++11','-DABI_PEER','--stats','-c',source,'-o',out/'stats.o'])
assert (out/'stats.o').read_bytes()==(out/'student.o').read_bytes()
(out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),telemetry_object_equal=True,symbols_equal=True,source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
print(str(len(rows))+' inspection checks passed')
