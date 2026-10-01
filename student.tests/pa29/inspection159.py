#!/usr/bin/env python3
"""Inspect cache demand, template body isolation, typed IR and selected native calls."""
import hashlib,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=pathlib.Path(sys.argv[1] if len(sys.argv)>1 else '/tmp/pa29-159/inspection').resolve();out.mkdir(parents=True,exist_ok=True)
cc=root/'dev/cppgm++';rows=[]
def run(args):
 p=subprocess.run(list(map(str,args)),capture_output=True,cwd=root,timeout=90)
 assert p.returncode==0,(args,p.stderr.decode())
 return p
for n in [100,10000]:
 src=out/('repeat'+str(n)+'.cpp')
 src.write_text('struct Plain{int x;}; struct Source { operator int() const; };\n'+n*'''static_assert(__has_trivial_constructor(Plain),"default");
static_assert(__has_nothrow_copy(Plain),"copy");
static_assert(__has_trivial_assign(Plain),"assignment");
static_assert(__reference_constructs_from_temporary(const int&,Source),"temporary");
static_assert(!__reference_constructs_from_temporary(const int&,int&),"binding");
''')
 p=run([cc,'-c','--stats',src,'-o',out/('repeat'+str(n)+'.o')]);stats=[json.loads(line) for line in p.stderr.decode().splitlines() if line.startswith('{')]
 rows.append(dict(repetitions=n,input_sha256=hashlib.sha256(src.read_bytes()).hexdigest(),phase_counters=stats))
keys=['semantic_legacy_trait_work','semantic_legacy_member_work','semantic_type_query_work','template_body_transitions']
for key in keys:
 values=[next((s[key] for s in row['phase_counters'] if key in s),None) for row in rows]
 assert values[0] is not None,(key,'missing counter')
 assert values[0]==values[1],(key,values)
checks=[]
for name in ['traits','operations']:
 src=root/'student.tests/pa29/controls155'/(name+'.cpp');obj=out/(name+'.o');exe=out/name
 run([cc,'-c',src,'-o',obj]);run(['g++',obj,'-o',exe]);run([exe]);checks.append(name)
src=root/'student.tests/pa29/controls159/reference-runtime.cpp';obj=out/'binding.o';low=out/'binding.lowir'
run([cc,'-c',src,'-o',obj]);run([cc,'-c','--emit-lowir','--validate-lowir',src,'-o',low])
dis=run(['objdump','-drC',obj]).stdout.decode();(out/'binding.disassembly').write_text(dis)
assert 'Explicit::operator int() const' in dis and 'ExplicitRef::operator int&() const' in dis
assert 'Both::operator int() const' not in dis,'nonselected conversion was emitted'
assert 'Both::operator int&() const' in dis
result=dict(compiler_sha256=hashlib.sha256(cc.read_bytes()).hexdigest(),cache=rows,inherited_controls=checks,native=dict(disassembly_sha256=hashlib.sha256(dis.encode()).hexdigest(),selected_only=True))
(out/'inspection.json').write_text(json.dumps(result,indent=2)+'\n');print('cache, inherited traits and selected native calls: PASS')
