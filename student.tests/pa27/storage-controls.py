#!/usr/bin/env python3
"""Projected anonymous-storage initialization, lifetime and constant controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=root/'dev/cppgm++';records=[]
def run(args,ok=True):
 p=subprocess.run(list(map(str,args)),capture_output=True,timeout=30)
 records.append(dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout.decode(),stderr=p.stderr.decode()))
 (out/'storage-controls.json').write_text(json.dumps(dict(binary_sha256=hashlib.sha256(compiler.read_bytes()).hexdigest(),commands=records),indent=2)+'\n')
 assert (p.returncode==0)==ok,records[-1]
 return p
cases={
'nested-order':'''int log; int mark(int x){log=log*10+x;return x;}
struct S{int prefix;struct{int a;struct{int b;int c;};int d;};int tail;
S():tail(mark(6)),c(mark(4)),b(mark(3)),prefix(mark(1)),d(mark(5)),a(mark(2)) {}};
int main(){S s;return log==123456&&s.prefix==1&&s.a==2&&s.b==3&&s.c==4&&s.d==5&&s.tail==6?0:1;}''',
'nontrivial-copy':'''int copies,defaults,dead;struct R{int& r;R(int& x):r(x){++defaults;}R(const R& x):r(x.r){++copies;}~R(){++dead;}};
struct S{long prefix;struct{int* p;R r;};S(int* p,const R& r):prefix(91),p(p),r(r){}};
int main(){int x=7;{R r(x);{S s(&x,r);if(s.prefix!=91||s.p!=&x||&s.r.r!=&x||defaults!=1||copies!=1||dead)return 1;}if(dead!=1)return 2;}return dead==2?0:3;}''',
'defaults':'''struct S{int prefix=1;struct{int a=4;struct{int b=5;};int c=6;};S(){} S(int x):b(x){}};
union U{struct{int a=8;struct{int b=9;};};long other;U(){} U(int x):b(x){}};
int main(){S s;S t(7);U u;U v(10);return s.prefix==1&&s.a==4&&s.b==5&&s.c==6&&t.a==4&&t.b==7&&t.c==6&&u.a==8&&u.b==9&&v.a==8&&v.b==10?0:1;}''',
'union-select':'''union U{struct{int a;int b;};long other;U(int x):a(x),b(x+1){} U():other(99){}};
struct S{int prefix;union{struct{int a;int b;};long other;};S(int x):prefix(3),b(x+1),a(x){} S():prefix(4),other(88){}};
int main(){U u(7);U v;S s(5);S t;return u.a==7&&u.b==8&&v.other==99&&s.prefix==3&&s.a==5&&s.b==6&&t.other==88?0:1;}''',
'array-bits':'''struct A{int x;A():x(17){}};struct S{long prefix;struct{unsigned a:3;unsigned b:5;A values[2];int tail;};S():prefix(123),b(19),a(5),tail(9){}};
int main(){S s;return s.prefix==123&&s.a==5&&s.b==19&&s.values[0].x==17&&s.values[1].x==17&&s.tail==9?0:1;}''',
'cleanup':'''int log;struct R{int x;R(int n):x(n){if(n==3)throw 3;log=log*10+n;}~R(){log=log*10+x+5;}};
struct S{R first;struct{R a;struct{R b;};};R last;S():last(3),b(2),a(1),first(0){}};
int main(){try{S s;}catch(int x){return x==3&&log==12765?0:1;}return 2;}''',
'cleanup-complete':'''int log;struct R{int x;R(int n):x(n){log=log*10+n;}~R(){log=log*10+x+5;}};
struct S{R first;struct{R a;struct{R b;};};S():b(2),a(1),first(0){throw 3;}};
int main(){try{S s;}catch(int x){return x==3&&log==12765?0:1;}return 2;}''',
'move':'''int moved;struct R{int value;R(int n):value(n){}R(const R&)=delete;R(R&& x):value(x.value){x.value=-1;++moved;}};
struct S{long prefix;struct{R a;};S(R&& x):prefix(41),a(static_cast<R&&>(x)){}};
int main(){R r(7);S s(static_cast<R&&>(r));return s.prefix==41&&s.a.value==7&&r.value==-1&&moved==1?0:1;}''',
'default-sibling':'''struct S{long prefix;struct{int a=5;int b=a+1;};S():prefix(9){}};
int main(){S s;return s.prefix==9&&s.a==5&&s.b==6?0:1;}''',
'mixed-default-receiver':'''struct S{long prefix;struct{int a=5;int b=this->a+1;int* p=&a;};S():prefix(9){} S(int x):prefix(10),a(x){}};
int main(){S s;S t(7);return s.prefix==9&&s.b==6&&s.p==&s.a&&t.prefix==10&&t.a==7&&t.b==8&&t.p==&t.a?0:1;}''',
'constant-default-reference':'''constexpr int read(const int& x){return x+1;}
struct S{long prefix;struct{int a;int b=read(a);};constexpr S(int x):prefix(9),a(x){}};
constexpr S s(7);constexpr S t(10);static_assert(s.b==8&&t.b==11,"projected argument identity");int main(){return s.b==8&&t.b==11?0:1;}''',
'constant-default-receiver':'''struct S{long prefix;struct{int a;int b=this->a+1;};constexpr S(int x):prefix(9),a(x){}};
constexpr S s(7);static_assert(s.a==7&&s.b==8,"default receiver");int main(){S t(10);return t.b==11?0:1;}''',
'implicit-defaults':'''struct S{long prefix=3;struct{int a=4;int b=5;};};
union U{struct{int a=8;int b=9;};};int main(){S s;U u;return s.prefix==3&&s.a==4&&s.b==5&&u.a==8&&u.b==9?0:1;}''',
'constant-union':'''union S{struct{int a;int b;};long other;constexpr S(int x):b(x+1),a(x){}};
constexpr S s(3);static_assert(s.a==3&&s.b==4,"active storage");int main(){return s.b==4?0:1;}''',
'constant':'''struct S{long prefix;struct{int a;struct{int b;};};constexpr S(int x):prefix(4),a(x),b(x+1){}};
constexpr S s(7);static_assert(s.prefix==4&&s.a==7&&s.b==8,"projected values");
int main(){return s.a==7&&s.b==8?0:1;}''',
'constant-sibling':'''struct S{long prefix;struct{int a;int b;};constexpr S(int x):prefix(x),a(prefix+1),b(a+1){}};
constexpr S s(7);static_assert(s.a==8&&s.b==9,"prior projected values");int main(){return s.b==9?0:1;}''',
}
negative={
'conflicting-variants':'union S{struct{int a;int b;};long c;S():a(1),c(2){}};int main(){S s;}',
'nested-conflict':'struct S{union{int a;int b;};S():a(1),b(2){}};int main(){S s;}',
'duplicate-leaf':'struct S{struct{int a;};S():a(1),a(2){}};int main(){S s;}',
'constant-missing':'struct S{struct{int a;int b;};constexpr S():a(1){}};int main(){S s;}',
'default-parameter':'struct S{struct{int a;int b=x;};S(int x):a(x){}};int main(){S s(1);}',
'nonmember':'struct X{int x;};struct S{S():X::x(1){}};int main(){S s;}',
'missing-reference':'struct S{struct{int a;int& r;};S():a(1){}};int main(){S s;}',
'conflicting-defaults':'union S{struct{int a=1;int b=2;};int c=3;S(){}};int main(){S s;}',
}
for name,body in cases.items():
 source=out/(name+'.cpp');source.write_text(body+'\n')
 for level in ('-O0','-O2'):
  obj=out/(name+level+'.o');exe=out/(name+level)
  run([compiler,level,'-c',source,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe])
for name,body in negative.items():
 source=out/(name+'.cpp');source.write_text(body+'\n')
 run([compiler,'-c',source,'-o',out/'rejected.o'],False)
print('PA27 storage controls PASS:',len(records),'commands')
