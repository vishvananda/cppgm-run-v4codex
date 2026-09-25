#!/usr/bin/env python3
"""Run historical proof 84 unchanged, then verify its composition with proof 87."""
from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys
from reference87 import CASES,ENTRY,repair
ROOT=Path(__file__).resolve().parents[2];W=Path(sys.argv[1]).resolve();W.mkdir(parents=True,exist_ok=True)
manifest=json.loads((ROOT/'student.tests/pa18/reference84-revisions.json').read_text())
for name in ('reference84.py','reference84-revisions.json'):
 dst=W/'student.tests/pa18'/name;dst.parent.mkdir(parents=True,exist_ok=True)
 shutil.copyfile(ROOT/'student.tests/pa18'/name,dst)
for row in manifest['revisions']:
 p=W/row['path'];p.parent.mkdir(parents=True,exist_ok=True)
 before=subprocess.check_output(['git','show',ENTRY+':'+row['path']],cwd=ROOT)
 assert hashlib.sha256(before).hexdigest()==row['after_sha256']
 p.write_bytes(before)
 expected=repair(before.decode(),CASES[row['path']]) if row['path'] in CASES else before.decode()
 assert (ROOT/row['path']).read_text()==expected
env=dict(os.environ,GIT_DIR=subprocess.check_output(['git','rev-parse','--absolute-git-dir'],cwd=ROOT,text=True).strip())
subprocess.run([sys.executable,W/'student.tests/pa18/reference84.py'],cwd=W,env=env,check=True)
print('Verified unchanged historical proof 84, then exact proof-87 composition; all five zero initializations remain.')
