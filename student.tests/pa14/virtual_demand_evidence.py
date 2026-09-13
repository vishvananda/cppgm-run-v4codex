#!/usr/bin/env python3
"""Freeze explicit virtual dependencies, source policy and coverage evidence."""
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]).resolve()
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def file(p):return dict(path=str(p),sha256=sha(p))
def run(name,cmd,expected=0):
 p=subprocess.run(list(map(str,cmd)),cwd=ROOT,capture_output=True,text=True,timeout=180)
 log=WORK/(name+'.log');log.write_text(p.stdout+p.stderr)
 assert p.returncode==expected,(cmd,p.returncode,p.stderr)
 return dict(command=list(map(str,cmd)),exit_code=p.returncode,log=str(log),log_sha256=sha(log))
result=dict(entry_commit='c1e17cdc9462422c7d973424f96f66b856ee4819',implementation_commits=['8d596c53','9ec76f55'],harness_sha256=sha(__file__),
 spec=file(ROOT/'spec.md'),standard=file(ROOT/'doc/n3485.txt'),handout=file(ROOT/'pa14/README.md'),
 policy='Preserve the existing PA13 key-definition demand policy when a selected template member definition is published during finish. This is a dependency/publication control, not a claim that C++ requires every template key definition to emit a vtable.',
 clauses=['spec.md sections 4-5: typed reasons, independent fact states and precise consumers',
          'PA13 tests/general/400-key-function-vtable-without-local-construction.t: existing key-definition emission policy',
          'N3485 [class.dtor]/12: ill-formed virtual destructor when deallocation lookup selects placement delete',
          'N3485 [temp.inst]/1: unused member bodies are not instantiated by class completion'],
 sources=[file(ROOT/'student.tests/pa14'/n) for n in ['virtual-demands.cpp','virtual-demands-pure.t','virtual-demands.cc','check_virtual_demands.py']],
 controls=[],checks=[],outputs=[],entry_observations=[],deduplication=[],
 coverage=[dict(path=str(p.relative_to(ROOT)),sha256=sha(p)) for p in sorted((ROOT/'pa14/tests').rglob('*')) if p.is_file() and '.my' not in p.name])
assert len(result['coverage'])==1266 and sum(r['path'].endswith('.t') for r in result['coverage'])==314
for mode in ['release','sanitized']:
 folder=WORK/('probes-final-'+mode)
 rows=json.loads((folder/'checks.json').read_text());assert len(rows)==6
 result['controls'].append(dict(mode=mode,binary=file(folder/'probe'),manifest=file(folder/'checks.json'),
  logs=[file(row['log']) for row in rows]))
 binary=WORK/('cppgm++-'+mode)
 source=ROOT/'student.tests/pa14/virtual-demands-pure.t';ir=WORK/('pure-'+mode+'.lowir')
 result['checks'].append(run('pure-'+mode,[binary,'--emit-lowir','-O0','--validate-lowir','-o',ir,source]))
 assert 'object=__cxa_pure_virtual' in ir.read_text()
 result['outputs'].append(file(ir))
assert len(set(o['sha256'] for o in result['outputs']))==1
# Retain the initial abstract-base test as a LowIR control. The supplied native
# backend cannot resolve its pure-virtual support symbol; the concrete-base
# companion exercises native lifecycle/dispatch and heap deletion instead.
check=run('pure-native-backend',[ROOT/'dev/lowir2native-ref','-O0','-o',WORK/'pure-native',WORK/'pure-release.lowir'],1)
assert 'undefined native symbol: pure_virtual' in Path(check['log']).read_text()
result['backend_limitation']=check
host=WORK/'pure-host'
result['checks'].append(run('pure-host-build',['g++','-x','c++','-std=c++11','-pedantic-errors',ROOT/'student.tests/pa14/virtual-demands-pure.t','-o',host]))
result['checks'].append(run('pure-host-run',[host]));result['host']=file(host)
for name in ['late-key','key-reverse','key-lifecycle','failure']:
 result['entry_observations'] += [file(WORK/(name+'.cpp')),file(WORK/(name+'-entry.json')),file(WORK/(name+'-current.json'))]
 if name!='failure':result['entry_observations'] += [file(WORK/(name+'-entry.lowir')),file(WORK/(name+'-current.lowir'))]
for name in ['native-first.log','native-caches.log','prior-caches.log','cppgm++-pre-order-fix','history-verification.log','preflight-harness.py','preflight.json','preflight-final.json']:
 result['entry_observations'].append(file(WORK/name))
for name in ['deduplicate.py','deduplication-plan.json','deduplication-complete.json','deduplication.log']:
 result['deduplication'].append(file(WORK/name))
history=WORK/'history-verification.log'
assert '14112 total frozen performance observations verified' in history.read_text()
result['historical_observations']=14112
result['personal']=[dict(path=str(p.relative_to(ROOT)),sha256=sha(p)) for p in sorted((ROOT/'student.tests/pa14').glob('*.cpp'))]
assert len(result['personal'])==35
(ROOT/'student.tests/pa14/virtual-demand-proofs.json').write_text(json.dumps(result,indent=2)+'\n')
print('12 public vtable runs, retained pure-virtual LowIR/native-boundary evidence, 314 course and 35 native source identities frozen')
