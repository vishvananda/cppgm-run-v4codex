#!/usr/bin/env python3
"""Reproduce the standalone RTTI scan limit with reference and student LowIR."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2];WORK=Path(sys.argv[1]).resolve();WORK.mkdir(parents=True,exist_ok=True)
source=ROOT/'student.tests/pa23/controls123-layout.json';case=next(r for r in json.loads(source.read_text())['cases'] if r['name']=='shared-rtti')
src=WORK/'shared-rtti.cpp';src.write_text(case['source']);rows=[]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def call(args):
 p=subprocess.run([str(x) for x in args],capture_output=True,text=True,timeout=30);return dict(command=[str(x) for x in args],exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
for label,cc in [('reference',ROOT/'dev/cppgm++-ref'),('student',ROOT/'dev/cppgm++')]:
 ir=WORK/(label+'.lowir');exe=WORK/(label+'.exe');row=dict(label=label)
 row['compile']=call([cc,'--emit-lowir','-O0','-o',ir,src]);assert row['compile']['exit']==0
 row['lowir_sha256']=sha(ir);assert re.search(r'call ptr @[^\n]+, -1\)',ir.read_text())
 row['standalone']=call([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir])
 if not row['standalone']['exit']:row['standalone_run']=call([exe])
 if label=='reference':
  private=WORK/'reference.private.lowir';count=[0]
  def rename(m):count[0]+=1;return 'object=__private_rtti123_'+str(count[0])
  private.write_text(re.sub(r'object=@[^,\]\s]+',rename,ir.read_text()))
  row['private_object_renames']=count[0];row['private_lowir_sha256']=sha(private)
  row['private_standalone']=call([ROOT/'dev/lowir2native-ref','-O0','-o',exe,private]);assert row['private_standalone']['exit']==0
  row['private_run']=call([exe]);assert row['private_run']['exit']==1
 else:assert row['standalone']['exit']==0 and row['standalone_run']['exit']==1
 obj=WORK/(label+'.o');exe=WORK/(label+'.host')
 row['hosted_backend']=call([ROOT/'dev/cppgm++-ref','-c','-O0','-o',obj,ir]);assert row['hosted_backend']['exit']==0
 row['hosted_link']=call(['g++','-no-pie',obj,'-o',exe]);assert row['hosted_link']['exit']==0
 row['hosted_run']=call([exe]);assert row['hosted_run']['exit']==0
 rows.append(row)
print(json.dumps(dict(bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',source=src.read_text(),source_sha256=sha(src),expected_source_exit=0,cases=rows,oracle_changes=[]),indent=2))
