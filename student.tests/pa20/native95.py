#!/usr/bin/env python3
"""Execute range fixtures and independently repaired oracles. Run CC WORK OUT."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
CC,WORK,OUT=[Path(p).resolve() for p in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def command(args):
 r=subprocess.run([str(x) for x in args],cwd=ROOT,capture_output=True,text=True,timeout=60)
 assert r.returncode==0,(args,r.returncode,r.stderr)
 return r
manifest=json.loads((ROOT/'student.tests/pa20/reference95-revisions.json').read_text())
paths={Path(r['path']).with_suffix('.t') for r in manifest['revisions']}
paths|={p.relative_to(ROOT) for p in (ROOT/'pa20/tests').rglob('*range-for*.t')}
paths.add(Path('pa20/tests/general/100-conversion-operator-reference-initialization.t'))
rows=[]
for path in sorted(paths):
 stem=WORK/(path.parent.name+'-'+path.stem)
 ir=stem.with_suffix('.lowir');exe=stem.with_suffix('.exe')
 result=command([CC,'--emit-lowir','-O0','--validate-lowir','--stats','-o',ir,ROOT/path])
 command([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);actual=subprocess.run([exe],timeout=15).returncode
 r=dict(path=str(path),source_sha256=sha(ROOT/path),ir_sha256=sha(ir),native_exit=actual,
  stats=[json.loads(l) for l in result.stderr.splitlines()],oracles=[])
 for label,content in [('old',subprocess.check_output(['git','show',manifest['entry']+':'+str(path.with_suffix('.ref'))],cwd=ROOT)),('current',(ROOT/path.with_suffix('.ref')).read_bytes())]:
  ref=WORK/(stem.name+'-'+label+'.lowir');ref.write_bytes(content);native=ref.with_suffix('.exe')
  command([ROOT/'dev/lowir2native-ref','-O0','-o',native,ref]);expected=subprocess.run([native],timeout=15).returncode
  assert actual==expected,(path,label,actual,expected)
  r['oracles'].append(dict(revision=label,sha256=sha(ref),native_exit=expected))
 rows.append(r)
 print(path,'PASS',actual,flush=True)
source=ROOT/'student.tests/pa20/array_reducer95.cpp';ir=WORK/'reducer.lowir';exe=WORK/'reducer.exe'
command([CC,'--emit-lowir','-O0','--validate-lowir','-o',ir,source]);command([ROOT/'dev/lowir2native-ref','-O0','-o',exe,ir]);command([exe])
text=ir.read_text();assert text.count('copyobj 12x4')==2
assert text.count('storage=readonly')==1
OUT.write_text(json.dumps(dict(compiler_sha256=sha(CC),backend_sha256=sha(ROOT/'reference-binaries/lowir2native'),
 fixtures=rows,reducer=dict(source_sha256=sha(source),native_exit=0,distinct_destinations=2,readonly_images=1)),indent=2)+'\n')
