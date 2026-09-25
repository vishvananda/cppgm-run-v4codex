#!/usr/bin/env python3
"""Execute boundary reducers and reconstructed oracles: WORK OUTPUT_JSON."""
from pathlib import Path
import hashlib,json,subprocess,sys
from reference87 import CASES,ENTRY
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
sources={
 'alias-pointer':'template<class T>struct Id{typedef T type;};template<class T>struct A{int n;A(int x):n(x){}};template<class T>typename Id<A<T>>::type make(int x){return A<T>(x);}int main(){A<int>(*p)(int)=make<int>;A<int>a=p(7);return a.n!=7;}',
 'conversion-implicit':'struct A{int n;};struct X{A a;template<class T>operator T()const{return a;}};int main(){X x={{7}};A a=x;return a.n!=7;}',
}
sources['alias-direct-spelling']=sources['alias-pointer'].replace('typename Id<A<T>>::type make','A<T> make')
sources['conversion-explicit']=sources['conversion-implicit'].replace('A a=x;','A a=x.operator A();')
for path in CASES:
 source=(ROOT/path.replace('.ref','.t')).read_text();name=Path(path).stem
 sources[name]=source
 if 'friend' in name:sources['friend-pointer']=source.replace('owner<int> two = make_owner<int>(1, 2);','owner<int> (*p)(int&&,int&&) = make_owner<int,int,int>; owner<int> two = p(1,2);')
 if 'defaulted' in name:sources['defaulted-pointer']=source.replace('units::box<double> result = input / 2.0;','units::box<double> (*p)(const units::box<long>&, double) = units::operator/<long,double>; units::box<double> result = p(input,2.0);')
rows=[]
def execute(name,ir):
 exe=W/(name+'.exe')
 v=subprocess.run([ROOT/'dev/lowir','-o',W/(name+'.roundtrip'),ir],capture_output=True,text=True)
 b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
 native=subprocess.run([exe],timeout=10).returncode if b.returncode==0 else None
 return dict(validation_exit=v.returncode,backend_exit=b.returncode,backend_diagnostic=b.stderr,native_exit=native)
for name,source in sources.items():
 src=W/(name+'.cpp');src.write_text(source)
 for label,cc in [('student',ROOT/'dev/cppgm++'),('reference',ROOT/'reference-binaries/cppgm++')]:
  ir=W/(name+'-'+label+'.lowir')
  r=subprocess.run([cc,'--emit-lowir','-O0','-o',ir,src],capture_output=True,text=True)
  row=dict(name=name,label=label,source=source,source_sha256=sha(src),compiler_sha256=sha(cc),compiler_exit=r.returncode,diagnostic=r.stderr)
  if r.returncode==0:row.update(lowir=ir.read_text(),lowir_sha256=sha(ir),**execute(name+'-'+label,ir))
  expected=(1 if name=='friend-pointer' else -11) if label=='reference' and name in ('alias-pointer','friend-pointer','defaulted-pointer') else 0
  row['expected_native']=expected
  row['passed']=r.returncode==1 if label=='reference' and name=='conversion-explicit' else r.returncode==0 and row['validation_exit']==0 and row['backend_exit']==0 and row['native_exit']==expected
  rows.append(row);print(name,label,row['passed'],flush=True)
for path in CASES:
 ir=ROOT/path;name=ir.stem
 row=dict(name=name+'-revised',oracle_sha256=sha(ir),**execute(name+'-revised',ir))
 row['passed']=row['validation_exit']==0 and row['backend_exit']==0 and row['native_exit']==0;rows.append(row)
 before=W/(name+'-entry.lowir');before.write_bytes(subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT))
 row=dict(name=name+'-entry-oracle',oracle_sha256=sha(before),**execute(name+'-entry',before))
 row['passed']=row['validation_exit']==0 and row['backend_exit']==0 and row['native_exit']==0;rows.append(row)
result=dict(bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),observations=rows)
Path(sys.argv[2]).write_text(json.dumps(result,indent=2)+'\n')
assert all(r['passed'] for r in rows),[r for r in rows if not r['passed']]
print('PASS:',len(rows),'boundary observations and oracle executions')
