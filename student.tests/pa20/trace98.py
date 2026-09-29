#!/usr/bin/env python3
"""Trace canonical specialization/capture identities through typed LowIR to ELF."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK,OUT=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 r=subprocess.run([str(x) for x in args],capture_output=True,text=True,timeout=60)
 assert r.returncode==0,(args,r.stderr);return r
source=ROOT/'student.tests/pa20/trace98.cpp';ir=WORK/'trace.lowir';ordinary=WORK/'ordinary.lowir';exe=WORK/'trace.exe'
r=run([CC,'--emit-lowir','-O0','--stats','--validate-lowir','-o',ir,source])
run([CC,'--emit-lowir','-O0','-o',ordinary,source]);assert ir.read_bytes()==ordinary.read_bytes()
stats=[json.loads(l) for l in r.stderr.splitlines()]
semantic=next(s for s in stats if 'semantic_capture_edges' in s)
assert semantic['semantic_closures']==5,semantic
assert semantic['semantic_capture_edges']==10,semantic
assert semantic['semantic_body_checks']==9,semantic
assert semantic['template_body_transitions']==2,semantic
run([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);run([exe])
OUT.write_text(json.dumps(dict(source_sha256=sha(source),compiler_sha256=sha(CC),stats=stats,
 ir_sha256=sha(ir),native_sha256=sha(exe),native_exit=0,telemetry_output_identical=True,
 checks='two demanded specializations; five closures; ten unique capture edges; nine checked bodies; repeated call reuses facts'),indent=2)+'\n')
print('capture source-to-native trace PASS')
