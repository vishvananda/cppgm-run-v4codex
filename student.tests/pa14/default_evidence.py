#!/usr/bin/env python3
"""Freeze default fact legality, demand controls and unchanged course coverage."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def file(p):return dict(path=str(Path(p).resolve()),sha256=sha(p))
previous=json.loads((ROOT/'student.tests/pa14/lifecycle-proofs.json').read_text())
result=dict(harness_sha256=sha(__file__),spec=file(ROOT/'spec.md'),standard=file(ROOT/'doc/n3485.txt'),handout=file(ROOT/'pa14/README.md'),coverage=previous['coverage'],personal=previous['personal'],sources=[],artifacts=[],controls=[],checkpoints=[])
for row in result['coverage']+result['personal']:assert sha(ROOT/row['path'])==row['sha256']
for name in ['check_default_facts.py','default-facts.cc','check_default_demand_states.py','default-demand-states.cc','check_default_demands.py','default-fact-exploration.py']:
 result['sources'].append(file(ROOT/'student.tests/pa14'/name))
for pattern in ['*.json','*.log','cppgm++-*']:
 for path in sorted(WORK.glob(pattern)):
  if not path.name.startswith(('verify','verification','evidence')):result['artifacts'].append(file(path))
for folder in ['entry','current','current-demands','demand-entry','demands-entry','candidate-defaults','fixed-defaults','facts-conversions','states-list','access-entry','access-release']:
 for path in sorted((WORK/folder).glob('*')):
  if path.is_file():result['artifacts'].append(file(path))
for label in ['release','sanitized']:
 for group in ['facts','states','defaults']:
  folder=WORK/'validation-access'/(group+'-'+label)
  result['controls'].append(dict(label=label,group=group,manifest=file(folder/'checks.json'),files=[file(p) for p in sorted(folder.iterdir()) if p.is_file()]))
for name,count in [('stage-access',314),('prior-access',1621)]:
 path=WORK/(name+'.log');assert 'ALL TESTS PASSED SUCCESSFULLY!' in path.read_text() and str(count) in path.read_text()
 result['checkpoints'].append(dict(name=name,count=count,exit_code=0,log=str(path),log_sha256=sha(path)))
result['proofs']=[
 dict(rule='N3485 8.3.6 [dcl.fct.default]/5; 8.5 [dcl.init]; 8.5.3 [dcl.init.ref]; 8.5.4 [dcl.init.list]',cases='reference, pointer, explicit, narrowing, private-conversion, list-conversion-identity',proof='Defaults satisfy parameter-type copy-initialization at their declaration environment. Invalid reference binding, incompatible pointer conversion, an explicit converting constructor, narrowing or inaccessible conversion cannot be repaired by call-site materialization. Each runtime use owns its own temporary/conversion identity.'),
 dict(rule='N3485 14.5 [temp.decls]/2; 14.7.1 [temp.inst]/10,12-13',cases='function, member, storage, used-default-bad, nested-query, unused-ordinary, unused-fixed-function, unused-fixed-constructor, nested-defaults, constructor-recipe',proof='Each default is a separate definition. Its conversion may be checked without instantiating function/member/storage definitions until the default is used. A use inside decltype still instantiates this separate initializer; a sizeof operand inside the initializer is itself unevaluated. Dependency publication and subsequent body checking have separate terminal states. GCC eager rejection of unused-ordinary conflicts with paragraph 10; Clang observations agree, but the rule is the proof.'),
 dict(rule='N3485 3.3.7 [basic.scope.class]/1; 9.2 [class.mem]/2; 14.7.1 [temp.inst]/1; 8.5.4 [dcl.init.list]/3; 13.3 [over.match]',cases='later-member, nested-later-member, self-default, selected-list-bad, unselected-list-bad, list-class-completion, incomplete-reference; inherited demand-regions.cpp',proof='Class defaults see the complete enclosing class, including later members. A class specialization must supply its definition before choosing aggregate or constructor list initialization. Candidate ranking uses supplied arguments; only a selected constructor requires its defaults. Direct reference binding does not require the referred class definition. Local-class template members retain unused dependent defaults.'),
 dict(rule='N3485 8.3.6 [dcl.fct.default]/5; 11 [class.access]',cases='fixed-private-default, list-private-default',proof='The declaring friend class can use its private conversion in a default. A fixed template call or list constructor must consume the checked recipe; a second access check in the caller scope incorrectly rejects both valid programs.'),
 dict(rule='N3485 3.2 [basic.def.odr]/3; 12.8 [class.copy]',cases='elided-template-copy-bad, unused-template-copy-bad; inherited fixed-calls.cpp',proof='A selected copy constructor is odr-used even when its runtime call is elided. Deferred template bodies must be checked when the default is used. Ordinary bodies were already checked; marking an elided ordinary copy for emission adds an unnecessary entry. The final compiler retains all 349 entry/current output hashes without skipping deferred template checks.'),
 dict(rule='spec.md sections 2,4,5,6,8,9',cases='seven conversion probes and three dependency/body/isolation probes, 10000 requests each under release and ASan/UBSan',proof='Declaration slots remain immutable. Concrete function-specialization and parameter-slot identity own checked conversion success/failure. Member/specialization/storage/nested-default edges are published once; completed dependency registration does not claim that deferred bodies are valid. Repeated success and failure preserve node/entity sizes. A TU owns flat indices, geometric fact/edge arenas and complete-class queue intervals.'),
]
(ROOT/'student.tests/pa14/default-proofs.json').write_text(json.dumps(result,indent=2)+'\n')
print(len(result['coverage']),'unchanged fixtures/references;',len(result['controls']),'frozen default control groups')
