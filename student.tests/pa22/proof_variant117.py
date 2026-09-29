#!/usr/bin/env python3
"""Freeze a same-source control with member adjustment analysis/consumption off."""
from pathlib import Path
import hashlib,json,shlex,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve(); WORK.mkdir(parents=True,exist_ok=True)
binary=WORK/'compiler-generic'; assert not binary.exists()
source=ROOT/'dev/src/semantic/member_pointer_values.cpp'
text=source.read_text()
getter='return member_pointer_value_states.get(object) == 2;'
prepare='void Analyzer::prepare_member_pointer_value(EntityId object, unsigned* remaining)\n{'
assert text.count(getter)==text.count(prepare)==1
text=text.replace(getter,'return false;').replace(prepare,prepare+'\n    return; // Experimental conservative lane only.')
variant=WORK/'proof-off.cpp'; variant.write_text(text)
obj=WORK/'proof-off.o'
compile=['g++','-std=gnu++11','-Wall','-O3','-I',str(ROOT/'dev/src'),'-c',str(variant),'-o',str(obj)]
subprocess.run(compile,check=True)
dry=subprocess.check_output(['make','-n','-C','dev','V=1','cppgm++'],cwd=ROOT,text=True)
line=next(l for l in dry.splitlines() if l.startswith('g++ ') and ' -o cppgm++.tmp ' in l)
link=shlex.split(line); link[link.index('-o')+1]=str(binary)
link[link.index('../obj/dev/semantic/member_pointer_values.o')]=str(obj)
subprocess.run(link,cwd=ROOT/'dev',check=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(WORK/'proof-off-build.json').write_text(json.dumps(dict(source_sha256=sha(source),variant_sha256=sha(variant),binary_sha256=sha(binary),compile=compile,link=link),indent=2)+'\n')
print(binary)
