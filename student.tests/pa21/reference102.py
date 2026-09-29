#!/usr/bin/env python3
"""Reconstruct the single RTTI name correction from the untouched stage base."""
from pathlib import Path
import hashlib, json, re, subprocess, sys
ROOT = Path(__file__).resolve().parents[2]
BASE = 'ac988ea33d4997b44e82baaca5a86623fff3127a'
PATH = 'pa21/tests/general/100-typeid-template-template-argument-typeinfo-name.ref'
old = subprocess.check_output(['git','show',BASE+':'+PATH],cwd=ROOT).decode()
block = re.search(r'(global @\S+ \[binding=weak, object=_ZTS([^\]]+)\] = \{\n)(.*?)(\n\})',old,re.S)
assert block
name = block[2]
data = bytes(int(v) for v in re.findall(r'^  i8 (\d+)$',block[3],re.M))
assert data == name.encode()+b'\0'
assert name.count('ENS5_IhJEEE') == 1
fixed = name.replace('ENS5_IhJEEE','ENS4_IhJEEE')
fixed_data = '\n'.join('  i8 '+str(b) for b in fixed.encode()+b'\0')
revised = old[:block.start(3)]+fixed_data+old[block.end(3):]
revised = revised.replace('_ZTS'+name,'_ZTS'+fixed).replace('_ZTI'+name,'_ZTI'+fixed)
manifest = dict(revision='pa21-rtti-name-102',base=BASE,path=PATH,
    bundle_source='c2f713cd70d06170632bfde3e75dd6fe1aa44d98',
    original_sha256=hashlib.sha256(old.encode()).hexdigest(),
    revised_sha256=hashlib.sha256(revised.encode()).hexdigest(),
    original_name=name,revised_name=fixed,
    changed_bytes=[dict(offset=i,old=a,new=b) for i,(a,b) in enumerate(zip(data,fixed.encode()+b'\0')) if a!=b])
if '--write' in sys.argv:
 (ROOT/PATH).write_text(revised)
 (ROOT/'student.tests/pa21/reference102-revision.json').write_text(json.dumps(manifest,indent=2)+'\n')
else:
 assert (ROOT/PATH).read_text()==revised
 assert json.loads((ROOT/'student.tests/pa21/reference102-revision.json').read_text())==manifest
print(json.dumps(manifest,indent=2))
