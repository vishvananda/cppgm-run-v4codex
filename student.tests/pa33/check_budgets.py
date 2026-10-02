#!/usr/bin/env python3
"""Exercise actual prefix admission caps and conservative boundary fallbacks."""
import os,pathlib,subprocess,json,re
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa33-219/budgets';out.mkdir(parents=True,exist_ok=True)
header='declare function @length(%s : ptr) -> i64 [object=cppgm_builtin_strlen, effects=readonly, unwind=no]\n'
body=lambda n:'function @f%d(%%s : ptr) -> i64 { block ^entry: '%n+' '.join('%%v%d = call i64 @length(%%s)'%i for i in range(12))+' return i64 %v11 }\n'
source=out/'bounded.lowir';source.write_text(header+''.join(body(n) for n in range(140)))
rows=[]
for level in range(4):
    mir=out/f'O{level}.mir'
    p=subprocess.run([str(root/'dev/lowir2native'),f'-O{level}','--stats','--dump-machine-ir',str(mir),str(source)],capture_output=True,text=True,timeout=60)
    assert p.returncode==0,p.stderr
    text=mir.read_text();count=text.count('strlen_prefix=16')
    assert count==(128 if level else 0),(level,count)
    assert all(f.count('strlen_prefix=16')<=8 for f in text.split('\nfunction '))
    assert len(re.findall(r'^    call @length ',text,re.M))==140*12
    rows.append(dict(level=level,prefix_calls=count,stats=p.stderr))
# Complete signature/effect/alias gates are independent of the marker spelling.
for suffix,metadata,args,kind in [
    ('effects','unwind=no','%s : ptr','i64'),
    ('unwind','effects=readonly','%s : ptr','i64'),
    ('result','effects=readonly, unwind=no','%s : ptr','i32'),
    ('passing','effects=readonly, unwind=no','%s : ptr [pass=by_address]','i64')]:
    src=out/(suffix+'.lowir');mir=out/(suffix+'.mir')
    src.write_text(f'declare function @length({args}) -> {kind} [object=cppgm_builtin_strlen, {metadata}]\nfunction @probe(%s : ptr) -> {kind} {{ block ^entry: %v = call {kind} @length(%s) return {kind} %v }}\n')
    p=subprocess.run([str(root/'dev/lowir2native'),'-O2','--dump-machine-ir',str(mir),str(src)],capture_output=True,text=True)
    assert p.returncode==0,p.stderr
    assert 'strlen_prefix=' not in mir.read_text()
    rows.append(dict(control=suffix,status=p.returncode))
(out/'checks.json').write_text(json.dumps(rows,indent=2)+'\n')
print('Prefix budgets and complete ABI/effect gates: PASS (4 levels, 4 negative boundaries)')
