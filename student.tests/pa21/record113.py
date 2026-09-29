#!/usr/bin/env python3
"""Final PA21 required gates, coverage inventory and reducer history."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
WORK,OUT=[Path(x).resolve() for x in sys.argv[1:]]
WORK.mkdir(parents=True,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True)
result=dict(entry=git('rev-parse','f3af4630').strip(),implementation=git('rev-parse','30aec177').strip(),
 compiler_sha256=sha(ROOT/'dev/cppgm++'),gates={},controls={})
def save():OUT.write_text(json.dumps(result,indent=2)+'\n')
for name,cmd in (
 ('fileAudit',['perl','scripts/cppgm_file_audit.pl','--stage','pa21','--paths','dev/src']),
 ('stageTests',['make','test-pa21']),('throughStageTests',['make','test-report-through-pa21'])):
 log=WORK/(name+'.log')
 with log.open('w') as f:p=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT)
 result['gates'][name]=dict(command=cmd,exit=p.returncode,output=log.read_text(),log_sha256=sha(log));save()
 assert p.returncode==0,(name,log.read_text())
 print(name,'PASS',flush=True)
paths=[]
for p in git('ls-files').splitlines():
 m=re.match(r'pa(\d+)/(tests/|scripts/|Makefile)',p)
 if (m and int(m[1])<=21) or p.startswith('scripts/') or p in ('Makefile','TESTING_AND_REFERENCES.md'):paths.append(p)
changed=git('diff','--name-only','f3af4630','--',*paths).splitlines()
assert changed==['pa12/tests/general/300-class-new-expression-default-constructor.ref'],changed
sources=[p for p in paths if p.startswith('pa21/tests/') and p.endswith('.t')]
assert len(sources)==116
assert git('ls-tree','-r','--name-only','f3af4630','pa21/tests').splitlines()==git('ls-files','pa21/tests').splitlines()
result['coverage']=dict(contract_paths=len(paths),only_changes=changed,required_sources={p:sha(ROOT/p) for p in sources},
 inventory_sha256=hashlib.sha256('\n'.join(p+' '+sha(ROOT/p) for p in paths).encode()).hexdigest())
for name in ('entry-controls','repaired-controls','cleanup-controls','cleanup2-controls','final-controls',
 'expanded-controls','current-controls','frozen-controls','entry-full','all-controls','access-final','allocation-abi','reference-proof'):
 path=WORK.parent/name/'results.json'
 if path.exists():result['controls'][name]=dict(sha256=sha(path),data=json.loads(path.read_text()))
for name in ('through-increment','through-final','through-recheck','through-current','stages-increment'):
 path=WORK.parent/(name+'.log')
 result[name]=dict(sha256=sha(path),output=path.read_text())
assert result['compiler_sha256']==sha(ROOT/'dev/cppgm++')==sha(WORK.parent/'compiler-final')
save()
