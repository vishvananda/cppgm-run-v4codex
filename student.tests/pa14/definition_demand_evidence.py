#!/usr/bin/env python3
"""Freeze reduced language proofs, selected-definition outputs and live layouts."""
from pathlib import Path
import json,re,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
import check_definition_demands as controls
A,B,WORK,OUT=map(Path,sys.argv[1:5]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()]
result=dict(harness_sha256=shared.sha(__file__),control_sha256=shared.sha(controls.__file__),
 standard_path=str(ROOT/'doc/n3485.txt'),standard_sha256=shared.sha(ROOT/'doc/n3485.txt'),
 binaries=[dict(path=str(p),sha256=shared.sha(p)) for p in binaries],rejections=[],positive=[])
def run(name,command):
 command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,timeout=120)
 log=WORK/(name+'.log');log.write_text(r.stdout+r.stderr)
 row=dict(command=command,exit_code=r.returncode,log=str(log),log_sha256=shared.sha(log))
 assert 'AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr,(name,r.stderr)
 return row,r
inputs={name:controls.source(*args) for name,args in controls.CASES.items()};inputs.update(controls.EXTRA_CASES)
for name,source in inputs.items():
 src=WORK/(name+'.cpp');src.write_text(source)
 clauses=['[class.mem]/1','[class.mfct]/2']
 if name=='noexcept':clauses=['[except.spec]/3–4']
 if name=='friend_not_member':clauses+=['[class.mfct]/1']
 if name in ('redefinition','inline_redefinition'):clauses+=['[basic.def.odr]/1']
 row=dict(name=name,source_path=str(src),source_sha256=shared.sha(src),clauses=clauses,outputs=[])
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir'
  checked,r=run(f'{name}-{b}',[binary,'--emit-lowir','-O0','-o',ir,src])
  assert r.returncode==(0 if b==0 else 1),(name,b,r.stderr)
  if r.returncode==0:checked.update(path=str(ir),sha256=shared.sha(ir))
  row['outputs'].append(checked)
 host,r=run(name+'-host',['g++','-std=c++11','-pedantic-errors','-fsyntax-only',src])
 assert r.returncode==1,(name,r.stderr)
 row['host']=host;result['rejections'].append(row)
 OUT.write_text(json.dumps(result,indent=2)+'\n')
for name in ('definition-demands','definition-signatures','definition-overloads','definition-parameters'):
 src=WORK/(name+'.cpp');shutil.copyfile(ROOT/f'student.tests/pa14/{name}.cpp',src)
 row=dict(name=name,source_path=str(src),source_sha256=shared.sha(src),outputs=[])
 for b,binary in enumerate(binaries):
  ir=WORK/f'{name}-{b}.lowir';exe=WORK/f'{name}-{b}'
  checked,r=run(f'{name}-{b}-compile',[binary,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,src])
  assert r.returncode==0,(name,b,r.stderr)
  native,nr=run(f'{name}-{b}-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);assert nr.returncode==0
  executed,er=run(f'{name}-{b}-run',[exe]);assert er.returncode==0
  checked.update(path=str(ir),sha256=shared.sha(ir),telemetry=[json.loads(line) for line in r.stderr.splitlines()],
   native_path=str(exe),native_sha256=shared.sha(exe),native=native,executed=executed)
  row['outputs'].append(checked)
 assert len(set(o['sha256'] for o in row['outputs']))==len(set(o['native_sha256'] for o in row['outputs']))==1
 result['positive'].append(row);OUT.write_text(json.dumps(result,indent=2)+'\n')
layout=WORK/'layout';layout.mkdir(exist_ok=True)
src=layout/'definition_demand_layout_probe.cc'
shutil.copyfile(ROOT/'student.tests/pa14/definition_demand_layout_probe.cc',src)
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
for name in ('ExpressionStore::Properties','ExpressionStore::Use','Analyzer','Analyzer::TemplatePrototype','MemberFacts','TemplateDefinition'):
 m=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',dump.read_text());assert m,name
 records[name]=dict(size=int(m[1]),align=int(m[2]))
result['layout']=dict(build=checked,headers=headers,source_path=str(src),source_sha256=shared.sha(src),
 binary_path=str(binary),binary_sha256=shared.sha(binary),dump_path=str(dump),dump_sha256=shared.sha(dump),records=records,
 sizes=list(map(int,shared.run([binary]).stdout.split())))
OUT.write_text(json.dumps(result,indent=2)+'\n')
print('12 entry-accepted invalid definitions rejected; four exact LowIR/native controls; current transitive layout frozen')
