#!/usr/bin/env python3
"""Whole-stage PA22 controls: physical identities and their semantic consumers."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
CC, WORK = [Path(x).resolve() for x in sys.argv[1:3]]
WORK.mkdir(parents=True, exist_ok=True)
cases = {
 'access-later-base': '''struct A{protected:int x;int f(){return x;}};
 struct B:A{public:using A::x;using A::f;};struct Pad{};struct D:Pad,B{};
 int main(){D d;d.x=7;return d.f()!=7;}''',
 'access-template': '''struct A{protected:int x;};struct B:A{public:using A::x;};
 struct Pad{};struct D:Pad,B{};template<class T>auto get(T& x)->decltype(x.x){return x.x;}
 int main(){D d;d.x=7;return get(d)!=7;}''',
 'protected-later-base': '''struct A{protected:int x;};struct Pad{};struct B:A{int read();};
 struct D:Pad,B{};int B::read(){D d;d.x=7;return d.x;}
 int main(){B b;return b.read()!=7;}''',
 'friend-protected-later-base': '''struct A{protected:int x;};struct B:A{friend int read();};
 struct Pad{};struct D:Pad,B{};int read(){D d;d.x=7;return d.x;}
 int main(){return read()!=7;}''',
 'protected-base-path': '''struct A{int x;};struct B:protected A{friend int read();};
 struct Pad{};struct D:Pad,B{};int read(){D d;A* a=&d;a->x=7;return a->x;}
 int main(){return read()!=7;}''',
 'static-public-path': '''struct A{static int x;};int A::x=7;
 struct B:private A{};struct C:A{};struct D:B,C{};int main(){return D::x!=7;}''',
 'static-public-path-reversed': '''struct A{static int x;};int A::x=7;
 struct B:private A{};struct C:A{};struct D:C,B{};int main(){return D::x!=7;}''',
 'reject-protected-base-object': '''struct A{protected:int x;};struct B:A{int get(A& a){return a.x;}};''',
 'reject-private-path': '''struct A{int x;};struct B:private A{};struct Pad{};
 struct D:Pad,B{};int main(){D d;return d.x;}''',
 'reject-private-using': '''struct A{protected:int x;};struct B:A{private:using A::x;};
 struct Pad{};struct D:Pad,B{};int main(){D d;return d.x;}''',
 'reject-repeated-object': '''struct A{int x;};struct B:A{};struct C:A{};struct D:B,C{};
 int main(){D d;return d.x;}''',
 'repeated-empty': '''struct E{};struct A:E{};struct B:E{};struct D:A,B{};
 int main(){D d;E* a=static_cast<A*>(&d);E* b=static_cast<B*>(&d);return a==b;}''',
 'nested-empty': '''struct E{};struct A:E{};struct B:E{};struct D:A,B{};
 struct C:E{};struct X:D,C{};int main(){X x;E* a=static_cast<A*>(&x);
 E* b=static_cast<B*>(&x);E* c=static_cast<C*>(&x);return a==b||a==c||b==c;}''',
 'empty-after-nonempty': '''struct E{};struct A:E{int x;};struct B:E{};struct D:A,B{};
 int main(){D d;d.x=7;return static_cast<E*>(static_cast<A*>(&d))==static_cast<E*>(static_cast<B*>(&d))||d.x!=7;}''',
 'nonempty-after-empty': '''struct E{};struct A:E{};struct B:E{int x;};struct D:A,B{};
 int main(){D d;d.x=7;return static_cast<E*>(static_cast<A*>(&d))==static_cast<E*>(static_cast<B*>(&d))||d.x!=7;}''',
 'second-empty-base-member': '''struct E{};struct F{};struct A:F,E{};struct D:A{E e;};
 int main(){D d;return static_cast<E*>(&d)==&d.e;}''',
 'empty-array-member': '''struct E{};struct D:E{E values[3];};int main(){D d;
 return static_cast<E*>(&d)==&d.values[0]||static_cast<E*>(&d)==&d.values[1]||&d.values[0]==&d.values[1];}''',
 'empty-nested-array': '''struct E{};struct Inner{E a[3];};struct D:E{Inner i;};
 int main(){D d;return static_cast<E*>(&d)==&d.i.a[0];}''',
 'nonempty-base-array': '''struct E{};struct A{E a[3];};struct B:E{};struct D:A,B{};
 int main(){D d;return static_cast<E*>(&d)==&d.a[0]||static_cast<E*>(&d)==&d.a[1]||static_cast<E*>(&d)==&d.a[2];}''',
 'array-of-repeated': '''struct E{};struct A:E{};struct B:E{};struct D:A,B{};
 int main(){D d[2];E* a=static_cast<A*>(&d[0]);E* b=static_cast<B*>(&d[0]);
 E* c=static_cast<A*>(&d[1]);E* e=static_cast<B*>(&d[1]);return a==b||a==c||a==e||b==c||b==e||c==e;}''',
 'constant-and-runtime': '''struct E{};struct A:E{};struct B:E{};struct D:A,B{};
 constexpr D d{};constexpr const E* a=static_cast<const A*>(&d);
 constexpr const E* b=static_cast<const B*>(&d);static_assert(a!=b,"distinct");
 int main(){return a==b;}''',
 'member-constant-repeated-empty': '''struct E{bool same(const E* p)const{return this==p;}};
 struct A:E{};struct B:E{};struct D:A,B{};typedef bool(D::*P)(const E*)const;
 constexpr P a=static_cast<bool(A::*)(const E*)const>(&E::same);
 constexpr P b=static_cast<bool(B::*)(const E*)const>(&E::same);
 static_assert(a!=b,"subobjects");int main(){D d;const E* ea=static_cast<A*>(&d);
 return !(d.*a)(ea)||(d.*b)(ea);}''',
 'empty-lifetime': '''int count;const void* addresses[4];struct E{E(){addresses[count++]=this;}
 ~E(){addresses[count++]=this;}};struct A:E{};struct B:E{};struct D:A,B{};
 int main(){{D d;if(count!=2||addresses[0]==addresses[1])return 1;}
 return count!=4||addresses[2]!=addresses[1]||addresses[3]!=addresses[0];}''',
 'demanded-template-trace': '''int trace;struct L{int l;L():l(3){trace=trace*10+1;}
 L(const L& x):l(x.l){trace=trace*10+3;}~L(){trace=trace*10+6;}};
 struct R{int r;R():r(7){trace=trace*10+2;}R(const R& x):r(x.r){trace=trace*10+4;}
 ~R(){trace=trace*10+5;}int read()const{return r;}};struct D:L,R{};
 template<class T,int(T::*F)()const>int call(const T& x){return (x.*F)();}
 int main(){{D d;if(trace!=12||call<R,&R::read>(d)!=7)return 1;
 {D copy=d;if(trace!=1234||copy.l!=3||copy.r!=7)return 2;}
 if(trace!=123456)return 3;}return trace!=12345656;}''',
 'large-array-no-expansion': '''struct E{};struct A{E values[1000000000];};struct B:E{};
 struct D:A,B{};int main(){return sizeof(D)<1000000001;}''',
}
# Force the summary cap without expanding object arrays or searching layout holes.
prefix = ''.join('struct E%d{};\n'%i for i in range(70))
bases = ','.join('E%d'%i for i in range(70))
cases['summary-budget'] = prefix+'struct A:'+bases+'''{};struct B:E69{};struct D:A,B{};
 int main(){D d;E69* a=static_cast<A*>(&d);E69* b=static_cast<B*>(&d);return a==b;}'''
rows=[]
for name,source in cases.items():
 src=WORK/(name+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 cmd=[str(CC),'--emit-lowir','-O0','--validate-lowir','-o',str(ir),str(src)]
 r=subprocess.run(cmd,capture_output=True,text=True,timeout=30)
 row=dict(name=name,source=source,source_sha256=hashlib.sha256(source.encode()).hexdigest(),compile_exit=r.returncode,diagnostic=r.stderr)
 if not r.returncode and not name.startswith('reject-'):
  raw=ir.read_bytes();stats=subprocess.run(cmd+['--stats'],capture_output=True,text=True,timeout=30)
  row.update(lowir_sha256=hashlib.sha256(raw).hexdigest(),stats_identical=stats.returncode==0 and ir.read_bytes()==raw,
             telemetry=[json.loads(x) for x in stats.stderr.splitlines() if x.startswith('{')])
  r=subprocess.run([str(ROOT/'dev/lowir2native-ref'),'-O0','-o',str(exe),str(ir)],capture_output=True,text=True,timeout=30)
  row.update(backend_exit=r.returncode,backend_diagnostic=r.stderr)
  if not r.returncode:row['runtime_exit']=subprocess.run([str(exe)],timeout=20).returncode
 row['passed']=bool(row['compile_exit']) if name.startswith('reject-') else row.get('runtime_exit')==0 and row.get('stats_identical',False)
 rows.append(row)
print(json.dumps(dict(compiler_sha256=hashlib.sha256(CC.read_bytes()).hexdigest(),cases=rows),indent=2))
sys.exit(not all(x['passed'] for x in rows))
