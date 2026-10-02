#!/usr/bin/env python3
"""PA30 implementation199 semantic, object and LowIR controls."""
import hashlib, json, pathlib, subprocess, sys, time
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
compiler = root / 'dev/cppgm++'
result = {'compiler_sha256': hashlib.sha256(compiler.read_bytes()).hexdigest(), 'commands': []}
def run(args, reject=False):
    args = list(map(str, args))
    start = time.perf_counter()
    p = subprocess.run(args, text=True, capture_output=True, timeout=45)
    result['commands'].append(dict(args=args, status=p.returncode,
        wall_s=time.perf_counter()-start, stdout=p.stdout, stderr=p.stderr,
        expected_rejection=reject))
    (out/'controls.json').write_text(json.dumps(result, indent=2)+'\n')
    assert (p.returncode != 0) == reject, result['commands'][-1]
objects=[p for p in sorted((root/'obj/dev').rglob('*.o')) if 'entry' not in p.parts and not p.name.startswith('test_runner')]
adapter=out/'ir-object'
run(['g++','-std=c++11','-O2','-I'+str(root/'dev/src'),root/'student.tests/pa29/ir-object161.cpp',*objects,'-o',adapter])
for group in [199]:
    for source in sorted((root/f'student.tests/pa30/source{group}').glob('*.cpp')):
        corrected = group == 197 and source.name == 'base-alias.reject.cpp'
        reject = '.reject.' in source.name and not corrected
        stem = str(group)+'-'+source.stem
        obj, exe, ir = out/(stem+'.o'), out/stem, out/(stem+'.lowir')
        run([compiler, '-O0', '-c', source, '-o', obj], reject)
        if reject or corrected:
            continue
        run(['g++', obj, '-o', exe])
        run([exe])
        run([compiler, '-c', '--emit-lowir', '--validate-lowir', source, '-o', ir])
        run([adapter, ir, out/(stem+'.roundtrip.o')])
        run(['g++', out/(stem+'.roundtrip.o'), '-o', out/(stem+'.roundtrip')])
        run([out/(stem+'.roundtrip')])
print(len(result['commands']), 'commands passed')
