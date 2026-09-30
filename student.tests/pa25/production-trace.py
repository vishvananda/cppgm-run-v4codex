#!/usr/bin/env python3
import pathlib, subprocess, json, sys
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
objects=[p for p in (root/'obj/dev').rglob('*.o') if 'entry' not in p.parts and not p.stem.startswith('test_runner')]
commands=[['g++','-std=c++11','-I'+str(root/'dev/src'),root/'student.tests/pa25/production-trace.cc',*objects,'-o',out/'trace-api']]
results=[]
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,text=True,timeout=60)
 (out/f'{len(results)}.stdout').write_text(p.stdout);(out/f'{len(results)}.stderr').write_text(p.stderr)
 results.append(dict(command=list(map(str,args)),status=p.returncode));assert p.returncode==0,(args,p.stderr)
for args in commands:run(args)
for name,src in [('floating',root/'student.tests/pa25/floating-evaluation.cc'),('exception',out.parent/'exceptions/cross1.cc')]:
 if name=='exception':
  src=out/'exception.cc';src.write_text('template<int N> struct E{int n;E():n(N){}}; int main(){try{throw E<7>();}catch(E<7>& e){return e.n!=7;}return 1;}\n')
 stem=out/name;run([out/'trace-api',src,stem]);run([str(stem)+'.elf'])
 assert pathlib.Path(str(stem)+'.lowir').read_bytes()==pathlib.Path(str(stem)+'.roundtrip').read_bytes()
 if name=='floating':
  assert '[eval=f80]' in pathlib.Path(str(stem)+'.lowir').read_text()
  assert '[eval=f80]' in pathlib.Path(str(stem)+'.mir').read_text()
  run([root/'dev/lowir2native','-o',out/'floating-adapter.elf',str(stem)+'.lowir']);run([out/'floating-adapter.elf'])
(out/'results.json').write_text(json.dumps(results,indent=2)+'\n')
print('production typed IR/roundtrip/MIR/native execution passed')
