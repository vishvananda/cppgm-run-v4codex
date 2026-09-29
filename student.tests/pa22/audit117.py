#!/usr/bin/env python3
"""Composition controls for the accumulated PA22 audit; explicit invocation only."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC = Path(sys.argv[1]).resolve()
WORK = Path(sys.argv[2]).resolve()
WORK.mkdir(parents=True, exist_ok=True)
base = '''struct A {int a;}; struct B {int b; int f()const{return b;}};
struct D:A,B {int d; int g()const{return d;}};
'''
sources = {}
for name, init in {
    'paren-copy': 'int(D::*p)()const=(q);',
    'paren-direct': 'int(D::*p)()const((q));',
    'paren-list': 'int(D::*p)()const{(q)};',
    'paren-write': 'int(D::*p)()const=nullptr; p=((q));',
    'plain-copy': 'int(D::*p)()const=q;',
}.items():
    sources[name] = base + '''int main(){D d;d.a=1;d.b=7;d.d=9;
int(B::*q)()const=&B::f;
''' + init + 'return (d.*p)()!=7;}\n'
for name, write in {
    'conditional-write': '(d.a?p:q)=v;',
    'comma-write': '(d.a,p)=v;',
    'assignment-write': '(p=q)=v;',
    'reference-write': 'auto& r=(d.a?p:q); r=v;',
    'address-write': 'auto r=&(d.a?p:q); *r=v;',
    'lambda-write': 'auto mutate=[&p,v](){p=v;}; mutate();',
}.items():
    sources[name] = base + '''int main(){D d;d.a=1;d.b=2;d.d=3;
int(B::*p)()const=&B::f;int(B::*q)()const=&B::f;
auto v=static_cast<int(B::*)()const>(&D::g);
''' + write + 'return (d.*p)()!=3;}\n'
sources['member-ranking'] = '''struct A{int a;int f()const{return a;}};struct B:A{};struct C:B{};
int pick(int B::*){return 1;} int pick(int C::*){return 2;}
int fun(int(B::*)()const){return 3;} int fun(int(C::*)()const){return 4;}
int main(){return pick(&A::a)!=1 || fun(&A::f)!=3;}
'''
sources['dependent-ranking'] = '''struct A{int a;};struct B:A{};struct C:B{};
int pick(int B::*){return 1;} int pick(int C::*){return 2;}
template<class T>auto apply(T p)->decltype(pick(p)){return pick(p);}
int main(){return apply(&A::a)!=1;}
'''
sources['dependent-proof'] = base + '''template<class T>int apply(T& d){
int(B::*q)()const=&B::f;int(T::*p)()const=(q);return (d.*p)();}
int main(){D d;d.a=1;d.b=7;return apply(d)!=7;}
'''
sources['member-result-ranking'] = '''struct A{int a;};struct B:A{int b;};struct C:B{};
struct X{typedef int A::*P;typedef int B::*Q;
operator P()const{return &A::a;}operator Q()const{return &B::b;}};
int main(){C c;c.a=1;c.b=2;int C::*p=X();return c.*p!=2;}
'''
sources['pointer-result-ranking'] = '''struct A{int a;};struct B:A{};struct C:B{};
B b; C c; struct X{operator B*()const{return &b;}operator C*()const{return &c;}};
int main(){b.a=1;c.a=2;A* p=X();return p->a!=1;}
'''
sources['reference-result-ranking'] = '''struct A{int a;};struct B:A{};struct C:B{};
B b; C c; struct X{operator B&()const{return b;}operator C&()const{return c;}};
int main(){b.a=1;c.a=2;A& p=X();return p.a!=1;}
'''
sources['sibling-user-truth'] = base + '''int calls;
struct Pad{int pad;};struct Truth{typedef int(B::*P)()const;
P p;operator P()const{++calls;return p;}};
struct Both:Pad,Truth{};
int main(){Both v;v.p=&B::f;int n=0;if(v)++n;if(!v)++n;
v.p=nullptr;if(v)++n;return n!=1 || calls!=3;}
'''
rows = []
for name, source in sources.items():
    src = WORK/(name+'.cpp'); src.write_text(source)
    ir = WORK/(name+'.lowir'); exe = WORK/name
    cmd = [str(CC),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)]
    compile = subprocess.run(cmd,capture_output=True,text=True)
    row = dict(name=name,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),
               compile_exit=compile.returncode,diagnostic=compile.stderr)
    if not compile.returncode:
        plain = ir.read_bytes()
        stats = subprocess.run(cmd+['--stats'],capture_output=True,text=True)
        row['telemetry'] = [json.loads(s) for s in stats.stderr.splitlines() if s.startswith('{')]
        row['stats_identical'] = stats.returncode == 0 and ir.read_bytes() == plain
        row['lowir_sha256'] = hashlib.sha256(plain).hexdigest()
        backend = subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
        row.update(backend_exit=backend.returncode,backend_diagnostic=backend.stderr)
        if not backend.returncode: row['runtime_exit'] = subprocess.run([str(exe)],timeout=20).returncode
    row['passed'] = row.get('runtime_exit') == 0 and row.get('stats_identical',False)
    rows.append(row)
print(json.dumps(dict(compiler=str(CC),compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),cases=rows),indent=2))
sys.exit(not all(r['passed'] for r in rows))
