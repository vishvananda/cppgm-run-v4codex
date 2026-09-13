#!/usr/bin/env python3
"""Freeze lifecycle proofs, repeated-query controls and unchanged coverage."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def file(p):return dict(path=str(Path(p).resolve()),sha256=sha(p))
validation=json.loads((ROOT/'student.tests/pa14/lifecycle-validation.json').read_text())
previous=json.loads((ROOT/'student.tests/pa14/virtual-demand-proofs.json').read_text())
result=dict(harness_sha256=sha(__file__),spec=file(ROOT/'spec.md'),standard=file(ROOT/'doc/n3485.txt'),handout=file(ROOT/'pa14/README.md'),coverage=previous['coverage'],personal=previous['personal'],sources=[],artifacts=[],controls=[],checkpoints=[])
for row in result['coverage']+result['personal']:assert sha(ROOT/row['path'])==row['sha256']
for name in ['check_lifecycle_properties.py','lifecycle-properties.cc','check_lifecycle_actions.py','lifecycle-actions.cc','lifecycle-transfers.cc','check_lifecycle_definitions.py']:
 result['sources'].append(file(ROOT/'student.tests/pa14'/name))
for pattern in ['*.json','*.cpp','*-entry.cc','*-entry.lowir','*-current.lowir','properties-entry','actions-entry','transfers-entry','cppgm++-properties']:
 for path in sorted(WORK.glob(pattern)):result['artifacts'].append(file(path))
for label in ['entry','release']:
 folder=WORK/('definitions-'+label)
 for path in sorted(folder.iterdir()):result['artifacts'].append(file(path))
for label in ['release','sanitized']:
 for group in ['properties','actions','definitions']:
  folder=WORK/'validation'/(group+'-'+label)
  manifest=folder/'checks.json';rows=json.loads(manifest.read_text())
  result['controls'].append(dict(label=label,group=group,manifest=file(manifest),files=[file(p) for p in sorted(folder.iterdir())]))
for name,count in [('stage-definitions',314),('prior-definitions',1621)]:
 path=WORK/(name+'.log');assert 'ALL TESTS PASSED SUCCESSFULLY!' in path.read_text() and str(count) in path.read_text()
 result['checkpoints'].append(dict(name=name,count=count,exit_code=0,log=str(path),log_sha256=sha(path)))
result['proofs']=[
 dict(rule='N3485 12.4 [class.dtor]/3 and 15.4 [except.spec]/3-4',cases='implicit-to-throwing, throwing-to-implicit, field-to-noexcept, noexcept-to-field and template variants',proof='Each absent destructor exception specification is inferred from subobjects, independently of a preceding explicit declaration. Empty subobjects infer non-throwing; a throwing member infers throwing. Opposite redeclarations are incompatible; matching controls remain valid.'),
 dict(rule='N3485 15.4 [except.spec]/5,14; 14.7.1 [temp.inst]/1',cases='late-subobject-override, unused-body, pointer-subobject',proof='The implicitly declared derived destructor may not loosen its virtual base exception specification. A class specialization supplies destructor declarations before this query; this does not instantiate the destructor body or a pointer pointee.'),
 dict(rule='N3485 12.4 [class.dtor]/5-8; 9.2 [class.mem]/9; spec.md sections 4-5',cases='forward, template, cyclic-triviality, ctor/dtor/transfer-failure, transfer-deleted',proof='An incomplete declaration cannot establish triviality or an action omission. Invalid recursive storage or inaccessible/uninitialized subobjects terminate the owning computation. A deleted transfer is a completed negative language fact, distinct from a failed preparation. Repeated public queries preserve graph sizes and terminal state.'),
 dict(rule='N3485 14.7.1 [temp.inst]/1; 12.4 [class.dtor]/6-8; 10.3 [class.virtual]',cases='external, owner, definition, defaulted-definition, virtual-definition',proof='An external owning destructor declaration does not require the member-specialization destructor body. A defined destructor must destroy its member regardless of use in its defining translation unit. A defined polymorphic constructor must establish dispatch for objects constructed through its external entry. Host results corroborate these rule-derived reducers; they are not production dependencies.'),
]
(ROOT/'student.tests/pa14/lifecycle-proofs.json').write_text(json.dumps(result,indent=2)+'\n')
print(len(result['coverage']),'unchanged fixtures/references;',len(result['controls']),'frozen lifecycle control groups')
