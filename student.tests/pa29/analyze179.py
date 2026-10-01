#!/usr/bin/env python3
"""Bind final checks to frozen images and retain every recorded measurement."""
import hashlib,json,pathlib,shutil,statistics,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
scratch=pathlib.Path(sys.argv[1]).resolve()
out=root/'student.tests/pa29/evidence179'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def save(name,data):(out/(name+'.json')).write_text(json.dumps(data,indent=2)+'\n')
copies={
 'controls':'controls-complete/controls.json',
 'inspection':'inspection-complete/inspection.json',
 'common-performance':'common-complete/performance.json',
 'owner-performance':'owner-complete/performance.json',
 'preliminary-common-performance':'common-performance/performance.json',
 'preliminary-expanded-controls':'controls-expanded/controls.json',
 'preliminary-reference-controls':'controls-reference/controls.json',
}
for name,source in copies.items():shutil.copyfile(scratch/source,out/(name+'.json'))
data={name:json.loads((out/(name+'.json')).read_text()) for name in copies}
final=sha(scratch/'complete-cppgm++')
assert final==sha(root/'dev/cppgm++')
for name in ['controls','inspection','validation','validated-source']:
 r=json.loads((out/(name+'.json')).read_text());assert r['compiler_sha256']==final
assert len(data['controls']['rows'])==42
assert all(r['passed'] and r['host_passed'] for r in data['controls']['rows'])
assert len(data['inspection']['rows'])==91 and all(r['status']==0 for r in data['inspection']['rows'])
common=data['common-performance'];owner=data['owner-performance']
assert len(common['runs'])==224 and len(owner['runs'])==144
assert len(owner['launcher_s'])==8
for r in [common,owner]:
 assert r['binaries']['B']['sha256']==final
 assert r['binaries']['A']['sha256']==sha(scratch/'entry-cppgm++')
 for sample in r['runs']:assert sample['status']==0
for name,images in common['images'].items():assert images['A']==images['B'],name
save('elf-comparison',dict(all_common_objects_and_executables_identical=True,images=common['images']))
fields=['parsed_nodes','nodes','template_occurrences','semantic_binding_shapes',
 'semantic_binding_members','semantic_binding_projections','semantic_binding_storage_bytes',
 'instructions','native_instructions','text_bytes','semantic_substitution_frames','template_body_transitions']
scaling={}
for name,info in owner['inputs'].items():
 rows=[r for r in owner['runs'] if r['workload']==name and r['mode']=='compile']
 counters=[dict(item for phase in r['counters'] for item in phase.items()) for r in rows]
 values={k:counters[0][k] for k in fields}
 assert all(all(r[k]==v for k,v in values.items()) for r in counters)
 assert values['semantic_binding_shapes']==1
 assert values['semantic_binding_members']==(0 if info['family']=='array' else 2)
 assert values['semantic_binding_projections']==2*(info['n']+1)
 assert values['semantic_substitution_frames']==info['n']
 assert values['template_body_transitions']==info['n']
 assert info['entry_status']!=0
 assert sha(pathlib.Path(info['path']))==info['sha256']
 scaling[name]=dict(n=info['n'],family=info['family'],counters=values)
equations={}
for family in ['class','array','lifetime']:
 equations[family]={}
 for key in fields:
  if key=='semantic_binding_storage_bytes':continue # vector capacity is geometric
  rows=[scaling[family+str(n)] for n in [600,1200,2400]]
  slope=(rows[1]['counters'][key]-rows[0]['counters'][key])/600
  offset=rows[0]['counters'][key]-600*slope
  assert all(r['counters'][key]==slope*r['n']+offset for r in rows),(family,key)
  equations[family][key]=dict(slope=slope,offset=offset)
save('scaling',dict(all_72_compilations_checked=True,workloads=scaling,equations=equations))
save('preliminary-owner-attempt',dict(status=1,diagnostic=(scratch/'owner-performance.log').read_text(),
 classification='No affected workload timing accepted: final at bcef72ad rejected the first input. Launcher timings were not persisted before this failure. Shared reference narrowing was subsequently repaired; final-only measurements use the corrected frozen image.'))
manifest=dict(code_tip=json.loads((out/'validation.json').read_text())['code_tip'],
 entry=json.loads((out/'entry.json').read_text())['entry'],
 final_compiler_sha256=final,entry_compiler_sha256=sha(scratch/'entry-cppgm++'),
 flags=owner['flags'],common_flags=common['flags'],
 machine=subprocess.check_output(['uname','-a'],text=True).strip(),
 host_compiler=subprocess.check_output(['g++','--version'],text=True),
 evidence={p.name:sha(p) for p in sorted(out.glob('*.json')) if p.name!='manifest.json'},
 scripts={str(p.relative_to(root)):sha(p) for p in [root/'student.tests/pa27/performance147_common.py',*sorted((root/'student.tests/pa29').glob('*179.py'))]},
 personal_inputs={str(p.relative_to(root)):sha(p) for p in sorted((root/'student.tests/pa29/source179').iterdir())},
 retained_inherited_performance={str(p.relative_to(root)):sha(p) for p in sorted((root/'student.tests/pa29/evidence178').glob('*performance.json'))})
save('manifest',manifest)
print('evidence bound:',final,'368 final timed samples; 8 launchers; 224 preliminary samples retained')
