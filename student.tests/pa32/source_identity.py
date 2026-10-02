#!/usr/bin/env python3
"""Declaration displays, complete objects, lifecycle entries and alias roots."""
import pathlib, re, subprocess, tempfile
root = pathlib.Path(__file__).resolve().parents[2]
def run(*args):
    p = subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=120)
    assert p.returncode == 0, (args,p.returncode,p.stdout,p.stderr)
    return p
sources = {
'linkage': r'''
extern "C" { struct C {
 int n; C(int x):n(x){} ~C(){}
 int f(int x){return n+x;} int f(long x){return n+x+1;}
}; }
template<int N> struct T {int n;T(int x):n(x){} ~T(){n+=N;} int f(){return n+N;}};
extern "C" int entry(int n){T<3> a(n);T<5> b(n);return a.f()+b.f();}
int main(){C c(4);return entry(7)!=22 || c.f(2)!=6 || c.f(2L)!=7;}
''',
'layouts': r'''
struct V { int value; V(int n):value(n){} ~V(){} };
struct L: virtual V { L(int n):V(n){} ~L(){} };
struct R: virtual V { R(int n):V(n){} ~R(){} };
struct Padding { long a,b,c; Padding():a(0),b(0),c(0){} };
struct D: Padding,L,R { D(int n):V(n),L(n+1),R(n+2){} ~D(){} };
struct Container { D member; Container(int n):member(n){} };
__attribute__((noinline)) int dynamic(L* p) { return p->value; }
__attribute__((noinline)) int ref(L& p) { return p.value; }
__attribute__((noinline)) int copied(D d) { return d.value; }
D make(int n) { D result(n); return result; }
int main(int argc,char**) {
  D d(argc+20); Container box(argc+30); D returned=make(argc+40);
  V& chain=static_cast<L&>(d);
  return d.value!=21 || dynamic(&d)!=21 || ref(d)!=21 || chain.value!=21 ||
    static_cast<R&>(d).value!=21 || box.member.value!=31 ||
    copied(d)!=21 || returned.value!=41;
}
''',
'effects': r'''
int seen;
struct V { int value; V(int n):value(n){} virtual int read(){return value;} virtual ~V(){seen+=read();} };
struct L: virtual V {
 L(int n):V(n){} int read(){return value+1;}
 ~L(){seen+=read();}
};
struct D: L { D(int n):V(n),L(n){} int read(){return value+2;} ~D(){seen+=read();} };
int main(){ { D d(7); } return seen!=24; }
''',
'names': r'''
namespace A {
int take(int x){return x;} int take(long x){return x+1;}
struct Box {
 long x; Box(long n):x(n){}
 __attribute__((noinline)) Box(const Box& b):x(b.x){}
 __attribute__((noinline)) Box(Box&& b):x(b.x){b.x=0;}
};
}
namespace A__Box { int Box(int x){return x+2;} }
template<int N> int f(int x) {
 auto op=[](int y) __attribute__((noinline)) {return y+N;}; return op(x);
}
int main(){A::Box a(9),b(a),c(static_cast<A::Box&&>(b));
 return c.x!=9 || b.x!=0 || A::take(2)!=2 || A::take(2L)!=3 ||
 A__Box::Box(4)!=6 || f<3>(4)!=7 || f<5>(4)!=9;}
''',
'termination': r'''
extern "C" int puts(const char*);
struct P { __attribute__((noinline)) ~P(){puts("cleanup");} };
int main(){P p;}
'''
}
with tempfile.TemporaryDirectory(prefix='pa32-identities-') as tmp:
    d=pathlib.Path(tmp)
    for name,source in sources.items():
        src=d/(name+'.cpp');src.write_text(source)
        for level in range(4):
            flag=f'-O{level}';ir=d/'out.lowir';obj=d/'direct.o';replay=d/'replay.o'
            run(root/'dev/cppgm++','--emit-lowir','-g0','-O0',src,'-o',ir)
            run(root/'dev/cppgm++','-c','-g0',flag,src,'-o',obj)
            run(root/'dev/cppgm++','-c','-g0',flag,ir,'-o',replay)
            assert obj.read_bytes()==replay.read_bytes(),(name,level,'object replay')
            run(root/'dev/cppgm++',flag,src,'-o',d/'direct')
            run(root/'dev/cppgm++',flag,ir,'-o',d/'replayed')
            assert run(d/'direct').stdout==run(d/'replayed').stdout
        for level in [1,3]:
            run(root/'dev/cppgm++','--emit-lowir','-gline-tables-only','-O0',src,'-o',ir)
            run(root/'dev/cppgm++','-c','-gline-tables-only',f'-O{level}',src,'-o',obj)
            run(root/'dev/cppgm++','-c','-gline-tables-only',f'-O{level}',ir,'-o',replay)
            assert obj.read_bytes()==replay.read_bytes(),(name,level,'debug replay')
        run(root/'dev/cppgm++','--emit-lowir','-g0','-O1',src,'-o',ir)
        text=ir.read_text()
        names=re.findall(r'^(?:declare )?function (@\w+)\(',text,re.M)
        assert len(names)==len(set(names)),(name,'collision')
        if name=='names':
            assert 'function @A__Box__Box__ov2(' in text and 'function @A__Box__Box__ov3(' in text
            assert re.search(r'function @__lambda_f_t\d+_\d+__operator__',text)
        if name=='termination':
            helper=text.split('function @cppgm_call_terminate(')[1].split('\n}')[0]
            assert 'no_inline=yes' in helper and 'call void' in helper
    src=d/'aliases.lowir'
    src.write_text('''
function @dead() -> i32 [binding=weak, inline_hint=yes] { block ^e: return i32 1 }
alias object weak_dead = @dead
function @live() -> i32 [binding=weak, inline_hint=yes, no_inline=yes] { block ^e: return i32 2 }
alias object weak_live = @live
function @rooted() -> i32 [binding=weak, inline_hint=yes, object_root=yes] { block ^e: return i32 3 }
alias object weak_rooted = @rooted
function @ordinary() -> i32 [binding=weak] { block ^e: return i32 4 }
alias object explicit_weak = @ordinary
function @exported() -> i32 [binding=internal, inline_hint=yes] { block ^e: return i32 5 }
alias object explicit_export = @exported
function @addressed() -> i32 [binding=weak, inline_hint=yes] { block ^e: return i32 6 }
alias object weak_addressed = @addressed
global @callback : ptr = addr @addressed
function @main() -> i32 [role=entry] { block ^e:
 %r = call i32 @live() %v = binary sub i32 %r, 2 return i32 %v }
''')
    for level in range(4):
        run(root/'dev/lowiropt',f'-O{level}','-o',ir,src)
        text=ir.read_text()
        assert ('alias object weak_dead' in text)==(level==0)
        for alias in ['weak_live','weak_rooted','explicit_weak','explicit_export','weak_addressed']:
            assert 'alias object '+alias in text,alias
        run(root/'dev/lowir','-o',d/'validated',ir)
        run(root/'dev/cppgm++','-O0',ir,'-o',d/'aliases')
        run(d/'aliases')
print('source identities: PASS (five source groups x four levels, normal/debug replay, aliases and roots)')
