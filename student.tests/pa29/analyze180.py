#!/usr/bin/env python3
"""Bind final validation, controls and all four performance dimensions to source."""
import pathlib,subprocess,json,hashlib,shutil,sys
root=pathlib.Path(__file__).resolve().parents[2]
art=pathlib.Path(sys.argv[1]).resolve();out=pathlib.Path(sys.argv[2]).resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(name,x):(out/(name+'.json')).write_text(json.dumps(x,indent=2)+'\n')
for source,dest in [('common-final/performance.json','common-performance.json'),('owner-final/performance.json','owner-performance.json'),('controls-ranking/controls.json','controls.json'),('inspection-complete180/inspection.json','inspection.json')]:shutil.copyfile(art/source,out/dest)
common=json.load(open(out/'common-performance.json'));owner=json.load(open(out/'owner-performance.json'))
validated=json.load(open(out/'validated-source.json'));entry=json.load(open(out/'entry.json'))
assert sha(root/'dev/cppgm++')==validated['compiler_sha256']==common['binaries']['B']['sha256']==owner['binaries']['B']['sha256']
assert common['binaries']['A']['sha256']==owner['binaries']['A']['sha256']==sha(art/'entry-cppgm++')
assert len(common['runs'])==224 and len(owner['runs'])==328
assert len(json.load(open(out/'controls.json')))==47
assert all('failure' not in x for x in json.load(open(out/'controls.json')))
inspection=json.load(open(out/'inspection.json'));assert len(inspection['rows'])==251 and all(x['status']==0 for x in inspection['rows'])
assert inspection['compiler_sha256']==validated['compiler_sha256']
assert all(sha(root/p)==digest for p,digest in validated['files'].items())
comparison={}
for name,images in common['images'].items():
 comparison[name]=dict(object_identical=images['A']['object_sha256']==images['B']['object_sha256'],executable_identical=images['A']['executable_sha256']==images['B']['executable_sha256'])
 assert comparison[name]['object_identical'] and comparison[name]['executable_identical']
save('elf-comparison',comparison)
adapters={}
for obj in sorted((art/'inspection-complete180').glob('*.adapter.o')):
 direct=obj.with_name(obj.name.replace('.adapter.o','.o'))
 def dump(path):return subprocess.check_output(['objdump','-dr',str(path)],text=True).splitlines()[3:]
 def symbols(path):return sorted(subprocess.check_output(['nm',str(path)],text=True).splitlines())
 adapters[direct.stem]=dict(direct_sha256=sha(direct),adapter_sha256=sha(obj),identical_bytes=direct.read_bytes()==obj.read_bytes(),identical_instructions_and_symbolic_relocations=dump(direct)==dump(obj),identical_symbols=symbols(direct)==symbols(obj))
 assert adapters[direct.stem]['identical_instructions_and_symbolic_relocations'] and adapters[direct.stem]['identical_symbols']
save('adapter-equivalence',adapters)
scaling={}
for name,input in owner['inputs'].items():
 rows=[r for r in owner['runs'] if r['workload']==name and r['mode']=='compile' and r['label']=='B']
 facts=[]
 for row in rows:
  low=next(c for c in row['counters'] if 'inline_calls' in c)
  semantic=next(c for c in row['counters'] if 'parsed_nodes' in c)
  native=next(c for c in row['counters'] if 'native_instructions' in c)
  assert low['inline_work']<=low['inline_budget_work']<=4194304 and low['inline_max_function_work']<=262144
  facts.append(dict(parsed_nodes=semantic['parsed_nodes'],nodes=low['nodes'],instructions=low['instructions'],candidates=semantic['semantic_candidate_work'],frames=semantic['semantic_substitution_frames'],inline_calls=low['inline_calls'],inline_work=low['inline_work'],inline_budget_work=low['inline_budget_work'],inline_max_function_work=low['inline_max_function_work'],inline_declined=low['inline_declined'],native_instructions=native['native_instructions'],text_bytes=native['text_bytes']))
 assert all(x==facts[0] for x in facts)
 scaling[name]=dict(n=input['n'],observations=len(rows),facts=facts[0])
save('scaling',scaling)
# Preserve every preliminary observation with its own frozen compiler binding.
assert len(json.load(open(out/'preliminary-common-performance.json'))['runs'])==224
assert len(json.load(open(out/'preliminary-owner-performance.json'))['runs'])==328
manifest=dict(code_tip=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),entry=entry['entry'],stage_base='2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543',last_reviewed=entry['previous_review'],compiler_sha256=validated['compiler_sha256'],entry_compiler_sha256=sha(art/'entry-cppgm++'),flags=owner['flags'],common_flags=common['flags'],machine=subprocess.check_output(['uname','-a'],text=True).strip(),host_compiler=subprocess.check_output(['clang++','--version'],text=True),evidence={p.name:sha(p) for p in sorted(out.glob('*.json')) if p.name!='manifest.json'},scripts={str(p.relative_to(root)):sha(p) for p in [root/'student.tests/pa27/performance147_common.py',*sorted((root/'student.tests/pa29').glob('*180*'))] if p.is_file()},inherited_performance={str(p.relative_to(root)):sha(p) for p in [root/'pa29/performance179.md',root/'student.tests/pa29/evidence179/common-performance.json',root/'student.tests/pa29/evidence179/owner-performance.json']})
save('manifest',manifest)
print('complete final performance/validation source binding',manifest['code_tip'],flush=True)
