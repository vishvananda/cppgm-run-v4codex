#!/usr/bin/env python3
"""Run PA30 injected-type reducers, boundary controls and direct/adapter checks explicitly."""
import hashlib, json, pathlib, subprocess, sys, time
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
result={'compiler':str(compiler),'sha256':hashlib.sha256(compiler.read_bytes()).hexdigest(),'commands':[]}
def run(args, reject=False):
    start=time.perf_counter(); p=subprocess.run(list(map(str,args)),text=True,capture_output=True,timeout=45)
    row={'args':list(map(str,args)),'status':p.returncode,'wall_s':time.perf_counter()-start,'stdout':p.stdout,'stderr':p.stderr,'expected_rejection':reject}
    result['commands'].append(row)
    (out/'controls.json').write_text(json.dumps(result,indent=2)+'\n')
    assert (p.returncode!=0) if reject else (p.returncode==0),row
    return p
for source in sorted((root/'student.tests/pa30/source196').glob('*.cpp')):
    reject='.reject.' in source.name
    obj=out/(source.stem+'.o')
    run([compiler,'-c',source,'-o',obj],reject)
    if reject: continue
    exe=out/source.stem
    run(['g++',obj,'-o',exe]); run([exe])
    ir=out/(source.stem+'.lowir')
    run([compiler,'--emit-lowir',source,'-o',ir])
    # The explicit LowIR adapter consumes the same typed representation.
    run([root/'dev/lowir2native','-o',out/(source.stem+'.native'),ir])
    run([out/(source.stem+'.native')])
print(len(result['commands']),'commands passed')
