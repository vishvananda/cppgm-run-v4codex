#!/usr/bin/env python3
"""Repeated public body-fact requests after failures in each owning interval."""
from pathlib import Path
import json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
WORK = Path(sys.argv[1]).resolve()
OBJECTS = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else ROOT/'obj/dev'
MODE = sys.argv[3] if len(sys.argv) > 3 else 'current'
WORK.mkdir(parents=True, exist_ok=True)
CASES = {
    'direct-expression': 'int target(){return missing;}',
    'direct-loop': 'int target(){while(1){return missing;}}',
    'parameter-cleanup': 'class C{~C();};int target(C c){return 0;}',
    'direct-jump': 'int target(){goto done;int x=1;done:return 0;}',
    'template-expression': 'template<class T>int target(){return T::missing;}int main(){return target<int>();}',
    'member-expression': 'template<class T>struct C{int target(){return T::missing;}};int main(){C<int> c;return c.target();}',
    'template-lifetime': 'struct C{C();~C();};template<class T>int target(){goto done;T x;done:return 0;}int main(){return target<C>();}',
    'member-lifetime': 'struct C{C();~C();};template<class T>struct D{int target(){goto done;T x;done:return 0;}};int main(){D<C> d;return d.target();}',
    'valid': 'int target(){int x=1;while(x<3)++x;return x;}int main(){return target()!=3;}',
}
def run(command):
    p = subprocess.run(list(map(str, command)), cwd=ROOT, capture_output=True, text=True, timeout=180)
    assert p.returncode == 0, (command, p.returncode, p.stdout, p.stderr)
    return p.stdout
names = run(['make','-s','-C','dev','--eval=probe-objects:\n\t@echo $(FRONTEND_OBJ_BASENAMES_cppgm++)','probe-objects']).split()
flags = ['-DENTRY_PUBLICATION'] if MODE == 'entry' else []
if MODE == 'sanitized': flags += ['-fsanitize=address,undefined','-fno-pie','-no-pie']
exe = WORK/'probe'
run(['g++','-std=c++11','-O2',*flags,'-I'+str(ROOT/'dev/src'),ROOT/'student.tests/pa14/body-publication.cc',*[OBJECTS/(n+'.o') for n in names],'-o',exe])
records = []
for name, source in CASES.items():
    path = WORK/(name+'.cpp'); path.write_text(source)
    output = run([exe,path,'valid' if name == 'valid' else 'invalid'])
    records.append(dict(name=name,source=source,output=output,exit_code=0))
    print(name, output.strip(), flush=True)
(WORK/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
