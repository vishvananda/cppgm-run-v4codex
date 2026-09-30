#!/usr/bin/env python3
"""Independent default-ELF, host EH, TLS and compiler-owned link controls."""
import hashlib, json, os, pathlib, subprocess, sys
root = pathlib.Path(__file__).resolve().parents[2]
out = pathlib.Path(sys.argv[1]).resolve(); out.mkdir(parents=True, exist_ok=True)
cxx = pathlib.Path(sys.argv[2] if len(sys.argv)>2 else root/'dev/cppgm++').resolve()
records = []
def run(args, expected=0, empty_path=False):
    p = subprocess.run(list(map(str,args)), capture_output=True, timeout=30,
                       env={**os.environ, **({'PATH':''} if empty_path else {})})
    records.append(dict(args=list(map(str,args)),status=p.returncode,expected=expected,
                        stderr=p.stderr.decode(errors='replace'),stdout=p.stdout.decode(errors='replace')))
    if p.returncode != expected: raise RuntimeError(records[-1])
    return p.stdout
cases = {
'constructor-outer-handler':('''int dead,seen,caught;struct G{~G(){++dead;}};
int boom(){throw 7;}struct B{G a,b;int n;B()try:n(boom()){}catch(int x){seen=dead;caught=x;}};
int main(){try{B b;}catch(...){return dead!=2||seen!=2||caught!=7;}return 9;}''',0),
'nested-source-catches':('''int dead;struct G{~G(){++dead;}};
int main(){try{G a;try{G b;throw 7L;}catch(int){return 1;}}catch(long n){return dead!=2||n!=7;}return 2;}''',0),
'boundary-selector-identity':('''extern "C" void _Exit(int);namespace std{void (*set_terminate(void(*)()))();}
void stop(){_Exit(73);}void f()noexcept{try{throw 7L;}catch(int){_Exit(1);}}
int main(){std::set_terminate(stop);f();return 2;}''',73),
'tls-value':('''thread_local int n=7;int main(){return ++n!=8;}''',0),
'tls-alignment':('''char pad;thread_local char c=3;alignas(64) thread_local int n=7;
int main(){return ((unsigned long)&n)%64||n!=7||c!=3;}''',0),
'tls-relocation':('''int n=9;thread_local int* p=&n;int main(){return *p!=9||p!=&n;}''',0),
'lifecycle':('''extern "C" void _Exit(int);int state;struct G{G(){state=7;}~G(){_Exit(state==9?73:1);}}g;
int main(){state+=2;return 2;}''',73),
'function-address':('''extern "C" void* malloc(unsigned long);extern "C" void free(void*);
extern "C" void* dlsym(void*,const char*);int main(){void* p=malloc(7);free(p);
return reinterpret_cast<void*>(&malloc)!=dlsym(0,"malloc");}''',0),
'imported-data':('''extern "C" void* stdout;extern "C" int fflush(void*);
int main(){return !stdout||fflush(stdout);}''',0),
'linkage-definition':('''extern "C" int value;extern "C" {int value=7;}
extern "C++" int other;int other=9;int main(){return value!=7||other!=9;}''',0),
}
try:
    for name,(text,status) in cases.items():
        src=out/(name+'.cpp'); src.write_text(text+'\n'); obj=out/(name+'.obj')
        run([cxx,'-c','-o',obj,src],empty_path=True)
        assert obj.read_bytes()[:4]==b'\x7fELF'
        for mode,inputs in [('direct',[src]),('object',[obj])]:
            exe=out/(name+'-'+mode); run([cxx,*inputs,'-o',exe],empty_path=True); run([exe],status)
        exe=out/(name+'-host'); run(['g++',obj,'-o',exe]); run([exe],status)
    a=out/'tls-a.cpp'; b=out/'tls-b.cpp'
    a.write_text('thread_local int n=7;int touch(){return ++n;}\n')
    b.write_text('extern thread_local int n;int touch();int main(){return touch()!=8||n!=8;}\n')
    ao=out/'tls-a.obj'; bo=out/'tls-b.obj'
    run([cxx,'-c','-o',ao,a]); run([cxx,'-c','-o',bo,b])
    for mode,inputs in [('direct',[a,b]),('object',[ao,bo]),('mixed',[ao,b])]:
        exe=out/('tls-tu-'+mode); run([cxx,*inputs,'-o',exe],empty_path=True); run([exe])
    helper=out/'thread.cpp'; helper.write_text('''#include <pthread.h>
extern int touch();void* work(void*){return (void*)(long)(touch()!=8);}
int main(){pthread_t t;void* result=0;if(touch()!=8)return 1;
if(pthread_create(&t,0,work,0)||pthread_join(t,&result))return 2;
return result!=0||touch()!=9;}\n''')
    ho=out/'thread.o'; run(['g++','-std=c++11','-c',helper,'-o',ho])
    exe=out/'thread'; run([cxx,ao,ho,'-o',exe],empty_path=True); run([exe])
    helper=out/'throw.cpp'; helper.write_text('extern "C" void fail(){throw 7;}\n')
    src=out/'catch.cpp'; src.write_text('extern "C" void fail();int main(){try{fail();}catch(int n){return n!=7;}return 2;}\n')
    ho=out/'throw.o'; run(['g++','-c',helper,'-o',ho]); exe=out/'foreign-eh'
    run([cxx,src,ho,'-o',exe],empty_path=True); run([exe])
    assert b'R_X86_64_COPY' in run(['readelf','-rW',out/'imported-data-direct'])
    assert b'TLS' in run(['readelf','-lW',out/'thread'])
    assert b'GNU_EH_FRAME' in run(['readelf','-lW',out/'foreign-eh'])
finally:
    (out/'controls.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cxx.read_bytes()).hexdigest(),
        records=records,sources={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.glob('*.cpp')}),indent=2)+'\n')
print(f'{len(records)} link/EH/TLS commands passed')
