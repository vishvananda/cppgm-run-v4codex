#!/usr/bin/env python3
"""Explicit ABI/name controls; host compilation is comparison/linking only."""
import hashlib, json, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
compiler = pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
records=[]
def run(args, okay=0):
    p=subprocess.run(list(map(str,args)),capture_output=True,timeout=60)
    records.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
    assert (p.returncode==0)==(okay==0), records[-1]
    return p.stdout.decode()
def source(name,text):
    p=out/(name+'.cpp');p.write_text(text);return p
# Anonymous types acquire the first typedef denoting the type, even if a
# pointer typedef comes first; aliases after the defining declaration don't
# replace that linkage name. Class scopes use the same identity as arguments.
s=source('typedefs','''typedef struct { int x; int read() { return x; } } *P, First, Second;
typedef union { int i; long l; } Union;
typedef enum { Red, Blue } Color;
int probe(First* p, Union* u, Color c) { return p->read()+u->i+c; }
''')
a=out/'typedefs.o';h=out/'host.o'
run([compiler,'-c',s,'-o',a]);run(['g++','-std=c++11','-c',s,'-o',h])
for obj in (a,h):
    symbols=run(['nm',obj]);assert '_Z5probeP5FirstP5Union5Color' in symbols, symbols
main=source('typedef-main','''typedef struct { int x; int read() { return x; } } First;
typedef union { int i; long l; } Union; typedef enum { Red, Blue } Color;
int probe(First*,Union*,Color);
int main() { First a={4};Union b;b.i=7;return probe(&a,&b,Blue)-12; }
''')
run(['g++','-std=c++11',main,a,'-o',out/'typedef-main']);run([out/'typedef-main'])
# Namespace C functions retain language linkage, static C functions remain
# local, and their addresses must point to the correct body.
s=source('c-linkage','''namespace { extern "C" int host_add(int); }
extern "C" { static int local(int x) { return x+1; } }
int main() { int (*p)(int)=local;return host_add(p(5))-9; }
''')
h=source('c-host','extern "C" int host_add(int x) { return x+3; }\n')
run([compiler,'-c',s,'-o',out/'c.o']);run(['g++',out/'c.o',h,'-o',out/'c']);run([out/'c'])
s=source('bad-c','extern "C" int f(); static int f(){return 0;}\n')
run([compiler,'-c',s,'-o',out/'bad.o'],okay=1)
# Internal arguments propagate through packs, pointers, function types and
# enclosing class templates. Cross-TU function-local storage cannot coalesce.
header='''namespace { struct Tag {}; }
template<class... Ts> int* slot() { static int x;return &x; }
template<class T> struct Box { static int* get() {static int x;return &x;} };
'''
a=source('local-a',header+'int* a(){return slot<Tag,Tag*>();} int* b(){return Box<Tag>::get();}\n')
b=source('local-b',header+'int* a(); int* b();int main(){return a()==slot<Tag,Tag*>() || b()==Box<Tag>::get();}\n')
for level in ('-O0','-O2'):
    for name,s in [('a',a),('b',b)]:run([compiler,level,'-c',s,'-o',out/(name+'.o')])
    run(['g++',out/'a.o',out/'b.o','-o',out/'locals']);run([out/'locals'])
# Exact ABI standard substitutions and near misses: only the prescribed
# namespace, character type and argument identities receive abbreviations.
s=source('standards','''namespace std {
template<class T> struct allocator {};
template<class T> struct char_traits {};
template<class C,class T> struct basic_ostream {};
template<class C,class T,class A> struct basic_string {};
}
namespace other {template<class T> struct allocator {};}
struct Traits {};
void a(std::allocator<int>*) {}
void b(std::basic_ostream<char,std::char_traits<char> >*) {}
void c(std::basic_ostream<char,Traits>*) {}
void d(std::basic_ostream<wchar_t,std::char_traits<wchar_t> >*) {}
void e(std::basic_string<char,std::char_traits<char>,std::allocator<char> >*) {}
void f(other::allocator<int>*) {}
''')
for label,binary in [('student',compiler),('host','g++')]:
    run([binary,'-std=c++11','-c',s,'-o',out/(label+'.o')])
a=run(['nm','--defined-only',out/'student.o']);b=run(['nm','--defined-only',out/'host.o'])
assert {l.split()[-1] for l in a.splitlines()}=={l.split()[-1] for l in b.splitlines()},(a,b)
report=dict(binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),commands=records)
(out/'linkage-controls.json').write_text(json.dumps(report,indent=2)+'\n')
print('PA27 linkage controls PASS:',len(records),'commands')
