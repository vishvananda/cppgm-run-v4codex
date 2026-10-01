#!/usr/bin/env python3
"""Trace integrated demand through typed views, serialized adapters and native ELF."""
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
rows=[];cc=root/'dev/cppgm++'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
    rows.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr))
    (out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=sha(cc),rows=rows),indent=2)+'\n')
    assert not p.returncode,rows[-1]
    return p.stdout
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/inspection174-object.cpp',*objects,'-o',out/'ir-object'])
for name in ['integrated','source-storage','inline-unevaluated','inline-bound-inner','default-frames','selection-source']:
    src=root/('student.tests/pa29/source178/'+name+'.cpp')
    ir=out/(name+'.lowir');again=out/(name+'.roundtrip');obj=out/(name+'.o')
    run([cc,'--emit-ast',src,'-o',out/(name+'.ast')])
    run([cc,'--emit-lowir',src,'-o',ir]);run([root/'dev/lowir',ir,'-o',again]);assert ir.read_bytes()==again.read_bytes()
    run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),again])
    run([out/'ir-object',again,out/(name+'.adapter.o')])
    run(['g++',out/(name+'.adapter.o'),'-o',out/(name+'.adapter')]);run([out/(name+'.adapter')])
    run([cc,'-O0','--stats','-c',src,'-o',obj]);run([cc,'-O0','-c',src,'-o',out/(name+'.plain.o')])
    assert obj.read_bytes()==(out/(name+'.plain.o')).read_bytes()
    run(['g++',obj,'-o',out/name]);run([out/name])
    run(['nm','-C',obj]);run(['readelf','-rSW',obj]);run(['readelf','--debug-dump=frames',obj]);run(['objdump','-dr',obj])
text=(out/'inline-bound-inner.lowir').read_text();assert 'function @dormant' not in text
text=(out/'source-storage.lowir').read_text();assert text.count('global @__source_string_')==2
text=(out/'integrated.lowir').read_text();assert 'missing' not in text and not re.search(r'call[^\n]*@__builtin_LINE\(',text)
print('inspection passed',len(rows),flush=True)
