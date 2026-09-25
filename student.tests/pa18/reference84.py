#!/usr/bin/env python3
"""Independent PA11 zero-init contract repair: [--write]. No student output read."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
ENTRY='09a77fca41efaa8eefc7913bc49898e7cd81e660'
COUNTS={
 'pa18/tests/general/300-adl-overload-set-argument-deduction.ref':2,
 'pa18/tests/general/300-friend-function-template-alias-result-definition.ref':2,
 'pa18/tests/general/500-inherited-constructor-template-member-alias-pack.ref':1,
}
def sha(data):return hashlib.sha256(data).hexdigest()
rows=[]
for path,count in COUNTS.items():
 old=subprocess.check_output(['git','show',ENTRY+':'+path],cwd=ROOT).decode()
 # Each tagged one-byte temporary is the source's empty-class T() value
 # initializer. Recover its destination from the oracle, retaining all names,
 # boundaries, instructions and metadata. Neither T{} nor default-init is here.
 addresses=[]
 def add(m):
  ptr,slot=m[1],m[2]
  assert re.search(r'  slot '+re.escape(slot)+r' : obj<1x1>\n',old)
  addresses.append(dict(pointer=ptr,slot=slot,bytes=1,alignment=1))
  return m[0]+'    zeroinit 1x1 '+ptr+'\n'
 new,n=re.subn(r'^    (%\w+) = addr (\$argobj__\d+)\n',add,old,flags=re.M)
 assert n==count,(path,n,count)
 if '--write' in sys.argv:(ROOT/path).write_text(new)
 assert (ROOT/path).read_text()==new,path
 rows.append(dict(path=path,before_sha256=sha(old.encode()),after_sha256=sha(new.encode()),insertions=addresses))
manifest=dict(entry=ENTRY,bundle='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',contract='pa11/README.md: one zeroinit for an exact contiguous nonvolatile non-union value-initialized span',revisions=rows)
p=ROOT/'student.tests/pa18/reference84-revisions.json'
if '--write' in sys.argv:p.write_text(json.dumps(manifest,indent=2)+'\n')
assert json.loads(p.read_text())==manifest
print('Verified three oracle revisions: five required one-byte zero initializations; all existing bytes retained.')
