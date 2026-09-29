#!/usr/bin/env python3
"""Reconstruct only the proved nested-handler corrections from the stage base."""
from pathlib import Path
import hashlib, json, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
BASE = 'ac988ea33d4997b44e82baaca5a86623fff3127a'
changes = {
 '200-source-catch-miss-cleans-outer-scope': [
  ('    eh_catch @__external_rtti__long_int, 1\n    jump ^catch_entry_5',
   '    eh_catch @__external_rtti__long_int, 1\n    eh_cleanup\n    eh_catch @__external_rtti__int, 2\n    eh_end\n    jump ^catch_entry_5'),
  ('  block ^catch_next_8:\n    %t9 = addr $guard\n    call void @Guard___Guard(%t9)\n    jump ^catch_entry_2',
   '  block ^catch_next_8:\n    %t9 = addr $guard\n    call void @Guard___Guard(%t9)\n    eh_end\n    jump ^catch_entry_2'),
 ],
 '100-source-nested-catch-miss-cleans-active-handler': [
  ('    eh_catch @__external_rtti__long_int, 3\n    jump ^catch_entry_11',
   '    eh_catch @__external_rtti__long_int, 3\n    eh_end\n    jump ^catch_entry_11'),
  ('  block ^catch_next_14:\n    %t14 = addr $guard\n    call void @Guard___Guard(%t14)\n    call void @__external_runtime____cxa_end_catch()\n    eh_end\n    jump ^catch_entry_2',
   '  block ^catch_next_14:\n    %t14 = addr $guard\n    call void @Guard___Guard(%t14)\n    call void @__external_runtime____cxa_end_catch()\n    eh_end\n    eh_end\n    jump ^catch_entry_2'),
 ],
}
sha = lambda s: hashlib.sha256(s.encode()).hexdigest()
manifest = dict(revision='pa21-nested-catch-106',base=BASE,
    bundle_source='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
    bundle_sha256='c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7',files=[])
for name, replacements in changes.items():
    path = 'pa21/tests/general/'+name+'.ref'
    original = subprocess.check_output(['git','show',BASE+':'+path],cwd=ROOT,text=True)
    revised = original
    for old, new in replacements:
        assert revised.count(old)==1
        revised = revised.replace(old,new)
    record = dict(path=path,original_sha256=sha(original),revised_sha256=sha(revised),replacements=replacements)
    manifest['files'].append(record)
    if '--write' in sys.argv: (ROOT/path).write_text(revised)
    else: assert (ROOT/path).read_text()==revised
path = ROOT/'student.tests/pa21/reference106-revision.json'
if '--write' in sys.argv: path.write_text(json.dumps(manifest,indent=2)+'\n')
else: assert json.loads(path.read_text())==json.loads(json.dumps(manifest))
print(json.dumps(manifest,indent=2))
