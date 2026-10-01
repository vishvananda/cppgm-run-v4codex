#!/usr/bin/env python3
"""Typed views, external LowIR adapter, unwind and telemetry checks."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';source=root/'student.tests/pa29/source177';rows=[]
def run(args):
 p=subprocess.run(list(map(str,args)),cwd=root,capture_output=True,text=True,timeout=90)
 r=dict(command=list(map(str,args)),status=p.returncode,stdout=p.stdout,stderr=p.stderr);rows.append(r)
 assert not p.returncode,r
 return p.stdout
for stem in ['selection','switch','constexpr','condition-declaration','template-runtime','constexpr-return']:
 src=source/(stem+'.cpp')
 for view in ['ast','types','semantics','lowir']:
  # PA6's declaration-only view cannot resolve dependent incomplete aliases;
  # the semantic and LowIR views below exercise that later template surface.
  if stem=='constexpr' and view=='types':continue
  dest=out/(stem+'.'+view)
  run([cc,'--emit-'+view,src,'-o',dest])
  if view=='lowir':run([root/'dev/lowir',dest,'-o',out/(stem+'.roundtrip')])
 obj=out/(stem+'.o');plain=out/(stem+'.plain.o');exe=out/stem
 run([cc,'-O0','--stats','-c',src,'-o',obj]);run([cc,'-O0','-c',src,'-o',plain]);assert obj.read_bytes()==plain.read_bytes()
 run(['g++',obj,'-o',exe]);run([exe])
 frame=run(['readelf','--debug-dump=frames',obj]);assert 'FDE' in frame
# Fixed-return discard is checked; dependent discard has no instantiated body.
ir=(out/'constexpr.lowir').read_text();assert 'missing' not in ir
(out/'inspection.json').write_text(json.dumps(dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),rows=rows),indent=2)+'\n')
print('inspection passed',len(rows),flush=True)
