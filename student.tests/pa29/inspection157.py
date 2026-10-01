#!/usr/bin/env python3
"""Explicit source/LowIR/native inspection and structural layout work controls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-157/inspection').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';source=root/'student.tests/pa29/controls157/inspection.cpp'
result=dict(checks=[],files=[],scaling=[])
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):
 args=list(map(str,args));p=subprocess.run(args,capture_output=True,timeout=90)
 result['checks'].append(dict(args=args,exit_code=p.returncode,stderr=p.stderr.decode()))
 assert p.returncode==0,(args,p.stderr.decode())
 return [json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]
for mode,flags in [('object',[]),('lowir',['--emit-lowir','--validate-lowir'])]:
 for stats in [False,True]:
  path=out/(mode+('-stats' if stats else ''))
  run([cc,'-c',*flags,*(['--stats'] if stats else []),source,'-o',path])
 assert (out/mode).read_bytes()==(out/(mode+'-stats')).read_bytes()
run([root/'dev/lowir','-o',out/'roundtrip',out/'lowir'])
assert (out/'roundtrip').read_bytes()==(out/'lowir').read_bytes()
run([root/'dev/lowir2native','--dump-machine-ir',out/'machine.mir','-o',out/'native',out/'roundtrip'])
run([out/'native'])
# These short cases establish structural work, not wall-time speedups. Array
# bounds grow 10,000-fold without expanding element footprints in the compiler.
for n in [100,10000,1000000]:
 src=out/('array%d.cpp'%n)
 src.write_text('struct E{};struct H{[[no_unique_address]] E e;};struct A{char c;H a[%d];[[no_unique_address]] E e;};static_assert(sizeof(A)==%d && __builtin_offsetof(A,e)==0,"compact exact footprint");int main(){return 0;}\n'%(n,n+1))
 counters=run([cc,'-c','--stats',src,'-o',out/'scale.o'])
 result['scaling'].append(dict(elements=n,source_sha256=sha(src),counters=counters))
for name in ['object','object-stats','lowir','lowir-stats','roundtrip','machine.mir','native']:
 p=out/name;result['files'].append(dict(path=str(p),sha256=sha(p),bytes=p.stat().st_size))
result.update(compiler_sha256=sha(cc),source_sha256=sha(source),canonical_roundtrip_identical=True,telemetry_object_and_lowir_identical=True)
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n')
print('%d inspection commands passed; roundtrip and telemetry output identical'%len(result['checks']))
