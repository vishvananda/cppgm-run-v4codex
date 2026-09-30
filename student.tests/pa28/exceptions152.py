#!/usr/bin/env python3
"""Explicit PA28 host-filter/cleanup controls; no assignment discovery changes."""
import json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa28-152/controls').resolve()
out.mkdir(parents=True,exist_ok=True)
records=[]
def run(args, ok=True):
    p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,timeout=30)
    records.append(dict(args=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
    (out/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
    assert (p.returncode == 0) == ok, records[-1]
    return p
for name in ['exceptions152','cleanup-reducer152']:
    obj=out/(name+'.o'); exe=out/name
    run(['dev/cppgm++','-c',root/'student.tests/pa28'/(name+'.cpp'),'-o',obj])
    run(['g++',obj,'-o',exe]);run([exe])
# Cleanup regions also obey the compiler's private runtime transfer rules.
run(['dev/cppgm++','--object-format=private',root/'student.tests/pa28/cleanup-reducer152.cpp','-o',out/'private'])
run([out/'private'])
inputs={
'empty': ('namespace std { typedef void (*unexpected_handler)(); unexpected_handler set_unexpected(unexpected_handler) throw(); } extern "C" void _Exit(int); int live; struct G { G(){++live;} ~G(){--live;} }; void u(){_Exit(live ? 3 : 0);} void f() throw() {G g; throw 1;} int main(){std::set_unexpected(u); f(); return 2;}',True),
'member-template': ('template<class T> struct S { static void f() throw(T) { throw T(3); } }; int main(){try {S<int>::f();} catch(int n){return n != 3;} return 2;}',True),
'conflict': ('void f() throw(int); void f() throw(double) {}',False),
'absent-spec': ('void f() throw(int); void f() {}',False),
'incomplete': ('struct S; void f() throw(S) {}',False),
'rvalue': ('void f() throw(int&&) {}',False),
}
for name,(source,valid) in inputs.items():
    path=out/(name+'.cpp');path.write_text(source);obj=out/(name+'.o');exe=out/name
    run(['dev/cppgm++','-c',path,'-o',obj],valid)
    if valid: run(['g++',obj,'-o',exe]);run([exe])
print('exception controls PASS:',len(records),'commands')
