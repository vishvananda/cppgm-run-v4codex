#!/usr/bin/env python3
"""Reproduce the repeated-empty-layout oracle correction, retaining both lanes."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK=[Path(x).resolve() for x in sys.argv[1:3]];WORK.mkdir(parents=True,exist_ok=True)
ENTRY='17bc7a06';PATH='pa22/tests/general/100-public-qualified-base-typedef-ambiguous-subobject.ref'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(command):
 p=subprocess.run([str(x) for x in command],cwd=ROOT,capture_output=True,text=True,timeout=30)
 return dict(command=[str(x) for x in command],exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
def execute(ir):
 exe=WORK/(ir.stem+'.exe');r=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);row=dict(backend=r)
 if not r['exit']:row['runtime']=run([exe])
 return row
original=WORK/'original.lowir';original.write_bytes(subprocess.check_output(['git','show',ENTRY+':'+PATH],cwd=ROOT))
fixed=ROOT/PATH
result=dict(entry=ENTRY,path=PATH,manifest=(ROOT/'reference-binaries/manifest.tsv').read_text(),
            compiler_sha256=sha(CC),original_sha256=sha(original),corrected_sha256=sha(fixed),fixtures=[])
for p in (original,fixed):
 row=dict(path=str(p),validation=run([ROOT/'dev/lowir','-o',WORK/'roundtrip.lowir',p]),**execute(p))
 assert row['validation']['exit']==0 and row['runtime']['exit']==0,row
 result['fixtures'].append(row)
source='''struct impl{};template<class T>struct trampoline:impl{};
struct inner:trampoline<long>{};struct outer:inner,trampoline<int>{};
int main(){outer o;impl* a=static_cast<inner*>(&o);
impl* b=static_cast<trampoline<int>*>(&o);return a==b;}
'''
src=WORK/'reducer.cpp';src.write_text(source);result.update(source=source,source_sha256=sha(src),reducers=[])
for name,cc in [('reference',ROOT/'dev/cppgm++-ref'),('entry',WORK.parent/'entry/cppgm++'),('corrected',CC)]:
 ir=WORK/(name+'.lowir');r=run([cc,'--emit-lowir','-O0','-o',ir,src]);assert not r['exit'],r
 row=dict(lane=name,compiler_sha256=sha(cc),compile=r,lowir_sha256=sha(ir),**execute(ir))
 assert row['runtime']['exit']==(0 if name=='corrected' else 1),row
 result['reducers'].append(row)
result['passed']=True
print(json.dumps(result,indent=2))
