#!/usr/bin/env python3
"""LowIR ABI probe: contextual truth must inspect the target, not adjustment."""
from pathlib import Path
import subprocess,json
ROOT=Path(__file__).resolve().parents[2];HERE=Path(__file__).parent;WORK=Path('/tmp/pa22-114/representation');WORK.mkdir(parents=True,exist_ok=True)
ir=WORK/'truth.lowir';merged=WORK/'merged.lowir';exe=WORK/'truth'
commands=[[ROOT/'dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',ir,HERE/'member-truth-word.source'],
 [ROOT/'dev/lowir','-o',merged,ir,HERE/'member-truth-word.lowir'],
 [ROOT/'dev/lowir2native-ref','-O0','-o',exe,merged],[exe]]
rows=[]
for cmd in commands:
 r=subprocess.run(cmd,capture_output=True,text=True);assert not r.returncode,(cmd,r.stderr)
 rows.append(dict(command=[str(x) for x in cmd],exit=r.returncode))
(HERE/'representation114.json').write_text(json.dumps(dict(null_target_nonzero_adjustment=True,checks=rows),indent=2)+'\n')
print('target-word truth probe passed')
