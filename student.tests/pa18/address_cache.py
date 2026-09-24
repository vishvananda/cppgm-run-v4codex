#!/usr/bin/env python3
"""Repeated completed address facts must not repeat overload candidate work."""
from pathlib import Path
import json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]);WORK.mkdir(parents=True,exist_ok=True)
rows=[]
for n in (32,128,512):
 source='int f(){return 7;}long f(int){return 3;}template<int(*P)()>struct A{static int get(){return P();}};int main(){'
 source+=''.join('A<&f> a'+str(i)+';' for i in range(n))+'return A<&f>::get()!=7;}'
 src=WORK/(str(n)+'.cpp');src.write_text(source);ir=src.with_suffix('.lowir');exe=src.with_suffix('.exe')
 r=subprocess.run([ROOT/'dev/cppgm++','--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,src],capture_output=True,text=True)
 telemetry=[json.loads(l) for l in r.stderr.splitlines()] if not r.returncode else []
 row=dict(count=n,source=source,compiler_exit=r.returncode,telemetry=telemetry)
 if not r.returncode:
  b=subprocess.run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir],capture_output=True,text=True)
  row['backend_exit']=b.returncode
  if not b.returncode:row['native_exit']=subprocess.run([exe]).returncode
 row['passed']=row.get('native_exit')==0 and telemetry[0]['semantic_candidate_work']<=8
 rows.append(row);print(n,'PASS' if row['passed'] else 'FAIL',telemetry[0]['semantic_candidate_work'] if telemetry else r.stderr)
(WORK/'results.json').write_text(json.dumps(rows,indent=2)+'\n')
assert all(r['passed'] for r in rows),rows
