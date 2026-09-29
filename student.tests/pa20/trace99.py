#!/usr/bin/env python3
"""Trace aggregate summaries, shared helpers and result ABI through native execution."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK,OUT=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
    r=subprocess.run([str(x) for x in args],capture_output=True,text=True,timeout=60)
    assert r.returncode==0,(args,r.stderr)
    return r
source=ROOT/'student.tests/pa20/trace99.cpp'
ir,ordinary,exe=WORK/'trace.lowir',WORK/'ordinary.lowir',WORK/'trace.exe'
r=run([CC,'--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,source])
run([CC,'--emit-lowir','-O0','-o',ordinary,source])
assert ir.read_bytes()==ordinary.read_bytes()
stats=[json.loads(l) for l in r.stderr.splitlines()]
text=ir.read_text()
helpers=re.findall(r'^function @__aggregate_.*$',text,re.M)
assert len(helpers)==2,helpers
boundaries=[re.search(r'^function @'+name+r'\(.*$',text,re.M)[0] for name in ('moved','forward')]
assert all('pass=indirect_result' in line and '-> void' in line for line in boundaries)
semantic=next(s for s in stats if 'template_body_transitions' in s)
assert semantic['template_body_transitions']==2,semantic
run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
OUT.write_text(json.dumps(dict(source_sha256=sha(source),compiler_sha256=sha(CC),stats=stats,
    ir_sha256=sha(ir),native_sha256=sha(exe),native_exit=0,telemetry_output_identical=True,
    helpers=helpers,boundaries=boundaries,
    checks='two demanded template bodies despite repeated calls; two shared type/shape helpers for three aggregate return bodies; two indirect result boundaries'),indent=2)+'\n')
print('aggregate/result source-to-native trace PASS')
