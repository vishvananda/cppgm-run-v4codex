#!/usr/bin/env python3
"""Class result source-form matrix, inspect bundle signatures and native results."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]);W.mkdir(parents=True,exist_ok=True)
sources={
 'conversion-value':'struct A{int n;};struct X{A a;template<class T>operator T()const{return a;}};int main(){X x={{7}};A a=x;return a.n!=7;}',
 'conversion-empty':'struct A{};struct X{A a;template<class T>operator T()const{return a;}};int main(){X x={};A a=x;}',
 'conversion-wrapped':'template<class T>struct A{int n;};struct X{A<int> a;template<class T>operator A<T>()const{return a;}};int main(){X x={{7}};A<int> a=x;return a.n!=7;}',
 'conversion-fixed':'struct A{int n;};struct X{A a;template<class T=int>operator A()const{return a;}};int main(){X x={{7}};A a=x;return a.n!=7;}',
}
for result in ('A<T>','typename Id<A<T>>::type','A<typename Id<T>::type>'):
 for tag in (False,True):
  defs='template<class T>struct Id{typedef T type;};struct Tag{};template<class T>struct A{int n;template<class U>A('+('Tag,' if tag else '')+'U x):n(x){}};'
  source=defs+'template<class T>'+result+' make(int x){return A<T>('+('Tag(),' if tag else '')+'x);}int main(){A<int> a=make<int>(7);return a.n!=7;}'
  sources['result-'+str(len(sources))]=source
for name,source in list(sources.items()):
 if ' make(' in source:
  sources[name+'-pointer']=source.replace('A<int> a=make<int>(7);','A<int>(*p)(int)=make<int>;A<int> a=p(7);')
for source_path in ('general/300-friend-function-template-alias-result-definition','spec/300-dependent-result-defaulted-nontype-declaring-scope'):
 s=(ROOT/'pa18/tests'/(source_path+'.t')).read_text();name=source_path.split('/')[-1]
 sources[name]=s
 if 'friend' in name:
  sources[name+'-noalias']=s.replace('owner<non_array<Y> >','owner<Y>').replace('owner<non_array<T> >','owner<T>')
  sources[name+'-pointer']=s.replace('owner<int> two = make_owner<int>(1, 2);','owner<int> (*p)(int&&,int&&) = make_owner<int,int,int>; owner<int> two = p(1,2);')
  sources[name+'-public']=s.replace('private:','public:')
  sources[name+'-simplector']=s.replace('template <class A, class... Args>\n  owner(allocation_tag<A>, Args&&...) : argument_count(sizeof...(Args)) {}','owner(allocation_tag<int>, int,int) : argument_count(2) {}\n  owner(allocation_tag<int>, int,int,int) : argument_count(3) {}')
 else:
  sources[name+'-simple']=s.replace('typename detail::divide_result<box<Rep>, Scalar>::type','box<Scalar>')
  sources[name+'-pointer']=s.replace('units::box<double> result = input / 2.0;','units::box<double> (*p)(const units::box<long>&, double) = units::operator/<long,double>; units::box<double> result = p(input,2.0);')
sources['conversion-explicit']=sources['conversion-value'].replace('A a=x;','A a=x.operator A();')
sources['conversion-pointer']=sources['conversion-value'].replace('A a=x;','A(X::*p)()const=&X::operator A;A a=(x.*p)();')
sources['conversion-specialization']=sources['conversion-value'].replace('int main()', 'template<>X::operator A()const{return a;}int main()')
sources['conversion-template-id']=sources['conversion-value'].replace('A a=x;','A a=x.operator A<A>();')
if len(sys.argv)>2:sources={k:v for k,v in sources.items() if k.startswith(sys.argv[2])}
rows=[]
for name,source in sources.items():
 src=W/(name+'.cpp');src.write_text(source)
 ir=W/(name+'.lowir');exe=W/(name+'.exe')
 r=subprocess.run([ROOT/'reference-binaries/cppgm++','--emit-lowir','-O0','-o',ir,src],capture_output=True,text=True)
 row=dict(name=name,source=source,compiler_exit=r.returncode,diagnostic=r.stderr)
 if r.returncode==0:
  row['lowir']=ir.read_text()
  b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
  row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
  if b.returncode==0:row['native_exit']=subprocess.run([exe],timeout=10).returncode
 print(name,[x for x in row.get('lowir','').splitlines() if x.startswith('function ') and ('@make' in x or 'operator' in x)],row.get('native_exit'),r.stderr,flush=True)
 rows.append(row)
(W/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
