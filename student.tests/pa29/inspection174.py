#!/usr/bin/env python3
"""Audit typed IR roundtrips, native execution, volatile effects and machine facts."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[]
def run(name,args):
    p=subprocess.run(list(map(str,args)),capture_output=True,timeout=120)
    rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=not p.returncode,
                     stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace')))
    (out/'results.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),checks=rows),indent=2)+'\n')
    assert not p.returncode,rows[-1]
    return p.stdout.decode()
sources=sorted((root/'student.tests/pa29/controls174').glob('*.cpp'))
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run('explicit LowIR object adapter build',['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),
    root/'student.tests/pa29/inspection174-object.cpp',*objects,'-o',out/'ir-object'])
run('parameter kind adapter build',['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),
    root/'student.tests/pa29/inspection174-graph.cpp',*objects,'-o',out/'abi-graph'])
run('parameter kind identity and roundtrip',[out/'abi-graph'])
for src in sorted((root/'student.tests/pa29/inspection174').glob('*.cpp')):
    symbols={}
    for label,binary,mode in [('student',cc,'c++11'),('host','g++' if src.stem=='packs' else 'clang++','c++20')]:
        obj=out/(src.stem+'-'+label+'.o')
        run(src.stem+' '+label+' object',[binary,'-std='+mode,'-c',src,'-o',obj])
        text=run(src.stem+' '+label+' symbols',['nm','--defined-only',obj])
        symbols[label]={s.split()[-1] for s in text.splitlines() if len(s.split())==3 and s.split()[1]=='W'}
    assert symbols['student'] and symbols['student']==symbols['host'],symbols
for src in sources:
    if src.stem.startswith('bad-'):continue
    name=src.stem;ir=out/(name+'.lowir');again=out/(name+'.roundtrip');native=out/(name+'.native')
    run(name+' LowIR',[cc,'--emit-lowir',src,'-o',ir])
    run(name+' LowIR roundtrip',[root/'dev/lowir',ir,'-o',again]);assert ir.read_bytes()==again.read_bytes()
    if name=='volatile-throw':
        run(name+' MIR',[root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),again])
        run(name+' LowIR host object',[out/'ir-object',again,out/(name+'.o')])
        run(name+' host link',['g++',out/(name+'.o'),'-o',native])
    else:
        run(name+' MIR and native',[root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),again,'-o',native])
    run(name+' native runtime',[native])
text=(out/'volatile-scalar.lowir').read_text()
direct=text.split('function @direct(')[1].split('\n}')[0]
calls=text.split('function @calls(')[1].split('\n}')[0]
assert direct.count('load volatile i32')==3
assert 'load volatile' not in calls and calls.count('call ptr @ref')==3
src=root/'student.tests/pa29/controls174/integrated.cpp'
run('integrated AST',[cc,'--emit-ast',src,'-o',out/'integrated.ast'])
for stats in [False,True]:
    run('integrated object stats='+str(stats),[cc,*(['--stats'] if stats else []),'-c',src,'-o',out/('stats.o' if stats else 'plain.o')])
assert (out/'stats.o').read_bytes()==(out/'plain.o').read_bytes()
run('integrated ELF',['readelf','-SWs',out/'plain.o'])
run('integrated disassembly',['objdump','-dr',out/'plain.o'])
symbols={}
for label,binary,mode in [('student',cc,'c++11'),('host','clang++','c++20')]:
    obj=out/(label+'-abi.o')
    run(label+' integrated ABI object',[binary,'-std='+mode,'-DABI_PEER','-c',src,'-o',obj])
    text=run(label+' integrated symbols',['nm','--defined-only',obj])
    symbols[label]={s.split()[-1] for s in text.splitlines() if '_Z5first' in s or '_ZZ5first' in s}
assert len(symbols['student'])==2 and symbols['student']==symbols['host'],symbols
run('integrated host declaration',['clang++','-std=c++20','-c',root/'student.tests/pa29/inspection174-peer.cpp','-o',out/'peer.o'])
run('integrated host peer link',['g++',out/'peer.o',out/'student-abi.o','-o',out/'peer'])
run('integrated host peer runtime',[out/'peer'])
for src in sources:
    if src.stem.startswith('capture-') or src.stem=='integrated':
        run(src.stem+' host compile',['clang++','-std=c++20','-c',src,'-o',out/'host.o'])
        run(src.stem+' host link',['g++',out/'host.o','-o',out/'host'])
        run(src.stem+' host runtime',[out/'host'])
print(len(rows),'inspection checks passed',flush=True)
