#!/usr/bin/env python3
"""Independent binding and storage controls: C++11 [except.handle], [dcl.align].

Mixed pointer qualifications use [conv.qual]/4-7. In C++11 a root member
pointer has no qualification catch conversion; keep that negative control.
"""
import json, os, pathlib, subprocess, sys
ROOT = pathlib.Path(__file__).resolve().parents[2]
CXX = pathlib.Path(os.environ.get('CPPGM_TEST_CXX', ROOT/'dev/cppgm++')).resolve()
OUT = pathlib.Path(sys.argv[1]).resolve(); OUT.mkdir(parents=True, exist_ok=True)
cases = {
'member-converted-reference-identity': '''struct A{int x;};int main(){try{throw nullptr;}catch(int A::*const& p){try{throw;}catch(int A::*const& q){return &p==&q;}return 2;}return 3;}''',
'member-null-reference-storage': '''struct A{int x;void f(){}};int main(){try{throw nullptr;}catch(int A::*const& p){try{throw;}catch(void(A::*const& q)()){if(q!=nullptr)return 1;}return p!=nullptr;}return 2;}''',
'member-function-null-reference-storage': '''struct A{int x;void f(){}};int main(){try{throw nullptr;}catch(void(A::*const& p)()){try{throw;}catch(int A::*const& q){if(q!=nullptr)return 1;}return p!=nullptr;}return 2;}''',
'aligned-declaration': '''char prefix=1;alignas(64) int a=7;alignas(128) int b;
int main(){alignas(64) int c=9;alignas(128) static int d=11;return ((unsigned long)&a)%64 || ((unsigned long)&b)%128 || ((unsigned long)&c)%64 || ((unsigned long)&d)%128 || a!=7 || b || c!=9 || d!=11;}''',
'aligned-template-declaration': '''template<int N> int f(){alignas(N) int a=7;return ((unsigned long)&a)%N || a!=7;}int main(){return f<64>() || f<128>();}''',
'packed-static': '''#pragma pack(push,1)
struct A{char x;int y;constexpr A():x(3),y(7){}};
#pragma pack(pop)
A a;int main(){return sizeof(A)!=5 || a.x!=3 || a.y!=7;}''',
'packed-relocation': '''int value=7;
#pragma pack(push,1)
struct A{char x;int* p;constexpr A():x(3),p(&value){}};
#pragma pack(pop)
A a;int main(){return sizeof(A)!=9 || a.x!=3 || a.p!=&value || *a.p!=7;}''',
'packed-array': '''#pragma pack(push,1)
struct A{char x;int y;};
#pragma pack(pop)
A a[2]={{3,7},{4,9}};int main(){return sizeof(A)!=5 || a[0].x!=3 || a[0].y!=7 || a[1].x!=4 || a[1].y!=9;}''',
'aligned-static': '''char prefix=1;struct alignas(64) A{int x;constexpr A():x(7){}};A a,b;
int main(){return ((unsigned long)&a)%64 || ((unsigned long)&b)%64 || a.x!=7 || b.x!=7;}''',
'aligned-zero': '''char prefix=1;struct alignas(128) A{int x;};A a;
int main(){return ((unsigned long)&a)%128 || a.x;}''',
'converted-miss-catchall': '''int main(){int a;try{throw &a;}catch(float*const&){return 1;}catch(...){return 0;}return 2;}''',
'converted-rethrow-catchall': '''int main(){int a;try{try{throw &a;}catch(const int*const&){throw;}}catch(...){return 0;}return 2;}''',
'pointer-reference-mutation': '''int main(){int a=1,b=2;try{try{throw &a;}catch(int*& p){p=&b;throw;}}catch(int* p){return p!=&b;}return 2;}''',
'pointer-reference-conversion': '''int main(){int a=1;try{throw &a;}catch(const int*&){return 1;}catch(const int* p){return p!=&a;}return 2;}''',
'pointer-reference-base': '''struct A{int x;};struct B{int y;};struct D:A,B{};int main(){D d;try{throw &d;}catch(B*&){return 1;}catch(B* p){return p!=(B*)&d;}return 2;}''',
'pointer-const-reference-exact': '''int main(){int a;try{throw &a;}catch(int*const& p){try{throw;}catch(int*const& q){return &p!=&q;}return 2;}return 3;}''',
'pointer-const-reference-mutation': '''int main(){int a,b;try{throw &a;}catch(int*const& p){try{throw;}catch(int*& q){q=&b;}return p!=&b;}return 3;}''',
'pointer-const-reference-converted': '''int main(){int a=7;try{throw &a;}catch(const int*const& p){return p!=&a || *p!=7;}return 2;}''',
'pointer-converted-storage': '''struct A{int a;};struct B{int b;};struct D:A,B{};int main(){D d;try{throw &d;}catch(B*const& p){try{throw;}catch(A*const& q){if(q!=(A*)&d)return 1;}return p!=(B*)&d;}return 2;}''',
'pointer-null-const-reference': '''int main(){try{throw nullptr;}catch(int*const& p){return p!=0;}return 2;}''',
'pointer-null-reference-rejected': '''int main(){try{throw nullptr;}catch(int*&){return 1;}catch(int* p){return p!=0;}return 2;}''',
'member-null-reference-rejected': '''struct A{int x;};int main(){try{throw nullptr;}catch(int A::*&){return 1;}catch(int A::* p){return p!=nullptr;}return 2;}''',
'member-reference-mutation': '''struct A{int x,y;};int main(){try{try{throw &A::x;}catch(int A::*& p){p=&A::y;throw;}}catch(int A::* p){return p!=&A::y;}return 2;}''',
'member-qualification-rejected': '''struct A{int x;};int main(){try{throw &A::x;}catch(const int A::*){return 1;}catch(int A::*){return 0;}return 2;}''',
'mixed-member-qualification': '''struct A{int x;};int main(){int A::* p=&A::x;try{throw &p;}catch(const int A::*const* q){A a;a.x=7;return a.**q!=7;}catch(...){return 1;}return 2;}''',
'mixed-member-qualification-reference': '''struct A{int x;};int main(){int A::* p=&A::x;try{throw &p;}catch(const int A::*const*const& q){return *q!=&A::x;}catch(...){return 1;}return 2;}''',
'mixed-member-unsafe-qualification': '''struct A{int x;};int main(){int A::* p=&A::x;try{throw &p;}catch(const int A::**){return 1;}catch(int A::** q){return *q!=&A::x;}return 2;}''',
'mixed-member-owner-rejected': '''struct A{int x;};struct B:A{};int main(){int A::* p=&A::x;try{throw &p;}catch(const int B::*const*){return 1;}catch(int A::**){return 0;}return 2;}''',
'deep-pointer-base-rejected': '''struct A{};struct D:A{};int main(){D d;D* p=&d;try{throw &p;}catch(A*const*){return 1;}catch(D** q){return *q!=&d;}return 2;}''',
'function-pointer-reference': '''int f(){return 7;}int g(){return 9;}int main(){try{try{throw &f;}catch(int(*&p)()){p=&g;throw;}}catch(int(*p)()){return p()!=9;}return 2;}''',
'rethrow-adjusted-base': '''struct A{int a;A():a(3){}};struct B{int b;B():b(7){}};struct D:A,B{};int main(){try{try{throw D();}catch(B& b){if(b.b!=7)return 1;throw;}}catch(A& a){return a.a!=3;}return 2;}''',
}
records = []
def command(args):
    p = subprocess.run(list(map(str,args)), capture_output=True, text=True, timeout=30,
                       env={**os.environ, 'PATH': ''})
    return p
def check(name, mode, args, expected=0):
    p = command(args)
    records.append(dict(case=name,mode=mode,args=list(map(str,args)),status=p.returncode,
                        expected=expected,passed=(p.returncode==expected),stderr=p.stderr))
    return p.returncode == expected
for name, source in cases.items():
    src=OUT/(name+'.cc'); src.write_text(source+'\n')
    for mode in ('direct','object'):
        exe=OUT/(name+'-'+mode); obj=OUT/(name+'.obj')
        if mode=='object' and not check(name,'compile',[CXX,'-c','-o',obj,src]): continue
        if check(name,mode+'-link',[CXX,'-o',exe,obj if mode=='object' else src]):
            check(name,mode+'-run',[exe])
for name, declaration in [('incomplete-pointer','A*'),('incomplete-reference','A&')]:
    src=OUT/(name+'.cc');src.write_text('struct A; int main(){try{throw 1;}catch('+declaration+'){} }\n')
    p=command([CXX,'-c','-o',OUT/(name+'.obj'),src])
    records.append(dict(case=name,mode='reject',status=p.returncode,passed=p.returncode>0,stderr=p.stderr))
for name,text in [('weakened-alignment','alignas(1) int a;'),
                  ('invalid-alignment','alignas(3) int a;'),
                  ('conflicting-alignment','alignas(64) extern int a;alignas(128) int a;'),
                  ('aligned-function','alignas(64) int f(){return 0;}')]:
    src=OUT/(name+'.cc');src.write_text(text+'\n')
    p=command([CXX,'-c','-o',OUT/(name+'.obj'),src])
    records.append(dict(case=name,mode='reject',status=p.returncode,passed=p.returncode>0,stderr=p.stderr))
texts = ['void raise(int* p){throw p;}\n',
         'void raise(int*);int handle(int* a,int* b){try{raise(a);}catch(int*& p){p=b;throw;}return 1;}\n',
         'int handle(int*,int*);int main(){int a,b;try{handle(&a,&b);}catch(int*const& p){return p!=&b;}return 2;}\n']
sources=[];objects=[]
for i, text in enumerate(texts):
    src=OUT/f'cross{i}.cc';src.write_text(text);sources.append(src);obj=OUT/f'cross{i}.obj';objects.append(obj)
    check('cross-tu','compile',[CXX,'-c','-o',obj,src])
for mode,inputs in [('direct',sources),('objects',objects),('mixed',[objects[0],*sources[1:]])]:
    exe=OUT/('cross-'+mode)
    if check('cross-tu',mode+'-link',[CXX,'-o',exe,*inputs]):check('cross-tu',mode+'-run',[exe])
layout_header=OUT/'layout.h'
layout_header.write_text('#pragma pack(push,1)\nstruct Packed{char tag;int* value;};\n#pragma pack(pop)\nstruct alignas(128) Aligned{int value;};extern Packed packed;extern Aligned aligned;extern int value;\n')
sources=[];objects=[]
for i,text in enumerate(['#include "layout.h"\nchar prefix=3;int value=7;Packed packed={9,&value};Aligned aligned={11};\n',
                         '#include "layout.h"\nint main(){return packed.tag!=9 || packed.value!=&value || *packed.value!=7 || ((unsigned long)&aligned)%128 || aligned.value!=11;}\n']):
    src=OUT/f'layout{i}.cc';src.write_text(text);sources.append(src);obj=OUT/f'layout{i}.obj';objects.append(obj)
    check('layout-cross-tu','compile',[CXX,'-c','-o',obj,src])
for mode,inputs in [('direct',sources),('objects',objects),('mixed',[objects[0],sources[1]])]:
    exe=OUT/('layout-'+mode)
    if check('layout-cross-tu',mode+'-link',[CXX,'-o',exe,*inputs]):check('layout-cross-tu',mode+'-run',[exe])
(OUT/'results.json').write_text(json.dumps(records,indent=2)+'\n')
failed=[r for r in records if not r['passed']]
print(json.dumps(failed,indent=2)); print(f'{len(records)-len(failed)}/{len(records)} checks passed')
sys.exit(bool(failed))
