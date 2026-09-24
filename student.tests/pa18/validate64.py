#!/usr/bin/env python3
"""Record loop 64's required checks, unchanged coverage and executable controls."""
from pathlib import Path
import hashlib,json,os,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(sys.argv[1]);WORK.mkdir(parents=True,exist_ok=True)
ENTRY='30a610065e586c481a094c60698f7ae8a7952904'
BASE='94dcb8ad21664137e87d574e878c14a4a047348a'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(command):return subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=900)
e=dict(stage_base=BASE,last_reviewed=BASE,entry_commit=ENTRY,implementation_commit=run(['git','rev-parse','HEAD']).stdout.strip(),checks={})
for name,command in [('stageTests',['make','test-pa18']),('priorThroughTests',['make','test-report-through-pa17']),('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa18','--paths','dev/src']),('throughStage',['make','test-report-through-pa18'])]:
 r=run(command);log=WORK/(name+'.log');log.write_text(r.stdout+r.stderr)
 e['checks'][name]=dict(command=command,exit=r.returncode,log=str(log),log_sha256=sha(log),summaries=[l for l in log.read_text().splitlines() if 'SUMMARY:' in l or 'ALL TESTS PASSED' in l or 'File audit' in l]);print(name,r.returncode,e['checks'][name]['summaries'][-3:],flush=True)
log=(WORK/'stageTests.log').read_text()
remaining={l.split(': ERROR:')[0]:l.split(': ERROR:')[1].strip() for l in log.splitlines() if '.t: ERROR:' in l}
baseline=json.loads((Path(os.environ['RALPH_ARTIFACT_DIR'])/'baseline-failures.json').read_text());prior={r['source'] for r in baseline}
counts=re.findall(r'TEST SUMMARY: (\d+) / (\d+) TESTS PASSED',log);passing,total=map(int,counts[-1])
e['stage']=dict(entry_passing=282,entry_total=420,final_passing=passing,final_total=total,fixed=sorted(prior-set(remaining)),new_failures=sorted(set(remaining)-prior),remaining=remaining)
e['controls']={}
for name,script in [('substitution','substitution_controls.py'),('ordering','ordering_controls.py')]:
 work=WORK/name;r=run(['python3','student.tests/pa18/'+script,'dev/cppgm++',str(work)]);(WORK/(name+'.log')).write_text(r.stdout+r.stderr)
 e['controls'][name]=json.loads((work/'results.json').read_text());print(name,r.returncode,flush=True)
e['controls']['increment_abi']=json.loads(Path('/tmp/pa18-loop64-increment-abi/results.json').read_text())
rows=[]
for i,name in enumerate(e['stage']['fixed']):
 src=ROOT/name;ir=WORK/(str(i)+'.lowir');exe=WORK/(str(i)+'.exe');row=dict(path=name,source_sha256=sha(src))
 r=run(['dev/cppgm++','--emit-lowir','-O0','--validate-lowir','-o',str(ir),name]);row.update(compiler_exit=r.returncode,diagnostic=r.stderr)
 native=bool(re.search(r'\bmain\s*\(',src.read_text()));row['oracle']='native-exit' if native else 'validated-lowir'
 # This fixture returns its chosen overload's value directly; success is 2.
 row['expected_native_exit']=2 if name.endswith('/300-using-declaration-imports-member-template-sfinae-shadow.t') else 0
 if r.returncode==0 and native:
  b=run(['dev/lowir2native-ref','-O0','-o',str(exe),str(ir)]);row.update(backend_exit=b.returncode,backend_diagnostic=b.stderr)
  if b.returncode==0:row['native_exit']=run([str(exe)]).returncode
 row['passed']=r.returncode==0 and (not native or row.get('native_exit')==row['expected_native_exit']);rows.append(row)
e['controls']['course_execution']=rows
paths=run(['git','diff','--name-only',ENTRY,'--','dev']).stdout.splitlines()
e['files']={p:sha(ROOT/p) for p in paths}
e['evidence_files']={str(p.relative_to(ROOT)):sha(p) for p in (ROOT/'student.tests/pa18').glob('*64.py')}
assert not run(['git','diff',ENTRY,'--','pa18/tests','pa18/Makefile','pa18/scripts','scripts','Makefile']).stdout
e['coverage_unchanged']=True
out=ROOT/'student.tests/pa18/loop64-evidence.json';out.write_text(json.dumps(e,indent=2)+'\n')
assert e['checks']['priorThroughTests']['exit']==e['checks']['fileAudit']['exit']==0
assert passing>282 and total==420 and not e['stage']['new_failures']
assert all(r['passed'] for rows in e['controls'].values() for r in rows)
print('Validated implementation handoff:',passing,'/420;',len(e['stage']['fixed']),'fixed, no new failures.',flush=True)
