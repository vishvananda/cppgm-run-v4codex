#!/usr/bin/env python3
"""One source interpretation, shared template bodies, checked hidden-friend calls."""
from pathlib import Path
import hashlib, json, re, subprocess, sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK,OUT=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    r=subprocess.run([str(x) for x in args],capture_output=True,text=True,timeout=60)
    assert r.returncode==0,(args,r.stderr)
    return r
source=ROOT/'student.tests/pa20/trace100.cpp'
ir,ordinary,exe=WORK/'trace.lowir',WORK/'ordinary.lowir',WORK/'trace.exe'
r=run([CC,'--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,source])
run([CC,'--emit-lowir','-O0','-o',ordinary,source])
assert ir.read_bytes()==ordinary.read_bytes()
stats=[json.loads(l) for l in r.stderr.splitlines()]
semantic=next(s for s in stats if 'template_body_transitions' in s)
assert semantic['template_body_transitions']==2,semantic
assert semantic['semantic_angle_interpretations']==1,semantic
text=ir.read_text()
assert len(re.findall(r'^function @use(?:__\d+)?\(',text,re.M))==2,text
run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
disassembly=run(['objdump','-D','-b','binary','-m','i386:x86-64',exe]).stdout
sys.path.insert(0,str(ROOT/'student.tests/pa20'))
import angle100
scaling=[]
for count in (40,600):
    src=WORK/('unclosed-'+str(count)+'.cpp')
    src.write_text(angle100.runner.GOOD['unclosed_chain_'+str(count)])
    parsed=run([CC,'--emit-ast','--stats','-o',WORK/'syntax.txt',src])
    data=json.loads(parsed.stderr)
    scaling.append(dict(count=count,source_sha256=sha(src),stats=data))
assert scaling[1]['stats']['angle_work'] < 30*scaling[0]['stats']['angle_work'],scaling
OUT.write_text(json.dumps(dict(source_sha256=sha(source),compiler_sha256=sha(CC),stats=stats,
    ir_sha256=sha(ir),native_sha256=sha(exe),native_exit=0,telemetry_output_identical=True,
    lowir=text,disassembly=disassembly,negative_angle_scaling=scaling,
    checks='one source grammar interpretation; two demanded specialization bodies for four calls; hidden friend increments checked count'),indent=2)+'\n')
print('retained-angle source-to-native trace PASS')
