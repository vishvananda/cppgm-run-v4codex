#!/usr/bin/env python3
"""Explicit PA28 naming/effect controls; host compilation is a test oracle."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
here = pathlib.Path(__file__).resolve().parent
out = pathlib.Path(sys.argv[1]).resolve()
out.mkdir(parents=True, exist_ok=True)
compiler = root/'dev/cppgm++'
checks = []
def run(args, expected=0):
    p = subprocess.run(list(map(str,args)), capture_output=True, timeout=60)
    checks.append(dict(command=list(map(str,args)), status=p.returncode,
                       stdout=p.stdout.decode(), stderr=p.stderr.decode()))
    assert (p.returncode == 0) == (expected == 0), checks[-1]
    return p.stdout.decode()
obj = out/'attributes.o'
run([compiler,'-c',here/'attributes151.cpp','-o',obj])
run(['g++','-std=c++11',here/'attributes151-host.cpp',obj,'-o',out/'attributes'])
run([out/'attributes'])
ir = out/'effects.lowir'
run([compiler,'--emit-lowir',here/'attributes151.cpp','-o',ir])
functions = [s for s in ir.read_text().splitlines() if s.startswith('function ')]
for name,effect in [('reader','readonly'),('strongest','readnone'),('identity','readnone'),('later','readnone'),('untouched',None)]:
    line = next(s for s in functions if s.startswith('function @'+name+'('))
    assert ('effects='+effect in line) if effect else ('effects=' not in line), line
run([compiler,'-c',here/'decay151.cpp','-o',out/'decay.o'])
run(['g++',out/'decay.o','-o',out/'decay'])
run([out/'decay'])
symbols = run(['nm',out/'decay.o'])
assert '_Z6resultIiEu7__decayIT_EPS0_' in symbols, symbols
for i,arg in enumerate(['1','"ok", 1','L"wide"','""','"bad-tag"','"x",']):
    source = out/('invalid'+str(i)+'.cpp')
    source.write_text('struct __attribute__((abi_tag('+arg+'))) C {}; int main(){}')
    run([compiler,'-c',source,'-o',out/'invalid.o'],1)
names = out/'unnamed.names'
run([root/'dev/abimangle','-o',names,here/'unnamed151.abi'])
assert names.read_text().splitlines() == [
    'Z5scopevEUt_', 'Z5scopevEUt0_', 'Z5scopevEUt1_B5alphaB4beta']
(out/'results.json').write_text(json.dumps(dict(
    binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),checks=checks,
    effects=functions,status='pass'),indent=2)+'\n')
print('naming/effects controls PASS:',len(checks),'commands')
