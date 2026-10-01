#!/usr/bin/env python3
"""Check query sharing, source-owned fixed calls, and typed IR/native transport."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-160/inspection').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';result={'compiler_sha256':hashlib.sha256(cc.read_bytes()).hexdigest(),'sharing':[],'checks':[]}
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,cwd=root,timeout=90)
 assert p.returncode==0,(args,p.returncode,p.stderr.decode())
 return p
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for n in [100,10000]:
 src=out/('repeat'+str(n)+'.cpp')
 src.write_text('template<class T>T&& declval() noexcept; struct B{int x; int add(int) const;}; struct P{B& operator*() const noexcept;};\n'+n*'''static_assert(__is_same(decltype(__builtin_invoke(&B::x,declval<P>())),int&),"data");
static_assert(__is_same(decltype(__builtin_invoke(&B::add,declval<P>(),1)),int),"call");
''')
 p=run([cc,'-c','--stats',src,'-o',out/'repeat.o'])
 stats=[json.loads(s) for s in p.stderr.decode().splitlines() if s.startswith('{')]
 result['sharing'].append(dict(repetitions=n,input_sha256=sha(src),phase_counters=stats))
for key in ['semantic_type_query_work','template_body_transitions']:
 values=[next((s[key] for s in row['phase_counters'] if key in s),None) for row in result['sharing']]
 assert values[0] is not None and values[0]==values[1],(key,values)
src=root/'student.tests/pa29/controls160/member-template.cpp';obj=out/'member.o';low=out/'member.lowir';rt=out/'roundtrip.lowir';exe=out/'native'
run([cc,'-c',src,'-o',obj]);stats=run([cc,'-c','--stats',src,'-o',out/'stats.o'])
result['fixed_template_counters']=[json.loads(s) for s in stats.stderr.decode().splitlines() if s.startswith('{')]
assert obj.read_bytes()==(out/'stats.o').read_bytes();result['checks'].append('telemetry preserves object bytes')
run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low]);run([root/'dev/lowir',low,'-o',rt])
assert low.read_bytes()==rt.read_bytes();result['checks'].append('LowIR roundtrip bytes')
run([root/'dev/lowir2native','--dump-machine-ir',out/'member.mir',rt,'-o',exe]);run([exe]);result['checks'].append('roundtrip native execution')
dis=run(['objdump','-drC',obj]).stdout.decode();(out/'member.disassembly').write_text(dis)
assert 'Pointer::operator*() const' in dis and 'Value::add(int) const' in dis
result['checks'].append('selected dereference and member native calls')
result['inspection_hashes']={p.name:sha(p) for p in [obj,low,rt,out/'member.mir',out/'member.disassembly']}
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n');print('query sharing, typed IR, native and telemetry checks: PASS')
