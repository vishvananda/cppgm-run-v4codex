#!/usr/bin/env python3
"""Reproduce and verify two standards-proved closure reference corrections.
Run --apply to install the documented oracle bytes, or WORK CC to verify/execute.
No implementation output is used to construct reference LowIR.
"""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='e75e0d6ce1e26a2d50bc0857cd620974c6ee451d'
def sha(b):return hashlib.sha256(b).hexdigest()
def run(args):return subprocess.run([str(x) for x in args],cwd=ROOT,capture_output=True,text=True,timeout=60)
replacements={
 'pa20/tests/general/200-captureless-lambda-wrapper-conversion.ref.exit_status':b'EXIT_FAILURE\n',
 'pa20/tests/general/200-lambda-constructor-template-preferred.ref':(ROOT/'student.tests/pa20/constructor_closure98.lowir').read_bytes(),
}
rows=[]
for path,after in replacements.items():
 before=subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT)
 source=path.split('.ref')[0]+'.t'
 rows.append(dict(path=path,before_sha256=sha(before),after_sha256=sha(after),source_sha256=sha((ROOT/source).read_bytes())))
 if '--apply' in sys.argv:(ROOT/path).write_bytes(after)
 else:assert (ROOT/path).read_bytes()==after,path
manifest=dict(entry=ENTRY,bundle_revision='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',local_revision='pa20-closure-conversions-98',proof='pa20/reference-corrections98.md',revisions=rows)
p=ROOT/'student.tests/pa20/reference98-revisions.json'
if '--apply' in sys.argv:p.write_text(json.dumps(manifest,indent=2)+'\n')
else:
 assert json.loads(p.read_text())==manifest
 work,cc=map(lambda x:Path(x).resolve(),sys.argv[1:]);work.mkdir(parents=True,exist_ok=True)
 checks=[]
 for name,status in [('wrapper_conversion98',1),('constructor_closure98',0)]:
  src=ROOT/'student.tests/pa20'/(name+'.cpp');ir=work/(name+'.lowir')
  r=run([cc,'--emit-lowir','-O0','--validate-lowir','-o',ir,src]);assert r.returncode==status,(name,r.stderr)
  checks.append(dict(name=name,expected=status,exit=r.returncode,diagnostic=r.stderr,source_sha256=sha(src.read_bytes())))
  if not status:
   exe=work/name;b=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);assert not b.returncode,b.stderr
   e=run([exe]);assert not e.returncode;checks[-1]['native_exit']=e.returncode
 path='pa20/tests/general/200-lambda-constructor-template-preferred.ref'
 for label,content in [('original',subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT)),('corrected',(ROOT/path).read_bytes())]:
  ir=work/(label+'.lowir');exe=work/label;ir.write_bytes(content)
  b=run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);assert not b.returncode,b.stderr
  e=run([exe]);assert not e.returncode
  checks.append(dict(name=label,lowir_sha256=sha(content),native_exit=e.returncode))
 after=replacements[path].decode();assert '%f : obj<1x1>' in after and 'call i32 @lambda_call(%closure, 6)' in after
 assert '_ZN4SinkC1IZ4mainvEUliE_EET_' in after and 'IPFiiE' not in after
 (work/'results.json').write_text(json.dumps(dict(manifest=manifest,checks=checks),indent=2)+'\n')
print('Verified two closure conversion corrections.')
