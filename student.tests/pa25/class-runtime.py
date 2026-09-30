#!/usr/bin/env python3
"""Standalone PA25 class runtime, executed explicitly. No host runtime oracle."""
import pathlib, subprocess, tempfile, os
ROOT=pathlib.Path(__file__).resolve().parents[2]
CXX=os.environ.get('CPPGM_TEST_CXX',str(ROOT/'dev/cppgm++'))
checks=0
def run(args,expected=0):
    global checks
    p=subprocess.run(list(map(str,args)),stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=30)
    assert p.returncode==expected,(args,p.returncode,p.stderr.decode(errors='replace'))
    checks+=1
    return p
cases={
'allocation':r'''
int alive, destroyed;
struct Value { int tag; Value():tag(++alive) {} ~Value(){destroyed=destroyed*10+tag;} };
struct A { virtual ~A(){} int a; };
struct B { virtual ~B(){} int b; };
struct D:A,B { ~D(){++alive;} };
int main(){
 for(int n=0;n<20000;n+=137){unsigned char* p=new unsigned char[n];
 if(!p || (unsigned long)p%16) return 1;
 for(int i=0;i<n;++i)p[i]=(unsigned char)i;
 for(int i=0;i<n;++i)if(p[i]!=(unsigned char)i)return 2;
 delete[] p;}
 Value* a=new Value[3]; delete[] a;
 if(destroyed!=321)return 3;
 B* b=new D; delete b; if(alive!=4)return 4;
 int* z=0; delete z; return 0;
}
''',
'casts':r'''
struct A { virtual ~A(){} int a; };
struct B { virtual ~B(){} int b; };
struct D:A,B { int d; };
struct Other:A {};
int main(){ D d; A* a=&d; B* b=&d; A* null=0;
 if(dynamic_cast<D*>(a)!=&d || dynamic_cast<D*>(b)!=&d)return 1;
 if(dynamic_cast<B*>(a)!=b || dynamic_cast<A*>(b)!=a)return 2;
 if(dynamic_cast<void*>(b)!=&d || dynamic_cast<D*>(null))return 3;
 if(dynamic_cast<Other*>(a))return 4;
 return 0;
}
''',
'repeated':r'''
struct A { virtual ~A(){} };
struct T:A {}; struct U:T {}; struct V:T {}; struct D:U,V {};
int main(){ D d; U* u=&d; V* v=&d; A* au=u; A* av=v;
 if(dynamic_cast<T*>(au)!=(T*)u || dynamic_cast<T*>(av)!=(T*)v)return 1;
 if(dynamic_cast<V*>(au)!=v || dynamic_cast<U*>(av)!=u)return 2;
 if(dynamic_cast<D*>(au)!=&d || dynamic_cast<D*>(av)!=&d)return 3;
 return 0;
}
''',
'virtual':r'''
struct A { virtual ~A(){} int a; };
struct L:virtual A { int l; }; struct R:virtual A { int r; }; struct D:L,R {};
struct T:virtual A {}; struct U:T {}; struct V:T {}; struct Amb:U,V {};
struct Priv:private virtual A { A* get(){return this;} };
struct Pub:public virtual A {}; struct Mixed:Priv,Pub {};
int main(){ D d; A* a=&d; R* r=&d;
 if(dynamic_cast<D*>(a)!=&d || dynamic_cast<R*>(a)!=r)return 1;
 if(dynamic_cast<L*>(r)!=(L*)&d || dynamic_cast<void*>(a)!=&d)return 2;
 Amb amb; A* base=&amb; if(dynamic_cast<T*>(base))return 3;
 Mixed m; A* shared=((Priv*)&m)->get();
 if(dynamic_cast<Mixed*>(shared)!=&m || dynamic_cast<Pub*>(shared)!=(Pub*)&m)return 4;
 return 0;
}
''',
'access':r'''
struct A { virtual ~A(){} };
struct B { virtual ~B(){} };
struct Hidden:private A,public B { A* get(){return this;} };
struct Root { virtual ~Root(){} };
struct Public:Root {}; struct Private:private Root { Root* get(){return this;} };
struct Complete:Public,Private {};
int main(){ Hidden h; A* a=h.get(); if(dynamic_cast<B*>(a) || dynamic_cast<Hidden*>(a))return 1;
 Complete d; Root* pub=(Public*)&d; Root* priv=((Private*)&d)->get();
 if(dynamic_cast<Complete*>(pub)!=&d || dynamic_cast<Public*>(priv))return 2;
 return 0;
}
''',
'constant-self':r'''
struct Self { Self* p; int value; constexpr Self():p(this),value(7){} virtual int f(){return value;} };
Self a; Self b;
int main(){if(a.p!=&a || b.p!=&b)return 1; a.value=17; return a.p->f()==17 && b.p->f()==7?0:2;}
''',
'access-ambiguity':r'''
struct A { virtual ~A(){} }; struct T { virtual ~T(){} };
struct Pub:public T {}; struct Priv:private T {}; struct D:A,Pub,Priv {};
struct V { virtual ~V(){} };
struct Target:public virtual V {}; struct Hidden:private virtual V {};
struct One:Target {}; struct Two:Target {}; struct Both:One,Two {};
int main(){D d; A* a=&d; if(dynamic_cast<T*>(a))return 1;
 Both both; V* v=&both; if(dynamic_cast<Target*>(v))return 2; return 0;}
''',
'construction':r'''
int wrong;
struct A { virtual ~A(){} };
struct B:virtual A { B(); ~B(); }; struct D:B {};
B::B(){A* a=this; if(dynamic_cast<B*>(a)!=this || dynamic_cast<D*>(a))++wrong;}
B::~B(){A* a=this; if(dynamic_cast<B*>(a)!=this || dynamic_cast<D*>(a))++wrong;}
int main(){ {D d; A* a=&d; if(dynamic_cast<D*>(a)!=&d)return 1;} return wrong;}
'''
}
with tempfile.TemporaryDirectory(prefix='pa25-class-') as tmp:
    d=pathlib.Path(tmp)
    for name,text in cases.items():
        src=d/(name+'.cc'); src.write_text(text); obj=d/(name+'.obj'); exe=d/name
        run([CXX,'-c','-o',obj,src]); run([CXX,'-o',exe,obj]); run([exe])
        run([CXX,'-o',exe,src]); run([exe])
    header=d/'types.h'; header.write_text('''struct A{virtual ~A(){}}; struct B{virtual ~B(){}};
template<int N> struct Box:A,B{int value(){return N;}};
A* make(); int inspect(A*); void destroy(A*);
''')
    a=d/'a.cc'; a.write_text('#include "types.h"\nA* make(){return new Box<73>;} void destroy(A* a){delete a;}')
    b=d/'b.cc'; b.write_text('#include "types.h"\nint inspect(A* a){Box<73>* p=dynamic_cast<Box<73>*>(a); return p && dynamic_cast<B*>(a)==(B*)p ? p->value():0;}')
    c=d/'c.cc'; c.write_text('#include "types.h"\nint main(){A* a=make();int n=inspect(a); destroy(a);return n-73;}')
    objs=[]
    for src in [a,b,c]:
        obj=src.with_suffix('.obj'); run([CXX,'-c','-o',obj,src]); objs.append(obj)
    exe=d/'cross'
    for inputs in [objs,[a,b,c],[objs[0],b,c],list(reversed(objs))]:
        run([CXX,'-o',exe,*inputs]); run([exe])
    # A shared spelling in anonymous namespaces must not become RTTI identity.
    common=d/'common.h'; common.write_text('struct Base{virtual ~Base(){}}; Base* local_make(); int local_check(Base*);')
    left=d/'local-a.cc'; left.write_text('#include "common.h"\nnamespace {struct Local:Base{};} Base* local_make(){return new Local;}')
    right=d/'local-b.cc'; right.write_text('#include "common.h"\nnamespace {struct Local:Base{};} int local_check(Base* b){return dynamic_cast<Local*>(b)==0;} int main(){Base* p=local_make();int okay=local_check(p);delete p;return !okay;}')
    run([CXX,'-o',exe,left,right]);run([exe])
    # Required static initialization must precede every TU's dynamic hooks.
    h=d/'static.h';h.write_text("""struct L{int l; constexpr L(int x):l(x){} virtual int f()const{return l;}};
struct R{int r; constexpr R(int x):r(x){} virtual int g()const{return r;}};
struct D:L,R{constexpr D(int x):L(x),R(x+1){} int f()const{return l+100;} int g()const{return r+200;}};
union H{D d;constexpr H():d(7){}};
struct Outer{int first; H h; int last; constexpr Outer():first(3),h(),last(9){}};
extern Outer value; extern H values[2];
""")
    init=d/'static-init.cc'; init.write_text('#include "static.h"\nOuter value; H values[2]={{},{}};')
    use=d/'static-use.cc'; use.write_text('#include "static.h"\nint observe(){L* l=&value.h.d;R* r=&value.h.d;L* a=&values[1].d;return l->f()+r->g()+a->f()+value.first+value.last;}int observed=observe();int main(){return observed==434?0:1;}')
    for src in [init,use]:run([CXX,'-c','-o',src.with_suffix('.obj'),src])
    for inputs in [[use,init],[use.with_suffix('.obj'),init.with_suffix('.obj')],[use,init.with_suffix('.obj')]]:
        run([CXX,'-o',exe,*inputs]);run([exe])
    # A discarded weak definition must not demand its allocator dependencies.
    weak=d/'weak.cc'; weak.write_text('extern "C" inline int choose(){int* p=new int;delete p;return 0;} int invoke(){return choose();}')
    strong=d/'strong.cc';strong.write_text('extern "C" int choose(){return 7;} int invoke();int main(){return invoke()-7;}')
    for inputs in [[weak,strong],[strong,weak]]:
        result=run([CXX,'--stats','-o',exe,*inputs]);run([exe])
        assert b'"runtime_functions":0' in result.stderr,result.stderr
    for src in [a,b,c]:src.unlink()
    run([CXX,'-o',exe,*objs]); run([exe])
print(f'{checks} class runtime checks passed')
