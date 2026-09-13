#!/usr/bin/env python3
"""Definition-time statement rules and dependent/concrete scope separation.

N3485 [stmt.select]/4, [stmt.switch]/2-4, [stmt.break]/1,
[stmt.cont]/1, [stmt.return]/2-3, [basic.scope.block]/2, [temp.res]/8.
PA14 explicitly requires unused supported bodies to be checked at definition.
"""
from pathlib import Path
import json, os, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
BINARY = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else ROOT/'dev/cppgm++'
WORK = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa14-final-audit/statements'
WORK.mkdir(parents=True, exist_ok=True)
BAD = {
    'break': 'break;', 'continue': 'continue;', 'case': 'case 1:;', 'default': 'default:;',
    'return-value': 'return 1;', 'return-pointer': 'int* p; return p;',
    'condition-class': 'if(C()) {}', 'condition-enum': 'if(E::one) {}',
    'switch-float': 'switch(1.0) {}', 'case-value': 'int x=1;switch(1){case x:break;}',
    'duplicate-case': 'switch(1){case 1:break;case 1:break;}',
    'duplicate-default': 'switch(1){default:break;default:break;}',
    'continue-switch': 'switch(1){case 1:continue;}',
    'nested-break': 'while(true){struct L{void g(){break;}};break;}',
    'nested-case': 'switch(1){struct L{void g(){case 1:;}};}',
    'do-scope': 'do int x=0;while(x);',
}
prefix = 'struct C{};enum class E{one};'
wrappers = {
    'function': lambda b: 'template<class T>void f(){'+b+'}',
    'member': lambda b: 'template<class T>struct A{void f(){'+b+'}};',
    'out-of-line': lambda b: 'template<class T>struct A{void f();};template<class U>void A<U>::f(){'+b+'}',
}
controls = [(kind+'-'+name, prefix+wrap(body)+'int main(){}', 1) for kind,wrap in wrappers.items() for name,body in BAD.items()]
controls += [
    ('missing-return','template<class T>int f(){return;}int main(){}',1),
    ('bad-reference-return','template<class T>int&& f(){int x;return x;}int main(){}',1),
    ('bad-fixed-conversion','template<class T>int* f(){return 2;}int main(){}',1),
    ('bad-class-return','struct C{};template<class T>C f(){return 2;}int main(){}',1),
    ('deleted-return-copy','struct C{C();C(const C&)=delete;};template<class T>C f(C c){return c;}int main(){}',1),
    ('fixed-base-cv','struct B{int g()const{return 3;}int* g(){return 0;}};template<class T>struct A{struct D:B{int f()const{return g();}};};int main(){A<int>::D d;return d.f()!=3;}',0),
    ('bad-fixed-base-cv','struct B{int g();};template<class T>struct A{struct D:B{int f()const{return g();}};};int main(){}',1),
    ('move-return','struct C{int n;C(int x):n(x){}C(C&& x):n(x.n){}C(const C&)=delete;};template<class T>C f(){C c(3);return c;}int main(){C c=f<int>();return c.n!=3;}',0),
    ('dependent','template<class T>T f(T x){if(x){return x;}return T();}int main(){return f<int>(3)!=3;}',0),
    ('nested-context','template<class T>int f(){int n=0;while(n<3){struct L{int g(){return 4;}};switch(n){case 0:++n;continue;default:break;}++n;}return n;}int main(){return f<int>()!=3;}',0),
    ('enum-cases','template<class T>int f(){enum E{a=1,b=2};switch(2){case a:return 1;case b:return 2;}return 0;}int main(){return f<int>()!=2;}',0),
    ('user-bool','struct C{explicit operator bool(){return true;}};template<class T>int f(){C c;if(c)return 3;return 0;}int main(){return f<int>()!=3;}',0),
    ('void-return','void g(){}template<class T>void f(){return g();}int main(){f<int>();}',0),
    ('fixed-return','struct C{operator int(){return 7;}};template<class T>int f(C c){return c;}int main(){C c;return f<int>(c)!=7;}',0),
    ('dependent-return','template<class T>T f(){return 3;}int main(){return f<int>()!=3;}',0),
    ('unused-dependent','template<class T>void f(){if(T()){}switch(T()){case T::v:break;}}int main(){}',0),
]
records = []
for name, source, expected in controls:
    path = WORK/(name+'.cpp'); path.write_text(source)
    ir = WORK/(name+'.lowir')
    p = subprocess.run([str(BINARY),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(path)],capture_output=True,text=True,timeout=30)
    records.append(dict(name=name,source=source,expected=expected,exit_code=p.returncode,diagnostic=p.stderr))
    (WORK/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
    assert p.returncode == expected, (name,expected,p.returncode,p.stderr)
    if not expected:
        exe = WORK/(name+'.exe')
        subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],check=True,capture_output=True,timeout=30)
        subprocess.run([str(exe)],check=True,timeout=30)
    print(name,'PASS',flush=True)
print(len(records),'statement checks PASS')
