#!/usr/bin/env python3
"""Explicit full-stage integration and extended NaN adapter controls."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
src = root / 'student.tests/pa29/source194'
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
cc = root / 'dev/cppgm++'
rows, properties = [], []
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def save():
    (out/'checks.json').write_text(json.dumps(dict(compiler=sha(cc),
        sources={p.name:sha(p) for p in sorted(src.iterdir())},
        commands=rows, properties=properties), indent=2)+'\n')
def run(args):
    args = list(map(str,args))
    p = subprocess.run(args, cwd=root, capture_output=True, text=True, timeout=90)
    rows.append(dict(command=args, status=p.returncode, stdout=p.stdout, stderr=p.stderr)); save()
    assert p.returncode == 0, rows[-1]
    return p.stdout
def check(ok, description):
    properties.append(dict(description=description, passed=bool(ok))); save()
    assert ok, description
objects = run(['make','-s','-C',root/'dev','--eval',
    'print-objects: ; @echo $(call frontend_objs,cppgm++) $(call frontend_objs,lowir)','print-objects']).split()
objects = list(dict.fromkeys(str((root/'dev'/p).resolve()) for p in objects))
run(['g++','-std=c++11','-I'+str(root/'dev/src'),
     root/'student.tests/pa29/source192/lowir-host.cpp',*objects,'-o',out/'lowir-host'])
run([root/'dev/lowir',src/'nan.lowir','-o',out/'nan.canonical.lowir'])
run([root/'dev/lowir',out/'nan.canonical.lowir','-o',out/'nan.twice.lowir'])
check((out/'nan.canonical.lowir').read_bytes()==(out/'nan.twice.lowir').read_bytes(), 'NaN text is idempotent')
check('return f128 -snanQ' in (out/'nan.canonical.lowir').read_text(), 'quad signaling spelling survives')
for name,ir in [('raw',src/'nan.lowir'),('canonical',out/'nan.canonical.lowir')]:
    run([out/'lowir-host',ir,out/(name+'.o')])
    run(['g++','-O0',src/'nan-host.cpp',out/(name+'.o'),'-o',out/name])
    run([out/name])
    run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),ir])
for option in ['-dr','-s']:
    # The writer orders globals before functions, changing symbol table order.
    # Compare every displayed section's bytes and symbolic relocations instead.
    views = [run(['objdump',option,out/(name+'.o')]).splitlines()[2:] for name in ['raw','canonical']]
    check(views[0]==views[1], 'NaN direct/adapter sections agree: '+option)
check(run(['nm',out/'raw.o'])==run(['nm',out/'canonical.o']), 'NaN direct/adapter symbols agree')
check('-snan' in (out/'raw.mir').read_text(), 'MIR preserves signaling representation')
for name in ['integrated','nan-source','quad-classification']:
    source=src/(name+'.cpp')
    for level in ['-O0','-O2']:
        obj=out/(name+level+'.o'); exe=out/(name+level)
        run([cc,level,'-c',source,'-o',obj]); run(['g++',obj,'-lm','-o',exe]); run([exe])
    run([cc,'--stats','-c',source,'-o',out/(name+'.stats.o')])
    check((out/(name+'.stats.o')).read_bytes()==(out/(name+'-O0.o')).read_bytes(), name+' telemetry equivalence')
    ir=out/(name+'.lowir'); canonical=out/(name+'.canonical.lowir')
    run([cc,'-c','--emit-lowir','--validate-lowir',source,'-o',ir])
    run([root/'dev/lowir',ir,'-o',canonical])
    run([out/'lowir-host',canonical,out/(name+'.adapted.o')])
    run(['g++',out/(name+'.adapted.o'),'-lm','-o',out/(name+'.adapted')]); run([out/(name+'.adapted')])
    for label in ['-O0','.adapted']:
        obj=out/(name+label+'.o')
        run(['readelf','-SWsrg',obj]); run(['readelf','--debug-dump=frames',obj]); run(['objdump','-dr',obj])
    run([root/'dev/lowir2native','--dump-machine-ir',out/(name+'.mir'),canonical])
symbols=run(['nm',out/'integrated-O0.o'])
check('dormant' not in symbols, 'unused member template has no emitted definition')
check('B11declaration' not in symbols, 'effective definition tags reach ELF')
print(len(rows),'commands,',len(properties),'properties passed')
