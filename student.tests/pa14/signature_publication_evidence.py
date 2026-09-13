#!/usr/bin/env python3
"""Freeze signature/class-use/enum proofs and current transitive layouts."""
from pathlib import Path
import json,re,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'student.tests/pa10'))
import benchmark as shared
import special_signature_compare as comparison
from check_signature_publications import CASES,CLAUSES,OPTIONAL
A,B,WORK,OUT,BASELINE=map(Path,sys.argv[1:6]);WORK.mkdir(parents=True,exist_ok=True)
binaries=[A.resolve(),B.resolve()];BASELINE=BASELINE.resolve()
result=dict(harness_sha256=shared.sha(__file__),control_sha256=shared.sha(ROOT/'student.tests/pa14/check_signature_publications.py'),
 comparison_harness_sha256=shared.sha(comparison.__file__),comparison_adapter_sha256=shared.sha(ROOT/'student.tests/pa14/special_signature_compare.pl'),comparison_sha256=shared.sha(ROOT/'scripts/compare_results_common.pl'),contract_sha256=shared.sha(ROOT/'pa8/lowir.md'),standard_path=str(ROOT/'doc/n3485.txt'),standard_sha256=shared.sha(ROOT/'doc/n3485.txt'),
 binaries=[dict(path=str(p),sha256=shared.sha(p),text_bytes=shared.text_size(p)) for p in binaries],declaration_baseline=dict(path=str(BASELINE),sha256=shared.sha(BASELINE),text_bytes=shared.text_size(BASELINE)),rejections=[],positive=[])
def run(name,command):
 command=list(map(str,command));p=subprocess.run(command,capture_output=True,text=True,timeout=120)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 row=dict(command=command,exit_code=p.returncode,log=str(log),log_sha256=shared.sha(log))
 assert not any(x in p.stderr for x in ['AddressSanitizer','UndefinedBehaviorSanitizer','runtime error:']),(name,p.stderr)
 return row,p
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
def native(name,src,binary,expected=0):
 ir=WORK/(name+'.lowir');exe=WORK/name
 checked,p=run(name+'-compile',[binary,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,src])
 assert p.returncode==expected,(name,p.stderr)
 if expected:return checked
 built,b=run(name+'-native',[ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);assert b.returncode==0,(name,b.stderr)
 executed,e=run(name+'-run',[exe]);assert e.returncode==0,(name,e.returncode)
 checked.update(path=str(ir),sha256=shared.sha(ir),telemetry=[json.loads(line) for line in p.stderr.splitlines()],native_path=str(exe),native_sha256=shared.sha(exe),native=built,executed=executed)
 return checked
for name,suffix,entry in [('signature-publications','.cpp',1),('enum-signatures','.cpp',0),('signature-source-identity','.t',0),('signature-query-reducer','.t',0),('signature-late-default','.t',1),('signature-late-initializer','.t',1)]:
 src=WORK/(name+suffix);shutil.copyfile(ROOT/f'student.tests/pa14/{name}{suffix}',src)
 row=dict(name=name,source_path=str(src),source_sha256=shared.sha(src),clauses=['[dcl.fct]/5','[dcl.type.simple]','[basic.scope.proto]','[basic.scope.class]/1','[class.mem]/2','[temp.inst]/1','[dcl.enum]','[temp.mem]'])
 row['entry_rejection' if entry else 'entry_output']=native(name+'-entry',src,binaries[0],entry)
 row['output']=native(name+'-final',src,binaries[1]);row['declaration_output']=native(name+'-declaration',src,BASELINE)
 for field in ['declaration_output']+([] if entry else ['entry_output']):
  outputs=[row[field],row['output']]
  if outputs[0]['sha256']!=outputs[1]['sha256']:row[field+'_comparison']=comparison.compare(src,outputs,WORK/'comparisons',name+'-'+field)
  assert outputs[0]['native_sha256']==outputs[1]['native_sha256'],(name,field)
 host=WORK/(name+'-host');built,p=run(name+'-host-build',['g++','-x','c++','-std=c++11','-pedantic-errors',src,'-o',host]);assert p.returncode==0,(name,p.stderr)
 executed,p=run(name+'-host-run',[host]);assert p.returncode==0,(name,p.returncode)
 row['host']=dict(build=built,executed=executed,path=str(host),sha256=shared.sha(host));result['positive'].append(row);save()
for name,source in CASES.items():
 src=WORK/(name+'.cpp');src.write_text(source)
 row=dict(name=name,source_path=str(src),source_sha256=shared.sha(src),clauses=CLAUSES[name],optional=name in OPTIONAL,outputs=[])
 for b,binary in enumerate(binaries):
  out,p=run(f'{name}-{b}',[binary,'--emit-lowir','-O0','-o',WORK/f'{name}-{b}.lowir',src]);assert p.returncode in ((0,1) if name in OPTIONAL else (1,)),(name,b,p.stderr);row['outputs'].append(out)
 row['host'],p=run(name+'-host',['g++','-x','c++','-std=c++11','-pedantic-errors','-fsyntax-only',src]);assert p.returncode in ((0,1) if name in OPTIONAL else (1,)),(name,p.stderr)
 result['rejections'].append(row);save()
layout=WORK/'layout';layout.mkdir(exist_ok=True);src=layout/'signature_publication_layout_probe.cc'
shutil.copyfile(ROOT/'student.tests/pa14/signature_publication_layout_probe.cc',src)
deps=shared.run(['g++','-std=c++11','-I'+str(ROOT/'dev/src'),'-MM',src]).stdout.replace('\\\n',' ').split(':',1)[1].split();headers=[]
for dep in deps:
 path=Path(dep)
 if path.suffix!='.h':continue
 rel=path.relative_to(ROOT/'dev/src');target=layout/'include'/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(path,target)
 headers.append(dict(source=str(path),path=str(target),sha256=shared.sha(target)))
binary=layout/'probe';dump=layout/'classes.txt'
checked,p=run('layout-build',['g++','-std=c++11','-I'+str(layout/'include'),'-fdump-lang-class='+str(dump),src,'-o',binary]);assert p.returncode==0,p.stderr
records={}
for name in ['ExpressionStore::Properties','ExpressionStore::Use','Analyzer','Analyzer::TemplatePrototype','Analyzer::TemplateClassUse','MemberFacts','TemplateDefinition','Fact','FactStore']:
 match=re.search(r'Class cppgm::semantic::'+name+r'\n\s*size=(\d+) align=(\d+)',dump.read_text());assert match,name
 records[name]=dict(size=int(match[1]),align=int(match[2]))
result['layout']=dict(build=checked,headers=headers,source_path=str(src),source_sha256=shared.sha(src),binary_path=str(binary),binary_sha256=shared.sha(binary),dump_path=str(dump),dump_sha256=shared.sha(dump),records=records,sizes=list(map(int,shared.run([binary]).stdout.split())))
save();print('Six signature/class-use/enum native proofs, nine required rejections, one optional diagnostic and current transitive layouts frozen')
