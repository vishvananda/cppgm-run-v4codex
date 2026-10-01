#!/usr/bin/env python3
"""Retained lambda facts, ABI peers, explicit ABI/LowIR views and native output."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';src=root/'student.tests/pa29/inspection173';rows=[]
def run(name,args):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=120)
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=p.returncode==0,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
 (out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
 assert not p.returncode,rows[-1];return p.stdout.decode()
for name in ['student','template']:
 for label,binary in [('student',cc),('host','clang++')]:
  run(name+' '+label+' object',[binary,'-std=c++11' if label=='student' else '-std=c++20','-c',src/(name+'.cpp'),'-o',out/(name+'-'+label+'.o')])
 symbols={label:run(name+' '+label+' symbols',['nm','--defined-only',out/(name+'-'+label+'.o')]) for label in ['student','host']}
 selected={k:{' '.join(l.split()[1:]) for l in s.splitlines() if '_clI' in l and 'callback' not in l} for k,s in symbols.items()}
 assert selected['student']==selected['host'],selected
for label,binary in [('student',cc),('host','clang++')]:
 run(label+' peer compile',[binary,'-std=c++11' if label=='student' else '-std=c++20','-c',src/'host.cpp','-o',out/'peer.o'])
 run(label+' peer link',['g++',out/'student-student.o',out/'peer.o','-o',out/'peer'])
 run(label+' peer runtime',[out/'peer'])
source=root/'student.tests/pa29/controls173/calls.cpp'
for mode in ['--emit-ast','--emit-lowir']:
 run(mode,[cc,mode,source,'-o',out/(mode[7:]+'.txt')])
run('LowIR roundtrip',[root/'dev/lowir',out/'lowir.txt','-o',out/'roundtrip.lowir'])
assert (out/'lowir.txt').read_bytes()==(out/'roundtrip.lowir').read_bytes()
run('MIR and native',[root/'dev/lowir2native','--dump-machine-ir',out/'closures.mir',out/'roundtrip.lowir','-o',out/'native'])
run('native runtime',[out/'native'])
run('telemetry object',[cc,'--stats','-c',src/'student.cpp','-o',out/'stats.o'])
assert (out/'stats.o').read_bytes()==(out/'student-student.o').read_bytes()
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run('ABI adapter build',['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),src/'abi-graph.cpp',*objects,'-o',out/'abi-graph'])
facts=run('ABI graph/fact roundtrip',[out/'abi-graph']);(out/'graph.abi').write_text(facts)
run('standalone ABI',[root/'dev/abimangle',out/'graph.abi','-o',out/'graph.name'])
assert (out/'graph.name').read_text().strip()=='Z1fvEUlTyT_E_'
print(str(len(rows))+' inspection checks passed')
