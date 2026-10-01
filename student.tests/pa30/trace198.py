#!/usr/bin/env python3
"""Record source/typed-LowIR/native traces and before/after adapter behavior."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
entry = pathlib.Path(sys.argv[2]).resolve()
compiler = root/'dev/cppgm++'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
r = dict(compiler_sha256=sha(compiler), entry_sha256=sha(entry), commands=[], images={}, views={})
def save(): (out/'trace.json').write_text(json.dumps(r,indent=2)+'\n')
def run(args, reject=False):
    args=list(map(str,args)); p=subprocess.run(args,capture_output=True,text=True,timeout=45)
    r['commands'].append(dict(args=args,status=p.returncode,stdout=p.stdout,stderr=p.stderr,expected_rejection=reject))
    save(); assert (p.returncode!=0)==reject, r['commands'][-1]
    return p
src = root/'student.tests/pa30/source198/namespace-entry.cpp'
run([entry,'--emit-lowir',src,'-o',out/'entry.lowir'])
run([root/'dev/lowir2native','-o',out/'entry.native',out/'entry.lowir'],True)
for name in ['interactions','namespace-entry','optimization']:
    src=root/f'student.tests/pa30/source198/{name}.cpp'
    obj,exe,ir,mir = [out/(name+ext) for ext in ['.o','','.lowir','.mir']]
    run([compiler,'-O0','-c',src,'-o',obj]); plain=sha(obj)
    run([compiler,'-O0','-c','--stats',src,'-o',obj]); assert plain==sha(obj)
    run(['g++',obj,'-o',exe]);run([exe])
    run([compiler,'--emit-lowir','--validate-lowir','--stats',src,'-o',ir])
    run([root/'dev/lowir2native','--stats','--dump-machine-ir',mir,'-o',out/(name+'.native'),ir])
    run([out/(name+'.native')])
    r['images'][name]=dict(source_sha256=sha(src),object_sha256=sha(obj),executable_sha256=sha(exe),telemetry_identical=True)
    r['views'][name]=dict(lowir=ir.read_text(),mir=mir.read_text(),
        disassembly=run(['objdump','-dr',obj]).stdout,
        symbols=run(['readelf','-Ws',obj]).stdout,
        frames=run(['readelf','--debug-dump=frames',obj]).stdout)
    save()
run(['g++','-std=c++11','-I'+str(root/'dev/src'),root/'student.tests/pa30/token-size195.cpp','-o',out/'token-size'])
assert run([out/'token-size']).stdout.strip()=='40 40'
print(len(r['commands']), 'trace commands passed')
