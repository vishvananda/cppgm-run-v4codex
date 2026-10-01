#!/usr/bin/env python3
"""Symbol, demand, mixed-host and LowIR adapter checks."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';source=root/'student.tests/pa29/source176';rows=[]
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 r=dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr);rows.append(r)
 assert not p.returncode,r
 return p.stdout
for stem in ['exclusion','exclusion-late','exclusion-direct','inline-demand','inline-constexpr-demand','inline-shared','inline-exclusion']:
 obj=out/(stem+'.o');run([cc,'-O0','-std=c++11','--stats','-c',source/(stem+'.cpp'),'-o',obj]);nm=run(['nm','-C',obj])
 plain=out/(stem+'.plain.o');run([cc,'-O0','-std=c++11','-c',source/(stem+'.cpp'),'-o',plain]);assert obj.read_bytes()==plain.read_bytes()
 if stem=='exclusion':
  assert ' W Excluded<int>::used() const' in nm and ' W Excluded<int>::Nested::call() const' in nm
  assert 'Excluded<long>::used' not in nm and 'Excluded<long>::ordinary() const' in nm
  assert 'unused()' not in nm and ' W Excluded<int>::data' not in nm  # Data symbols use V.
 if stem=='exclusion-late':assert 'Late<long>::dormant' not in nm and ' W Late<int>::used()' in nm
 if stem=='inline-demand':assert 'Lazy<int>::dormant' not in nm and 'Lazy<int>::selected' in nm
 if stem=='inline-constexpr-demand':assert 'Constants<int>' not in nm
 if stem=='inline-shared':
  assert ' V scalar' in nm and ' V letters' in nm and ' V guard variable for dynamic' in nm
  run(['readelf','--section-groups',obj]);run(['readelf','-r',obj])
 if stem=='inline-exclusion':
  assert ' U Members<int>::supplied' in nm and 'Members<int>::used' in nm and 'dormant' not in nm
# Host interfaces consume the exact same object identities and guard ABI.
for stem in ['inline-shared','inline-temporaries']:
 for label,front,back in [('student-host',cc,'g++'),('host-student','g++',cc)]:
  a=out/(stem+label+'-a.o');b=out/(stem+label+'-b.o');exe=out/(stem+label)
  run([front,'-std=c++17','-O0','-c',source/(stem+'.cpp'),'-o',a])
  run([back,'-std=c++17','-O0',*(['-x','c++'] if back=='g++' else []),'-c',source/(stem+'.peer'),'-o',b])
  run(['g++',a,b,'-o',exe]);run([exe])
# Retained question: implementation agrees with GCC and disagrees with Clang.
for label,front in [('student',cc),('gcc','g++'),('clang','clang++')]:
 obj=out/(label+'-tag.o');run([front,'-std=c++11','-c',source/'tag-review.cpp','-o',obj]);run(['nm',obj])
names=out/'support-names.txt'
run([root/'dev/abimangle','-o',names,source/'support-names.facts'])
assert names.read_text().splitlines()==['_ZGVN2ns5valueE','_ZGRN2ns5valueE_','_ZGRN2ns5valueE0_','_ZGRN2ns5valueE10_','_ZGVN2ns5valueE0_']
ir=out/'inline.lowir'
run([cc,'--emit-lowir',source/'inline-redeclarations.cpp','-o',ir])
run([root/'dev/lowir',ir,'-o',out/'inline.roundtrip'])
(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
print('inspection passed',len(rows),flush=True)
