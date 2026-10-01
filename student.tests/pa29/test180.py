#!/usr/bin/env python3
"""Callable extensions: explicit controls, runtime, rejection and typed views."""
import pathlib,subprocess,sys,json,hashlib
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
compiler=pathlib.Path(sys.argv[2] if len(sys.argv)>2 else root/'dev/cppgm++').resolve()
cases={
'static-call':('struct F { static int operator()(int x){return x+1;} }; int main(){return F()(6)!=7;}',True),
'static-subscript':('struct F { static int operator[](int x){return x+1;} }; int main(){return F()[6]!=7;}',True),
'static-template':('struct F { template<class T> static T operator()(T x){return x+1;} }; template<class T> int call(T t){return t(8);} int main(){return call(F())!=9;}',True),
'static-default':('struct F { static int operator()(int x=9){return x;} }; int main(){return F()()!=9;}',True),
'static-effects':('int made,dead; struct F { F(){++made;} ~F(){++dead;} static int operator()(int x){return x+made;} }; int main(){int x=F()(8);return x!=9||made!=1||dead!=1;}',True),
'static-query':('struct F { static constexpr int operator()(int x=5) noexcept {return x+2;} }; template<class T> auto call(T t)->decltype(t()){return t();} static_assert(F()()==7,"value"); static_assert(noexcept(F()(1)),"noexcept"); int main(){return call(F())!=7;}',True),
'static-fixed':('struct F { static int operator()(int x=4){return x;} }; template<class T> int call(T){return F()();} int main(){return call(0)!=4;}',True),
'static-pointer':('struct F { static int operator()(int x){return x+1;} }; int main(){int(*p)(int)=&F::operator();return p(5)!=6;}',True),
'static-inherited':('struct F { static int operator()(int x){return x+1;} }; struct G:F{}; int main(){const G g;return g(5)!=6;}',True),
'static-mixed':('struct F { static int operator()(int x){return 1;} int operator()(long) const {return 2;} }; int main(){F f;return f(5)!=1||f(5L)!=2||f.operator()(5)!=1;}',True),
'static-partial':('struct F { template<class T> static int operator()(T*){return 1;} template<class T> static int operator()(T){return 2;} }; int main(){int x;return F()(&x)!=1||F()(x)!=2;}',True),
'static-reference':('struct F { static int& operator()(int& x){return x;} }; int main(){int x=3;F()(x)=7;return x!=7;}',True),
'static-private':('struct F { private: static int operator()(int){return 1;} }; int main(){return F()(1);}',False),
'static-deleted':('struct F { static int operator()(int)=delete; }; int main(){return F()(1);}',False),
'static-wrong-arity':('struct F { static int operator()(int){return 1;} }; int main(){return F()();}',False),
'static-illegal-op':('struct F { static int operator+(F){return 1;} }; int main(){return 0;}',False),
'static-ambiguous':('struct F { static int operator()(int,long){return 1;} int operator()(long,int)const{return 2;} }; int main(){return F()(1,1);}',False),
'static-this':('struct F { static F* operator()(){return this;} }; int main(){return 0;}',False),
'static-qualifier':('struct F { static int operator()() const{return 1;} }; int main(){return 0;}',False),
}
results=[]
def command(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 return dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr)
for name,(source,ok) in cases.items():
 src=out/(name+'.cpp');src.write_text(source+'\n'); row=dict(name=name,accepted=ok,sha256=hashlib.sha256(src.read_bytes()).hexdigest(),source=source,checks=[])
 for label,binary,flags in [('student',compiler,['-std=c++11']),('clang','clang++',['-std=c++11','-Wno-c++23-extensions'])]:
  obj=out/(name+label+'.o');exe=out/(name+label)
  r=command([binary,*flags,'-c',src,'-o',obj]);row['checks'].append(r)
  if (r['status']==0)!=ok:row['failure']=label+' acceptance'
  if ok and not r['status']:
   r=command(['g++',obj,'-o',exe]);row['checks'].append(r)
   if r['status']:row['failure']=label+' link'
   else:
    r=command([exe]);row['checks'].append(r)
    if r['status']:row['failure']=label+' runtime'
 if ok:
  r=command([compiler,'-c','--emit-lowir','--validate-lowir',src,'-o',out/(name+'.lowir')]);row['checks'].append(r)
  if r['status']:row['failure']='LowIR audit'
 results.append(row);(out/'controls.json').write_text(json.dumps(results,indent=2)+'\n');print(name,row.get('failure','pass'),flush=True)
assert not [r for r in results if 'failure' in r]
