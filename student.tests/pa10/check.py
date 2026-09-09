#!/usr/bin/env python3
"""Independent procedural behavior tests; explicitly run, never course-discovered."""
from pathlib import Path
import subprocess
import tempfile
import sys
ROOT = Path(__file__).resolve().parents[2]
COMPILER = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'dev/cppgm++'
CASES = {
'c-linkage-namespaces': r'''
namespace A {extern "C" int f();}
namespace B {extern "C" int f(){return 7;}}
int main(){return A::f()==7?0:1;}
''',
'generated-global-identities': r'''
int x=1; int __global_x(){return 2;}
int __reference_5(){return 3;} const int&r=4;
int main(){return x+__global_x()+__reference_5()+r==10?0:1;}
''',
'volatile-value-contexts': r'''
volatile int x=1, y=2;
int readback(){int a=(x=3);int b=++x;return a+b;}
void mixed(){true?x:(x=5);}
void both(){true?x:y;}
void prvalue(){true?(x=6):2;}
int main(){if(readback()!=7)return 1;mixed();both();prvalue();return x==6?0:1;}
''',
'nested-control-entries': r'''
int jump(){goto L;if(false){L:return 7;}return 0;}
int loop(){int x=0;goto L;while(false){L:++x;continue;}return x;}
int dispatch(){int x=0;switch(2){while(false){case 2:x=3;break;}}return x;}
int storage(){goto L;int x;L:x=9;return x;}
int main(){return jump()==7 && loop()==1 && dispatch()==3 && storage()==9 ? 0:1;}
''',
'static-conversions-addresses': r'''
int a[3]={2,4,6}; int *p=1+a; int *q=&2[a]; int &r=*(a+1);
bool fraction=0.5, negative=-0.5, zero=0.0, address=&a[1];
bool chosen=true?0.5:0.0; unsigned char narrowed=(unsigned char)256;
int main(){return p==a+1 && *q==6 && r==4 && fraction && negative
 && !zero && address && chosen && narrowed==0 ? 0:1;}
''',
'callee-order': r'''
int order=0; int f(int n){return n;}
int (*pick())(int){order=order*10+1;return f;}
int arg(){order=order*10+2;return 0;}
int main(){pick()(arg());return order==21?0:1;}
''',
# The course fixes RHS-before-LHS-address for assignment, including compound.
'assignment-order': r'''
int order=0, value=0;
int& left(){order=order*10+1;return value;}
int right(){order=order*10+2;return 3;}
int main(){left()+=right();return order==21 && value==3 ? 0:1;}
''',
'control-defaults': r'''
namespace N { int seed=3; int plus(int n=seed) { return n+2; } }
int &alias(int &n) { return n; }
int sum(int n) {
  int result=0;
  for(int i=0;i<n;++i) {
    switch(i) { case 1: continue; case 4: break; default: alias(result)+=i; }
  }
  return result;
}
int main(){ int seed=90; int x=0; do { ++x; } while(x<2);
  if(N::plus()!=5 || sum(6)!=10 || x!=2) return 1;
  goto done;
  { int unreachable=3; }
done: return 0;
}
''',
'increment-widths': r'''
int main(){ signed char c=126; bool b=true; bool old=b++;
  if(!old || !b || int(b)!=1) return 1;
  if(++c != 127) return 2;
  unsigned char u=255; if(u++ != 255 || u != 0) return 3;
  int a[4]={1,2}; int* p=a; int*& r=p;
  r+=3; --r; if(*r || r-a!=2 || a-r!=-2 || a[3]) return 4;
  char rows[4][3]; if(&rows[1]-&rows[3]!=-2) return 5;
  return 0;
}
''',
'volatile-discard': r'''
volatile int counter=1;
int main(){counter; (counter,0); (void)counter; counter+=2; return counter-3;}
''',
'global-storage': r'''
int data[4]={2,3}; int *end=data+4; int *second=&data[1];
const int value=7; const int &r=value; const int &s=r;
const char* names[3]={"abc",nullptr};
int twice(int x){return x*2;} int (*function)(int)=twice;
int main(){ return end-data==4 && *second==3 && s==7 && names[0][2]=='c'
  && names[1]==nullptr && names[2]==nullptr && function(4)==8 ? 0:1; }
''',
'aggregate-padding': r'''
struct Record {char tag; int number; volatile short state;};
Record global={'a',7,3};
int main(){Record local={'b',9,4}; local.state=5;
 return global.tag=='a' && global.number==7 && global.state==3
 && local.tag=='b' && local.number==9 && local.state==5 ? 0:1;}
''',
'nested-calls': r'''
int f(int x){return x+1;} int g(int x,int y){return x*10+y;}
int (*fp)(int)=f;
int main(){return g(fp(g(f(1),f(2))),fp(4))==245 ? 0:1;}
''',
'return-references': r'''
int& pick(bool c,int &a,int &b){return c?a:b;}
int main(){int a=3,b=7; int &r=pick(false,a,b); r=9;
 int *p=&(a=4); return *p==4 && a==4 && b==9 ? 0:1;}
'''
}
BAD = {
'jump-condition-initializer': 'int main(){goto L;if(int x=1){L:return x;}return 0;}',
'jump-loop-condition': 'int main(){goto L;while(int x=1){L:return x;}return 0;}',
'jump-initializer': 'int main(){goto end; int x=1; end:return 0;}',
'switch-initializer': 'int main(){switch(1){case 0:int x=0;case 1:return 0;}}',
'array-excess': 'int main(){int a[1]={1,2};return 0;}',
'default-order': 'int f(int x=1,int y);int main(){return 0;}',
'undefined-label': 'int main(){goto missing;}',
'duplicate-label': 'int main(){label:;label:return 0;}',
'bad-conversion': 'enum class E{a};int main(){int x=E::a;return x;}'
}
with tempfile.TemporaryDirectory(prefix='pa10-personal-') as work:
    work=Path(work)
    for name,source in CASES.items():
        src, ir, exe = work/f'{name}.cpp',work/f'{name}.lowir',work/name
        src.write_text(source)
        r=subprocess.run([COMPILER,'--emit-lowir','--validate-lowir','-O0','-o',ir,src],capture_output=True,text=True)
        assert r.returncode==0,(name,r.stderr)
        if name=='volatile-discard':
            assert ir.read_text().count('load volatile')==5,ir.read_text()
        if name=='volatile-value-contexts':
            # Three readback loads, zero mixed-arm loads, two possible both-arm
            # loads, one prvalue-arm readback, and one final check.
            assert ir.read_text().count('load volatile')==7,ir.read_text()
        native=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
        assert native.returncode==0,(name,native.stderr,ir.read_text())
        r=subprocess.run([exe],capture_output=True,timeout=10)
        assert r.returncode==0 and r.stdout==b'',(name,r.returncode,r.stdout,ir.read_text())
        print('PASS',name)
    for name,source in BAD.items():
        src=work/f'{name}.cpp';src.write_text(source)
        r=subprocess.run([COMPILER,'--emit-lowir','--validate-lowir','-O0','-o',work/'bad.lowir',src],capture_output=True,text=True)
        assert r.returncode==1,(name,r.returncode,r.stderr)
        print('PASS rejection',name)
print(f'PASS: {len(CASES)} native programs and {len(BAD)} semantic rejections')
