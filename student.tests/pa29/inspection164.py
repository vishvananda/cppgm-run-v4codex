#!/usr/bin/env python3
"""Inspect predefined-string storage, external LowIR/MIR and unfinished context reducer."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
rows=[]
def run(name,args,expect=0):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 rows.append(dict(name=name,args=list(map(str,args)),status=p.returncode,passed=p.returncode==expect,stdout=p.stdout,stderr=p.stderr))
 assert p.returncode==expect,(name,p.returncode,p.stderr)
 return p
low=out/'strings.lowir';run('source IR',['dev/cppgm++','-c','--emit-lowir','--validate-lowir','student.tests/pa29/controls164/function-context.cpp','-o',low])
s=low.read_text();assert 'global @__local_static_' in s and '[binding=internal]' in s
# This is an explicit external-IR debug annotation test, not a claim that the
# source compiler supports -g with --emit-lowir or emits native DWARF.
lines=s.splitlines();inserted=False
for i,line in enumerate(lines):
 if line.startswith('function ') and line.endswith(' {'):
  lines[i]=line[:-2]+' !dbg(context.cpp, 1, 1) {';inserted=True;break
assert inserted
low.write_text('\n'.join(lines)+'\n')
rt=out/'strings.roundtrip';run('roundtrip',['dev/lowir',low,'-o',rt]);assert low.read_bytes()==rt.read_bytes()
mir=out/'strings.mir';exe=out/'strings-native';run('MIR/native',['dev/lowir2native','--dump-machine-ir',mir,rt,'-o',exe]);run('execution',[exe]);assert '!dbg(context.cpp, 1, 1)' in mir.read_text()
run('unfinished evaluation-context reducer',['dev/cppgm++','-std=c++14','-c','student.tests/pa29/pending164/evaluation-context.cpp','-o',out/'pending.o'],1)
(out/'inspection.json').write_text(json.dumps(dict(checks=rows,internal_string_storage=True,explicit_ir_debug_transport=True,lowir_sha256=hashlib.sha256(low.read_bytes()).hexdigest(),pending_disposition='unfinished implementation; rejection is not stage progress'),indent=2)+'\n');print('String storage/LowIR/MIR inspection: PASS')
