#!/usr/bin/env python3
"""Explicit cumulative semantic, query-completion and repaired-course execution."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
cc=Path(sys.argv[1]).resolve();work=Path(sys.argv[2]);work.mkdir(parents=True,exist_ok=True)
groups={'invoke':'invoke73','prototype':'prototype73','alias':'alias72','pack':'pack72','context':'context71','ordering':'ordering','substitution':'substitution','conversion':'conversion','address':'address','deduction':'deduction68','query':'query69','audit':'audit70'}
rows={}
for name,script in groups.items():
 r=subprocess.run(['python3',ROOT/'student.tests/pa18'/f'{script}_controls.py',cc,work/name],capture_output=True,text=True,timeout=400)
 (work/(name+'.log')).write_text(r.stdout+r.stderr)
 cases=json.loads((work/name/'results.json').read_text());rows[name]=dict(exit=r.returncode,count=len(cases),passing=sum(c['passed'] for c in cases));print(name,rows[name],flush=True)
r=subprocess.run(['python3',ROOT/'student.tests/pa18/completion_scaling.py',cc,work/'completion'],capture_output=True,text=True,timeout=120)
(work/'completion.log').write_text(r.stdout+r.stderr);rows['completion']=dict(exit=r.returncode);print('completion',r.returncode,flush=True)
(work/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['exit']==0 for r in rows.values()),rows
