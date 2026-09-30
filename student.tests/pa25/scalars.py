#!/usr/bin/env python3
"""Explicit PA25 scalar correctness controls. Python integers are the oracle."""
import json, os, pathlib, random, subprocess, sys
root=pathlib.Path(__file__).resolve().parents[2]
art=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else os.environ.get('RALPH_ARTIFACT_DIR','/tmp'))/'pa25-scalars'
art.mkdir(parents=True,exist_ok=True)
compiler=root/'dev/cppgm++'; checks=[]
def run(args,ok=True):
 r=subprocess.run(list(map(str,args)),capture_output=True,timeout=45)
 if ok and r.returncode: raise RuntimeError((args,r.returncode,r.stderr.decode()))
 return r
def program(name,text):
 src=art/(name+'.cc');src.write_text(text)
 exe=art/name
 run([compiler,'-o',exe,src]);run([exe]);checks.append(name)
 obj=art/(name+'.obj');run([compiler,'-c','-o',obj,src]);run([compiler,'-o',exe,obj]);run([exe]);checks.append(name+'-object')
for name in ['wide','wide-fields','floating-builtins']:
 program(name,(root/'student.tests/pa25'/f'{name}.cc').read_text())
mask=(1<<128)-1
literal=lambda n:f'((U({(n&mask)>>64}ULL)<<64)|U({n&((1<<64)-1)}ULL))'
rng=random.Random(250136)
rows=[]
for k in range(96):
 a=rng.getrandbits(128);b=rng.getrandbits(128) or 1; shift=rng.randrange(128)
 signed_a=a-(1<<128) if a>>127 else a
 signed_b=b-(1<<128) if b>>127 else b
 quo=abs(signed_a)//abs(signed_b)*(-1 if (signed_a<0)!=(signed_b<0) else 1)
 rows.append('{' + ','.join(literal(v) for v in [a,b,(a+b)&mask,(a-b)&mask,(a*b)&mask,a//b,a%b,(a<<shift)&mask,a>>shift,signed_a>>shift,quo,signed_a-quo*signed_b]) +f',{shift},'+str(int(signed_a<signed_b))+'}')
program('random-wide','''using U=unsigned __int128;using I=__int128;
struct Case{U a,b,add,sub,mul,div,mod,left,right,aright,sdiv,smod;int count,less;};
Case cases[]={'''+','.join(rows)+'''};
U add(U a,U b){return a+b;}U sub(U a,U b){return a-b;}U mul(U a,U b){return a*b;}
int main(int argc,char**){for(int i=0;i<argc*96;++i){Case &c=cases[i];
if(add(c.a,c.b)!=c.add || sub(c.a,c.b)!=c.sub || mul(c.a,c.b)!=c.mul) return 1;
if(c.a/c.b!=c.div || c.a%c.b!=c.mod) return 2;
if((c.a<<c.count)!=c.left || (c.a>>c.count)!=c.right || U(I(c.a)>>c.count)!=c.aright) return 3;
if(U(I(c.a)/I(c.b))!=c.sdiv || U(I(c.a)%I(c.b))!=c.smod) return 4;
if((I(c.a)<I(c.b))!=bool(c.less)) return 5;
if((I(c.a)<c.b)!=(c.a<c.b) || (c.b<I(c.a))!=(c.b<c.a))return 6;
}return 0;}''')
for n,expr in enumerate(['(I(1)<<126)+(I(1)<<126)','(I(1)<<127)-1','(I(1)<<126)*4','(I(1)<<127)/-1','-(I(1)<<127)','I(1)<<128','I(-1)<<3','I(1)/0']):
 src=art/f'invalid-{n}.cc';src.write_text('using I=__int128;constexpr I x='+expr+';int main(){return 0;}')
 r=run([compiler,'-c','-o',art/'invalid.obj',src],False)
 assert r.returncode,(src,'accepted invalid constant')
 checks.append(f'invalid-{n}')
# Full-width template identity and ABI values must survive separate compilation.
src1=art/'template1.cc';src2=art/'template2.cc'
common='using U=unsigned __int128;template<U N> U value(){return N;}\n'
src1.write_text(common+'template U value<(U(1)<<100)+3>();U other(){return value<(U(1)<<100)+3>();}')
src2.write_text(common+'extern template U value<(U(1)<<100)+3>();U other();int main(){return other()==value<(U(1)<<100)+3>()?0:1;}')
obj=art/'template.obj';run([compiler,'-c','-o',obj,src1]);run([compiler,'-o',art/'templates',obj,src2]);run([art/'templates']);checks.append('cross-tu-wide-template')
# Compare the public Itanium symbol with the host ABI as a validation-only oracle.
host=run(['g++','-std=c++11','-c','-o',art/'host.o',src1])
names=run(['nm',art/'host.o']).stdout.decode()
run([compiler,'--emit-lowir','-o',art/'template.lowir',src1])
lowir=(art/'template.lowir').read_text()
expected='_Z5valueILo1267650600228229401496703205379EEov'
assert expected in names and expected in lowir
checks.append('wide-template-itanium')
# Standalone normalized-fact adapter shares the production full-width encoder.
facts=art/'wide.facts'
facts.write_text('let-arg N value uint128 1267650600228229401496703205379\ntype template Box N\n')
run([root/'dev/abimangle','-o',art/'wide.mangled',facts])
assert (art/'wide.mangled').read_text().strip().endswith('3BoxILo1267650600228229401496703205379EE')
checks.append('wide-abi-facts')
(art/'results.json').write_text(json.dumps({'checks':checks,'count':len(checks)},indent=2)+'\n')
print(f'{len(checks)} scalar controls passed; 96 randomized full-width operand pairs')
