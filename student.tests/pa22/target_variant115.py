#!/usr/bin/env python3
"""Freeze a correct generic member-call lane with immediate-target recording off."""
from pathlib import Path
import hashlib,json,shlex,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else Path('/tmp/pa22-115')
WORK.mkdir(parents=True,exist_ok=True)
assert not (WORK/'compiler-generic').exists(), 'Preserve the frozen lane; choose a fresh scratch directory'
p=ROOT/'dev/src/semantic/member_pointers.cpp';s=p.read_text()
needle='if (result.form == ExpressionForm::BoundMember && ast[operand].kind'
assert s.count(needle)==1
variant=WORK/'target-off.cpp';variant.write_text(s.replace(needle,'if (false && result.form == ExpressionForm::BoundMember && ast[operand].kind'))
obj=WORK/'target-off.o';binary=WORK/'compiler-generic'
compile=['g++','-std=gnu++11','-Wall','-O3','-I',str(ROOT/'dev/src'),'-c',str(variant),'-o',str(obj)]
subprocess.run(compile,check=True)
dry=subprocess.check_output(['make','-n','-C','dev','V=1','cppgm++'],cwd=ROOT,text=True)
line=next(l for l in dry.splitlines() if l.startswith('g++ ') and ' -o cppgm++.tmp ' in l)
link=shlex.split(line);link[link.index('-o')+1]=str(binary)
assert '../obj/dev/semantic/member_pointers.o' in link
link[link.index('../obj/dev/semantic/member_pointers.o')]=str(obj)
subprocess.run(link,cwd=ROOT/'dev',check=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(WORK/'target-off-build.json').write_text(json.dumps(dict(source_sha256=sha(p),variant_sha256=sha(variant),binary_sha256=sha(binary),compile=compile,link=link),indent=2)+'\n')
print(binary)
