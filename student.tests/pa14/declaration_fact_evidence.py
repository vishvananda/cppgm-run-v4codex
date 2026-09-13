#!/usr/bin/env python3
"""Freeze declaration-identity and initializer proofs, with live fact-store layouts."""
from pathlib import Path
import json,re,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
import special_signature_compare as comparison
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
result=dict(harness_sha256=shared.sha(__file__),
 comparison_harness_sha256=shared.sha(comparison.__file__),comparison_adapter_sha256=shared.sha(ROOT/'student.tests/pa14/special_signature_compare.pl'),comparison_sha256=shared.sha(ROOT/'scripts/compare_results_common.pl'),contract_sha256=shared.sha(ROOT/'pa8/lowir.md'),standard_path=str(ROOT/'doc/n3485.txt'),standard_sha256=shared.sha(ROOT/'doc/n3485.txt'),
 binaries=[dict(path=str(p),sha256=shared.sha(p)) for p in binaries],rejections=[],positive=[])
def run(name,command):
 command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,timeout=120)
 log=WORK/(name+'.log');log.write_text(r.stdout+r.stderr)
 row=dict(command=command,exit_code=r.returncode,log=str(log),log_sha256=shared.sha(log))
 assert 'AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr,(name,r.stderr)
 return row,r
for name,suffix in (('local-declaration-facts','.cpp'),('direct-initializer','.t')):
 src=WORK/(name+suffix);shutil.copyfile(ROOT/f'student.tests/pa14/{name}{suffix}',src)
 row=dict(name=name,source_path=str(src),source_sha256=shared.sha(src),clauses=['[dcl.ambig.res]/1','[dcl.decl]','[dcl.fct]','[temp.arg.type]/2','[basic.life]','[class.dtor]'],outputs=[])
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';exe=WORK/f'{name}-{b}'
  checked,r=run(f'{name}-{b}-compile',[binary,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,src])
  if b==0:
   assert r.returncode==1,(name,r.stderr)
   row['entry_rejection']=checked;continue
  assert r.returncode==0,(name,b,r.stderr)
  native,nr=run(f'{name}-{b}-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);assert nr.returncode==0
  executed,er=run(f'{name}-{b}-run',[exe]);assert er.returncode==0
  checked.update(path=str(ir),sha256=shared.sha(ir),telemetry=[json.loads(line) for line in r.stderr.splitlines()],
   native_path=str(exe),native_sha256=shared.sha(exe),native=native,executed=executed)
  row['outputs'].append(checked)
 if len(set(o['sha256'] for o in row['outputs']))!=1:
  row['comparison']=comparison.compare(src,row['outputs'],WORK/'comparisons',name)
 assert len(set(o['native_sha256'] for o in row['outputs']))==1
 host=WORK/(name+'-host');built,br=run(name+'-host-build',['g++','-x','c++','-std=c++11','-pedantic-errors',src,'-o',host]);assert br.returncode==0
 executed,er=run(name+'-host-run',[host]);assert er.returncode==0
 row['host']=dict(build=built,executed=executed,path=str(host),sha256=shared.sha(host))
 result['positive'].append(row);OUT.write_text(json.dumps(result,indent=2)+'\n')
declaration_baseline=Path(sys.argv[5]).resolve()
result['declaration_baseline']=dict(path=str(declaration_baseline),sha256=shared.sha(declaration_baseline),text_bytes=shared.text_size(declaration_baseline))
for row in result['positive']:
 name=row['name'];src=Path(row['source_path']);ir=WORK/(name+'-declaration.lowir');exe=WORK/(name+'-declaration')
 checked,r=run(name+'-declaration-compile',[declaration_baseline,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,src]);assert r.returncode==0,(name,r.stderr)
 native,nr=run(name+'-declaration-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);assert nr.returncode==0
 executed,er=run(name+'-declaration-run',[exe]);assert er.returncode==0
 checked.update(path=str(ir),sha256=shared.sha(ir),telemetry=[json.loads(line) for line in r.stderr.splitlines()],native_path=str(exe),native_sha256=shared.sha(exe),native=native,executed=executed)
 row['declaration_output']=checked
 if checked['sha256']!=row['outputs'][0]['sha256']:
  row['declaration_comparison']=comparison.compare(src,[checked,row['outputs'][0]],WORK/'comparisons',name+'-declaration')
 assert checked['native_sha256']==row['outputs'][0]['native_sha256'],name
OUT.write_text(json.dumps(result,indent=2)+'\n')
layout=WORK/'layout';layout.mkdir(exist_ok=True)
src=layout/'declaration_fact_layout_probe.cc'
shutil.copyfile(ROOT/'student.tests/pa14/declaration_fact_layout_probe.cc',src)
deps=shared.run(['g++','-std=c++11','-I'+str(ROOT/'dev/src'),'-MM',src]).stdout.replace('\\\n',' ').split(':',1)[1].split()
headers=[]
for dep in deps:
 path=Path(dep)
 if path.suffix!='.h':continue
 rel=path.relative_to(ROOT/'dev/src');target=layout/'include'/rel
 target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
 headers.append(dict(source=str(path),path=str(target),sha256=shared.sha(target)))
binary=layout/'probe';dump=layout/'classes.txt'
checked,r=run('layout-build',['g++','-std=c++11','-I'+str(layout/'include'),'-fdump-lang-class='+str(dump),src,'-o',binary]);assert r.returncode==0
records={}
for name in ('ExpressionStore::Properties','ExpressionStore::Use','Analyzer','Analyzer::TemplatePrototype','MemberFacts','TemplateDefinition','Fact','FactStore'):
 m=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',dump.read_text());assert m,name
 records[name]=dict(size=int(m[1]),align=int(m[2]))
result['layout']=dict(build=checked,headers=headers,source_path=str(src),source_sha256=shared.sha(src),
 binary_path=str(binary),binary_sha256=shared.sha(binary),dump_path=str(dump),dump_sha256=shared.sha(dump),records=records,
 sizes=list(map(int,shared.run([binary]).stdout.split())))
OUT.write_text(json.dumps(result,indent=2)+'\n')
print('Two entry-rejected valid declaration controls, correct intermediate/native comparisons and current fact-store layouts frozen')
