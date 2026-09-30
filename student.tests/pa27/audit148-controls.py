#!/usr/bin/env python3
"""Whole-stage audit: projected access, canonical receivers and GOT scratch use."""
import hashlib,json,os,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else root/'dev/cppgm++'
records=[]
def run(args,success=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 records.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr))
 (out/'controls.json').write_text(json.dumps(dict(binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),commands=records),indent=2)+'\n')
 assert (p.returncode==0)==success,records[-1]
 return p.stdout
def source(name,body):
 p=out/(name+'.cpp');p.write_text(body+'\n');return p
cases={
'qualified-union':'''struct B{long prefix;union{int a;long unused;};B(int x):prefix(3),a(x){}};
struct L:B{L():B(7){}};struct R:B{R():B(9){}};
struct D:L,R{int read(){return L::a+R::a;}};
int main(){D d;return d.L::a==7&&d.R::a==9&&d.read()==16?0:1;}''',
'qualified-nested':(root/'student.tests/pa27/audit-storage.cpp').read_text(),
'protected-union':'''struct B{protected:union{int x;};B(int n):x(n){}};
struct D:B{D():B(11){}int read(){return x;}};int main(){D d;return d.read()-11;}''',
'virtual-union':'''struct B{long prefix;union{int a;long unused;};B(int x):prefix(3),a(x){}};
struct L:virtual B{L():B(7){}};struct R:virtual B{R():B(9){}};
struct D:L,R{D():B(13){}int read(){return L::a+R::a;}};
int main(){D d;return d.L::a==13&&d.R::a==13&&d.read()==26?0:1;}''',
'constexpr-qualified':'''struct B{int prefix;union{int a;};constexpr B(int x):prefix(3),a(x){}};
struct L:B{constexpr L():B(7){}};struct R:B{constexpr R():B(9){}};
struct D:L,R{constexpr D(){} constexpr int read()const{return L::a+R::a;}};
constexpr D d;static_assert(d.read()==16,"selected anonymous storage");int main(){return d.read()-16;}''',
'nested-receiver':'''constexpr int read(const int& x){return x+1;}
struct S{long prefix;struct{int a;struct{int b;int c=read(b);int d=this->c+1;};};
constexpr S(int x):prefix(3),a(x),b(a+1){}};
constexpr S s(7);constexpr S t(11);static_assert(s.d==10&&t.d==14,"shared receiver paths");
int main(){S x(19);return x.a==19&&x.b==20&&x.c==21&&x.d==22?0:1;}''',
}
for name,body in cases.items():
 s=source(name,body)
 for level in ('-O0','-O2'):
  obj=out/(name+level+'.o');exe=out/(name+level)
  run([compiler,level,'-c',s,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
 # The course permits nested anonymous structs with default initializers;
 # GCC rejects that extension. Standard anonymous-union access is corroborated.
 if name!='nested-receiver':
  host=out/(name+'-host');run(['g++','-std=c++11',s,'-o',host]);run([host])
for name,body in {
'private':'struct B{private:union{int x;};};int main(){B b;return b.x;}',
'private-base':'struct B{union{int x;};};struct D:private B{};int main(){D d;return d.x;}',
'protected-outside':'struct B{protected:union{int x;};};struct D:B{};int main(){D d;return d.x;}',
'ambiguous':'struct B{union{int x;};};struct L:B{};struct R:B{};struct D:L,R{};int main(){D d;return d.x;}',
}.items():
 s=source(name,body)
 for binary in (compiler,'g++'):run([binary,'-std=c++11','-c',s,'-o',out/'rejected.o'],False)
# Imported addresses cover multiple scratch consumers, aggregate copies,
# narrow promotion, FP conversion, indexed stores and pointer displacements.
h=source('got-host','''long first=21,second=7;unsigned char byte=5;double real=4.5;
struct Pair{long x,y;};Pair pairs[2]={{1,2},{3,4}};
extern "C" int check(){return first==28&&second==7&&byte==26&&real==11.5&&pairs[1].x==1&&pairs[1].y==2;}''')
s=source('got-user','''extern long first,second;extern unsigned char byte;extern double real;
struct Pair{long x,y;};extern Pair pairs[2];extern "C" int check();
int main(){if(first+second!=28||first-second!=14||first*second!=147||first/second!=3||first%second!=0||first<=second)return 1;
byte+=first;real+=second;first+=second;pairs[1]=pairs[0];return check()?0:2;}''')
dso=out/'input.so';run(['g++','-shared','-fPIC',h,'-o',dso])
for level in ('-O0','-O2'):
 obj=out/('got'+level+'.o');exe=out/('got'+level)
 run([compiler,level,'-c',s,'-o',obj]);run(['g++',obj,dso,'-Wl,-z,text','-o',exe]);run([exe])
 rel=run(['readelf','-rW',obj])
 for symbol in ('first','second','byte','real','pairs'):
  assert any('GOTPCREL' in line and ' '+symbol+' ' in line for line in rel.splitlines()),symbol
print('PA27 audit controls PASS:',len(records),'commands')
