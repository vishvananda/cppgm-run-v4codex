#!/usr/bin/env python3
"""Reconstruct PA21 contract/region corrections without reading student output."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
BASE='9457e2fca6bdb876eaee509b0481f631b13587b4'
ARRAY='pa21/tests/general/100-function-template-local-class-specialization-identity.ref'
HANDLER='pa21/tests/general/100-source-nested-catch-miss-cleans-active-handler.ref'
changes={ARRAY:[
 ('function @cppgm_call_terminate(','''global @__integers_image [binding=internal, storage=readonly] = {
  i32 0
  i32 0
}
global @__longs_image [binding=internal, storage=readonly] = {
  i64 0
  i64 0
}

function @cppgm_call_terminate('''),
 ('''    store i32 0, %t1
    %t2 = index i8 %t1, 4
    store i32 0, %t2''','''    copyobj 8x4 @__integers_image, %t1'''),
 ('''    store i64 0, %t3
    %t4 = index i8 %t3, 8
    store i64 0, %t4''','''    copyobj 16x8 @__longs_image, %t3''')],
 HANDLER:[
 ('''  block ^catch_cleanup_15:
    eh_catch @__external_rtti__long_int, 3
    call void @__external_runtime____cxa_end_catch()
    %t13 = addr $guard
    call void @Guard___Guard(%t13)
    eh_end
    call void @__external_runtime____cxa_end_catch()
    eh_end
    jump ^catch_entry_2''',
 '''  block ^catch_cleanup_15:
    eh_catch @__external_rtti__long_int, 3
    call void @__external_runtime____cxa_end_catch()
    eh_end
    %t13 = addr $guard
    call void @Guard___Guard(%t13)
    call void @__external_runtime____cxa_end_catch()
    eh_end
    eh_end
    jump ^catch_entry_2''')]
}
sha=lambda s:hashlib.sha256(s.encode()).hexdigest()
def reconstruct(path):
 original=subprocess.check_output(['git','show',BASE+':'+path],cwd=ROOT,text=True)
 revised=original
 for old,new in changes[path]:
  assert revised.count(old)==1,(path,old)
  revised=revised.replace(old,new)
 return original,revised
if __name__=='__main__':
 manifest=dict(revision='pa21-array-contract-raw-handler-112',base=BASE,
  bundle_source='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
  bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',
  proof='pa21/reference-corrections112.md',files=[])
 for path in changes:
  original,revised=reconstruct(path)
  manifest['files'].append(dict(path=path,original_sha256=sha(original),revised_sha256=sha(revised),replacements=changes[path]))
  if '--write' in sys.argv:(ROOT/path).write_text(revised)
  else:assert (ROOT/path).read_text()==revised
 output=ROOT/'student.tests/pa21/reference112-revision.json'
 if '--write' in sys.argv:output.write_text(json.dumps(manifest,indent=2)+'\n')
 else:assert json.loads(output.read_text())==json.loads(json.dumps(manifest))
 print(json.dumps(manifest,indent=2))
