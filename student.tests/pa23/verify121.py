#!/usr/bin/env python3
"""Nonvirtual polymorphic view, conversion, RTTI and rejection controls."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:3]]
WORK.mkdir(parents=True,exist_ok=True)
cases={}
def add(name,source,reject=False):cases[name]=(source,reject)
base='struct A{virtual int a(){return 1;}};struct B{int value;virtual int b(){return value;}};'
add('secondary-override',base+'struct D:A,B{int b(){return value+2;}};int run(B& b){return b.b();}int main(){D d;d.value=5;return run(d)!=7;}')
add('secondary-inherited',base+'struct D:A,B{};int run(B& b){return b.b();}int main(){D d;d.value=7;return run(d)!=7;}')
add('deep-overrides',base+'struct C:A,B{};struct D:C{int b()override{return value+2;}};int run(B* b){return b->b();}int main(){D d;d.value=5;return run(&d)!=7;}')
add('later-primary','struct Data{int n;};struct A{virtual int f(){return 1;}};struct D:Data,A{int f(){return n;}};int run(A& a){return a.f();}int main(){D d;d.n=9;return run(d)!=9||sizeof(D)!=16;}')
add('primary-ancestor-slots','struct Data{int n;};struct A{virtual int f(){return 1;}};struct B:A{virtual int g(){return 2;}};struct D:Data,B{int f(){return n;}int g(){return n+1;}};int run(B& b){return b.f()+b.g();}int main(){D d;d.n=9;return run(d)!=19;}')
add('unrelated-roots','struct A{virtual int f(){return 3;}};struct B{virtual int f(){return 5;}};struct D:A,B{};int main(){D d;A&a=d;B&b=d;return a.f()!=3||b.f()!=5;}')
add('repeated-roots','struct A{int n;virtual int f(){return n;}};struct B:A{};struct C:A{};struct D:B,C{};int main(){D d;B&b=d;C&c=d;b.n=3;c.n=5;A&x=b;A&y=c;return x.f()!=3||y.f()!=5;}')
add('all-roots-override','struct A{virtual int f(){return 3;}};struct B{virtual int f(){return 5;}};struct D:A,B{int n;int f(){return n;}};int main(){D d;d.n=7;A&a=d;B&b=d;return a.f()!=7||b.f()!=7;}')
add('triple-view','struct A{virtual int f(){return 1;}};struct B{virtual int g(){return 2;}};struct C{virtual int h(){return 3;}};struct D:A,B,C{int f(){return 4;}int g(){return 5;}int h(){return 6;}};int main(){D d;A&a=d;B&b=d;C&c=d;return a.f()+b.g()+c.h()!=15;}')
add('forward-arguments','struct A{virtual int a(){return 1;}};struct B{virtual double f(int i,double d)=0;};struct D:A,B{int n;double f(int i,double d){return n+i+d;}};int main(){D d;d.n=3;B&b=d;return b.f(4,0.5)!=7.5;}')
add('value-return','struct S{int n;};struct A{virtual int a(){return 1;}};struct B{virtual S f()=0;};struct D:A,B{S f(){S s={7};return s;}};int main(){D d;B&b=d;return b.f().n!=7;}')
add('indirect-return','struct S{int n;~S(){}};struct A{virtual int a(){return 1;}};struct B{virtual S f()=0;};struct D:A,B{S f(){S s;s.n=7;return s;}};int main(){D d;B&b=d;return b.f().n!=7;}')
add('template-views','template<int I>struct A{virtual int f(){return I;}};struct D:A<3>,A<5>{};int main(){D d;A<3>&a=d;A<5>&b=d;return a.f()!=3||b.f()!=5;}')
add('static-downcast',base+'struct D:A,B{int n;};D* back(B*p){return static_cast<D*>(p);}int main(){D d;d.n=7;B*b=&d;return back(b)!=&d||back(0)!=0||static_cast<D&>(*b).n!=7;}')
add('global-views',base+'struct D:A,B{int b(){return value+2;}};D d;int main(){d.value=5;B&b=d;return b.b()!=7;}')
rtti='namespace std{struct type_info{bool operator==(const type_info&)const;bool operator!=(const type_info&)const;};}'
add('crosscast-null-and-miss',base+'struct D:A,B{};B* cross(A*a){return dynamic_cast<B*>(a);}int main(){D d;A a;return cross(&d)!=static_cast<B*>(&d)||cross(&a)!=0||cross(0)!=0;}')
add('typeid-secondary',rtti+base+'struct D:A,B{};bool run(B&b){return typeid(b)==typeid(D);}int main(){D d;return !run(d);}')
add('rtti-top',base+'struct D:A,B{};int main(){D d;B*b=&d;return dynamic_cast<void*>(b)!=static_cast<void*>(&d);}')
add('secondary-covariance','struct A{virtual int a(){return 1;}};struct B{int n;virtual B* self(){return this;}};struct D:A,B{D* self(){return this;}};int main(){D d;B&b=d;return b.self()!=static_cast<B*>(&d);}')
add('secondary-covariance-null','struct A{virtual int a(){return 1;}};struct B{virtual B* self(){return this;}};struct D:A,B{D* self(){return 0;}};int main(){D d;B&b=d;return b.self()!=0;}')
add('primary-covariance','struct Pad{int n;};struct R{int r;};struct Result:Pad,R{};struct A{virtual R* get()=0;};struct B:A{Result value;Result* get(){return &value;}};int main(){B b;A&a=b;B&d=b;return a.get()!=static_cast<R*>(&b.value)||d.get()!=&b.value;}')
add('covariance-reference','struct Pad{int n;};struct R{int r;};struct Result:Pad,R{};struct A{virtual int f(){return 0;}};struct B{virtual R& get()=0;};struct D:A,B{Result value;Result& get(){return value;}};int main(){D d;B&b=d;return &b.get()!=static_cast<R*>(&d.value);}')
add('later-final-overrider','struct A{virtual int a(){return 1;}};struct B{int n;virtual int b(){return n;}};struct C:A,B{int b(){return n+1;}};struct P{virtual int p(){return 0;}};struct D:P,C{};int main(){D d;d.n=6;C&c=d;B&b=d;return c.b()!=7||b.b()!=7;}')
add('crosscast-private-base','struct A{virtual int f(){return 1;}};struct B:A{};struct C:private A{};struct D:B,C{};C* cross(A*a){return dynamic_cast<C*>(a);}int main(){D d;B&b=d;return cross(&b)!=static_cast<C*>(&d);}')
add('repeated-source-downcast','struct A{virtual int f(){return 1;}};struct B:A{};struct C:A{};struct D:B,C{};D* back(A*a){return dynamic_cast<D*>(a);}int main(){D d;B&b=d;C&c=d;return back(&b)!=&d||back(&c)!=&d;}')
add('secondary-pure-reject','struct A{virtual int a(){return 1;}};struct B{virtual int b()=0;};struct D:A,B{};int main(){D d;}',True)
add('secondary-final-reject','struct A{virtual int a(){return 1;}};struct B{virtual int b()final{return 1;}};struct D:A,B{int b(){return 2;}};',True)
add('secondary-exception-reject','struct A{virtual int a(){return 1;}};struct B{virtual int b()noexcept{return 1;}};struct D:A,B{int b(){return 2;}};',True)
add('secondary-static-reject','struct A{virtual int a(){return 1;}};struct B{virtual int b(){return 1;}};struct D:A,B{static int b(){return 2;}};',True)
rows=[]
for name,(source,reject) in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe');obj=src.with_suffix('.o')
 command=[str(CC),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)]
 p=subprocess.run(command,capture_output=True,text=True)
 row=dict(name=name,source=source,reject=reject,compile_exit=p.returncode,diagnostic=p.stderr)
 if not p.returncode and not reject:
  raw=ir.read_bytes();stats=subprocess.run(command+['--stats'],capture_output=True,text=True)
  row.update(lowir_sha256=hashlib.sha256(raw).hexdigest(),stats_identical=stats.returncode==0 and raw==ir.read_bytes(),telemetry=[json.loads(x) for x in stats.stderr.splitlines() if x.startswith('{')])
  p=subprocess.run([str(ROOT/'dev/cppgm++-ref'),'-c','-O0','-o',str(obj),str(ir)],capture_output=True,text=True)
  row.update(backend_exit=p.returncode,backend_diagnostic=p.stderr)
  if not p.returncode:
   p=subprocess.run(['g++','-no-pie',str(obj),'-o',str(exe)],capture_output=True,text=True)
   row.update(link_exit=p.returncode,link_diagnostic=p.stderr)
   if not p.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 row['passed']=row['compile_exit']!=0 if reject else row.get('runtime_exit')==0 and row.get('stats_identical',False)
 rows.append(row)
print(json.dumps(dict(compiler=str(CC),compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),cases=rows),indent=2))
sys.exit(not all(r['passed'] for r in rows))
