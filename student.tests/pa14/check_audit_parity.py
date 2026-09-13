#!/usr/bin/env python3
"""Compare frozen student outputs using the unchanged PA14 course contract."""
from pathlib import Path
import hashlib, json, subprocess, sys
from body_compare import compare
ROOT = Path(__file__).resolve().parents[2]
A,B,WORK = map(lambda p: Path(p).resolve(),sys.argv[1:4])
WORK.mkdir(parents=True,exist_ok=True)
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
sources = sorted((ROOT/'pa14/tests').glob('*/*.t'))+sorted((ROOT/'student.tests/pa14').glob('*.cpp'))
records=[]
for i,source in enumerate(sources):
    row=dict(source=str(source),source_sha256=sha(source),outputs=[])
    for b,binary in enumerate((A,B)):
        out=WORK/f'{i}-{b}.lowir'
        p=subprocess.run([str(binary),'--emit-lowir','-O0','--validate-lowir','-o',str(out),str(source)],capture_output=True,text=True,timeout=60)
        assert p.returncode in (0,1),(source,p.returncode,p.stderr)
        row['outputs'].append(dict(path=str(out),exit_code=p.returncode,sha256=sha(out) if p.returncode==0 else None))
    a,b=row['outputs']
    assert a['exit_code']==b['exit_code'],(source,a,b)
    row['exact']=a['sha256']==b['sha256']
    if not row['exact']: row['comparison']=compare(source,row['outputs'],WORK/'canonical',str(i))
    records.append(row)
    (WORK/'checks.json').write_text(json.dumps(records,indent=2)+'\n')
    if (i+1)%50==0:print(i+1,'checked',flush=True)
print(len(records),'course-contract parity checks PASS;',sum(not r['exact'] for r in records),'presentation differences')
