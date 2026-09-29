#!/usr/bin/env python3
"""Explicit local member-value flow controls, with checked native execution."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK = [Path(p).resolve() for p in sys.argv[1:3]]
WORK.mkdir(parents=True, exist_ok=True)
prefix = '''struct Pad{int pad;};struct S{int x;int f()const{return x;}};
struct D:Pad,S{int z;int g()const{return z;}};
typedef int(S::*P)()const;struct Box{P p;};
'''
setup = '''int main(){D d;d.pad=1;d.x=3;d.z=7;
P back=static_cast<P>(&D::g); Box box;
'''
cases = {}
def add(name, body, expected=3, extra='', shape=None):
    cases[name] = (prefix+extra+setup+body+
                   '\nreturn (d.*box.p)()!='+str(expected)+';}\n', shape)
add('direct-field', 'box.p=&S::f;', shape='proven')
add('wrapped-field', '(box.p)=((&S::f));', shape='proven')
add('local-value', 'P p=&S::f;box.p=p;', shape='proven')
add('distinct-object', 'Box other;box.p=&S::f;other.p=back;', shape='proven')
add('adjusted-write', 'box.p=back;', 7, shape='generic')
add('wrapped-adjusted-write', 'box.p=((back));', 7, shape='generic')
add('alias-write', 'P* alias=&box.p;box.p=&S::f;*alias=back;', 7, shape='generic')
add('reference-write', 'P& alias=box.p;box.p=&S::f;alias=back;', 7, shape='generic')
add('call-write', 'box.p=&S::f;change(box,back);', 7,
    'void change(Box& b,P p){b.p=p;}\n', 'generic')
add('whole-object-write', 'Box other;other.p=back;box.p=&S::f;box=other;', 7, shape='generic')
add('branch-write', 'box.p=&S::f;if(d.x)box.p=back;', 7, shape='generic')
add('conditional-write', 'Box other;box.p=&S::f;(d.x?box.p:other.p)=back;', 7, shape='generic')
add('comma-write', 'box.p=&S::f;(d.x,box.p)=back;', 7, shape='generic')
add('assignment-write', 'Box other;other.p=&S::f;box.p=&S::f;(box.p=other.p)=back;', 7, shape='generic')
add('destructor-write', '(Guard(box,back),box.p=&S::f);', 7,
    'struct Guard{Box& b;P p;Guard(Box& x,P y):b(x),p(y){}~Guard(){b.p=p;}};\n', 'generic')
add('scope-destructor', 'box.p=&S::f;{Guard g(box,back);}', 7,
    'struct Guard{Box& b;P p;Guard(Box& x,P y):b(x),p(y){}~Guard(){b.p=p;}};\n', 'generic')
add('goto-write', 'box.p=&S::f;goto use;box.p=back;use:box.p=back;', 7, shape='generic')
cases['nested-field'] = (prefix+'''struct Nest{Box a;Box b;};int main(){D d;
d.x=3;d.z=7;Nest n;n.a.p=&S::f;n.b.p=static_cast<P>(&D::g);
return (d.*n.a.p)()!=3 || (d.*n.b.p)()!=7;}\n''', None)
cases['union-alias'] = (prefix+'''union U{P a;P b;};int main(){D d;d.x=3;d.z=7;
U u;u.a=&S::f;u.b=static_cast<P>(&D::g);return (d.*u.b)()!=7;}\n''', 'generic')
cases['volatile-field'] = (prefix+'''struct V{P volatile p;};int main(){D d;d.x=3;
V v;v.p=&S::f;return (d.*v.p)()!=3;}\n''', 'generic')
cases['loop-definitions'] = (prefix+'''int main(){D d;d.x=3;Box box;int sum=0;
for(int i=0;i<10;++i){box.p=&S::f;sum+=(d.*box.p)();}return sum!=30;}\n''', 'proven')
cases['loop-carried'] = (prefix+'''int main(){D d;d.x=3;d.z=7;Box box;
box.p=&S::f;int sum=0;for(int i=0;i<2;++i){sum+=(d.*box.p)();
box.p=static_cast<P>(&D::g);}return sum!=10;}\n''', 'generic')
cases['captured-before-arguments'] = ('''struct Pad{int pad;};
struct S{int x;int f(int v)const{return x+v;}};
struct D:Pad,S{int z;int g(int v)const{return z+v;}};
typedef int(S::*P)(int)const;struct Box{P p;};
int change(Box& b,P p){b.p=p;return 1;}
int main(){D d;d.x=3;d.z=7;Box b;b.p=&S::f;
int a=(d.*b.p)(change(b,static_cast<P>(&D::g)));
return a!=4 || (d.*b.p)(1)!=8;}\n''', None)
cases['nested-template'] = ((ROOT/'pa22/tests/general/300-repeated-nested-owner-member-template-address.t').read_text(), 'proven')
cases['repeated-base-fields'] = (prefix+'''struct A:Box{};struct B:Box{};struct Both:A,B{};
int main(){D d;d.x=3;d.z=7;Both b;b.A::p=static_cast<P>(&D::g);b.B::p=&S::f;
return (d.*b.A::p)()!=7;}\n''', 'generic')
cases['equivalent-field-paths'] = (prefix+'''struct A:Box{};
int main(){D d;d.x=3;d.z=7;A b;b.p=&S::f;b.Box::p=static_cast<P>(&D::g);
return (d.*b.p)()!=7;}\n''', 'generic')
cases['nested-field-overwrite'] = (prefix+'''struct Inner:Box{};struct Outer{Inner child;};
int main(){D d;d.x=3;d.z=7;Outer b;b.child.p=&S::f;
b.child.Box::p=static_cast<P>(&D::g);return (d.*b.child.p)()!=7;}\n''', 'generic')
cases['qualified-field-runtime'] = ('''struct Root{int n;};struct Left:Root{};
struct Right:Root{};struct D:Left,Right{
int get()const{return Left::n+Right::n;}};
int main(){D d;d.Left::n=3;d.Right::n=7;int* p=&d.Right::n;
return d.get()!=10 || *p!=7 || (&d)->Left::n!=3;}\n''', None)
cases['qualified-field-constant'] = ('''struct Root{int n;constexpr Root(int x):n(x){}};
struct Left:Root{constexpr Left():Root(3){}};
struct Right:Root{constexpr Right():Root(7){}};
struct D:Left,Right{constexpr D():Left(),Right(){}
constexpr int get()const{return Left::n+Right::n;}};
constexpr D d;static_assert(d.Left::n==3,"left");
static_assert(d.Right::n==7,"right");static_assert(d.get()==10,"implicit");
int main(){return d.get()!=10;}\n''', None)
cases['qualified-field-template'] = ('''struct Root{int n;};struct Left:Root{};
struct Right:Root{};struct D:Left,Right{};
template<class T>int get(T& x){return x.Left::n+x.Right::n;}
template<class T>auto right(T& x)->decltype(x.Right::n){return x.Right::n;}
int main(){D d;d.Left::n=3;d.Right::n=7;return get(d)!=10 || right(d)!=7;}\n''', None)
add('arrow-effect', 'Arrow a(box,back);box.p=&S::f;(a->n,0);', 7,
    '''struct Target{int n;};Target target;
struct Arrow{Box& b;P p;Arrow(Box& x,P y):b(x),p(y){}
Target* operator->(){b.p=p;return &target;}};\n''', 'generic')
add('bounded-function', 'box.p=&S::f;\n'+'d.x=3;\n'*4200, shape='generic')
add('bounded-depth', 'box.p=&S::f;\n'+'('*70+'d.x'+')'*70+';', shape='generic')
cases['reject-ambiguous-qualifier'] = ('''struct Root{int n;};struct A:Root{};
struct B:Root{};struct D:A,B{};int main(){D d;return d.Root::n;}\n''', 'reject')
cases['reject-private-qualifier'] = ('''struct Root{int n;};struct A:Root{};
struct B:Root{};struct D:private A,B{};int main(){D d;return d.A::n;}\n''', 'reject')
cases['arrow-call-effect'] = (prefix+'''struct Target{int take(int n){return n;}};Target target;
struct Arrow{Box& b;P p;Arrow(Box& x,P y):b(x),p(y){}
Target* operator->(){b.p=p;return &target;}};
'''+setup+'''Arrow a(box,back);box.p=&S::f;
return a->take((d.*box.p)())!=7;}\n''', 'generic')
rows = []
for name, (source, shape) in cases.items():
    src = WORK/(name+'.cpp'); src.write_text(source)
    ir = WORK/(name+'.lowir'); exe = WORK/name
    command = [str(CC),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)]
    result = subprocess.run(command,capture_output=True,text=True)
    row = dict(name=name,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),
               compile_exit=result.returncode,diagnostic=result.stderr,expected_shape=shape)
    if not result.returncode and shape != 'reject':
        text = ir.read_text(); raw = ir.read_bytes()
        stats = subprocess.run(command+['--stats'],capture_output=True,text=True)
        row['telemetry'] = [json.loads(s) for s in stats.stderr.splitlines() if s.startswith('{')]
        row['stats_identical'] = stats.returncode == 0 and ir.read_bytes() == raw
        row['adjustment_extracts'] = text.count('binary shr i128')
        row['shape_passed'] = shape is None or (row['adjustment_extracts']==0 if shape=='proven' else row['adjustment_extracts']>0)
        row['lowir_sha256'] = hashlib.sha256(raw).hexdigest()
        backend = subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True)
        row.update(backend_exit=backend.returncode,backend_diagnostic=backend.stderr)
        if not backend.returncode: row['runtime_exit'] = subprocess.run([str(exe)],timeout=20).returncode
    row['passed'] = bool(result.returncode) if shape == 'reject' else (
        row.get('runtime_exit') == 0 and row.get('stats_identical',False) and row.get('shape_passed',False))
    rows.append(row)
print(json.dumps(dict(compiler=str(CC),compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),cases=rows),indent=2))
sys.exit(not all(r['passed'] for r in rows))
