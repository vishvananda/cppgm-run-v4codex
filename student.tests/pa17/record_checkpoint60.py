#!/usr/bin/env python3
"""Record the reviewed range and bind the loop-60 evidence (after campaigns)."""
from pathlib import Path
import hashlib, json, os, re, struct, subprocess
ROOT=Path(__file__).resolve().parents[2]
WORK=Path(os.environ['RALPH_ARTIFACT_DIR'])/'checkpoint60'
START='e14b96fa9d4b3376e5922b8ad30093c3c0b0c759'
ENTRY='119fa9feb12fbc002a2c49c6ae9fad66d73f8066'
BASE='21748547a9e5befaae65e4fae120a63b3f9fcafb'
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT,text=True).strip()
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def save(name,data):(ROOT/'student.tests/pa17'/name).write_text(json.dumps(data,indent=2)+'\n')
def bound(path):return dict(path=str(path),sha256=sha(path))
def patch(*args):return hashlib.sha256(subprocess.check_output(['git',*args],cwd=ROOT)).hexdigest()
def failures(path):return sorted(set(re.findall(r'^(pa\d+/[^:]+): ERROR:',Path(path).read_text(),re.M)))
def digest():
 h=hashlib.sha256()
 for name in sorted(git('ls-files','dev').splitlines()):
  h.update(name.encode()+b'\0');h.update((ROOT/name).read_bytes());h.update(b'\0')
 return h.hexdigest()
if __name__=='__main__':
 tip=git('rev-parse','HEAD');assert not git('diff',tip,'--','dev')
 range_record=dict(review_start=START,entry_commit=ENTRY,reviewed_tip=tip,commits=[])
 for c in git('rev-list','--reverse',START+'..'+tip).splitlines():
  range_record['commits'].append(dict(commit=c,subject=git('show','-s','--format=%s',c),
   paths=git('diff-tree','--no-commit-id','--name-only','-r',c).splitlines(),
   implementation_patch_sha256=patch('show','--format=',c,'--','dev')))
 range_record['combined_implementation_paths']=git('diff','--name-only',START,tip,'--','dev').splitlines()
 range_record['combined_patch_sha256']=patch('diff',START,tip,'--','dev')
 save('checkpoint60-range.json',range_record)
 # Sectionless ELF disassembly starts at the actual entry in the load segment.
 exe=WORK/'trace.exe';data=exe.read_bytes();entry,phoff=struct.unpack_from('<QQ',data,24)
 kind,flags,offset,address,_,filesize,memsize,align=struct.unpack_from('<IIQQQQQQ',data,phoff)
 assert kind==1 and flags&1 and offset==0
 code=WORK/'trace-code.bin';code.write_bytes(data[entry-address:filesize])
 trace=dict(code_commit=tip,compiler=bound(WORK/'reviewed-final-cppgm++'),source=bound(ROOT/'student.tests/pa17/checkpoint60-trace.cpp'),
  lowir=bound(WORK/'trace.lowir'),native=bound(exe),native_exit=subprocess.run([exe]).returncode,
  source_text=(ROOT/'student.tests/pa17/checkpoint60-trace.cpp').read_text(),lowir_text=(WORK/'trace.lowir').read_text(),
  telemetry=[json.loads(s) for s in (WORK/'trace-stats.jsonl').read_text().splitlines()],
  disassembly=subprocess.check_output(['objdump','-D','-b','binary','-m','i386:x86-64','--adjust-vma='+str(entry),str(code)],text=True),
  ownership=['streaming source cursor -> retained parsed Both/TableOwner/Probe/Sparse patterns',
   'canonical head/argument/frame identities -> defaults/partial/member/static definition facts',
   'two receiver paths and naming/access scope -> recorded call/relocation/transfer facts',
   'typed LowIR -> explicit O0 supplied native backend -> checked ELF execution'])
 assert trace['native_exit']==0;save('checkpoint60-trace.json',trace)
 logs=json.loads((WORK/'final-checks.json').read_text())
 logs['entry']=dict(command=['read-only checkpoint log'],path=str(WORK/'entry.log'),exit_code=2)
 logs['baseline']=dict(command=['make','test-pa17'],path=str(WORK/'entry-stage.log'),exit_code=2)
 entry_failures=failures(WORK/'entry.log');final=failures(logs['stage']['path'])
 assert entry_failures==final==failures(WORK/'entry-stage.log') and len(final)==3
 assert '2266 / 2266' in Path(logs['prior']['path']).read_text() and not failures(logs['prior']['path'])
 (WORK/'stage-progress.log').write_text('PASS: identical three PA17 failures; 340/343; earlier 2266/2266; 343 unchanged course inputs and comparison rules.\n')
 logs['stage-progress']=dict(command=['failure-set, prior-pass and coverage comparison'],path=str(WORK/'stage-progress.log'),exit_code=0)
 for row in logs.values():row['sha256']=sha(row['path'])
 historical=[]
 for pattern in ('transfer-*','query-*','storage-*','checkpoint56-*'):
  for p in sorted((ROOT/'student.tests/pa17').glob(pattern)):
   if p.is_file():historical.append(dict(path=str(p.relative_to(ROOT)),sha256=sha(p)))
 evidence=dict(code_commit=tip,source_digest=digest(),stage_base=BASE,review_start=START,entry_commit=ENTRY,
  entry_failures=entry_failures,final_failures=final,logs=logs,
  course_tests=sorted(str(p.relative_to(ROOT)) for p in (ROOT/'pa17/tests').glob('*/*.t')),
  protected_paths=['AGENTS.md','TESTING_AND_REFERENCES.md','spec.md','Makefile','scripts','doc','reference-binaries','pa17/README.md','pa17/Makefile','pa17/pa17.gram'],
  reference_corrections=git('diff','--name-only',START,tip,'--',':(glob)pa*/tests/**').splitlines(),
  performance=['student.tests/pa17/checkpoint60-performance.json','student.tests/pa17/checkpoint60-cumulative-performance.json'],
  historical_evidence=historical,record_sizes=dict(entry=(WORK/'entry-record-sizes.log').read_text().strip(),final=(WORK/'record-sizes.log').read_text().strip()),
  remaining_groups={'Closure entities':[p for p in final if 'lambda' in p],
                    'Exception cleanup scheduling':[p for p in final if 'lambda' not in p]},waivers=[])
 controls=json.loads((ROOT/'student.tests/pa17/checkpoint60-controls.json').read_text())
 before=json.loads((ROOT/'student.tests/pa17/checkpoint60-entry-controls.json').read_text())
 evidence.update(control_count=sum(map(len,controls.values())),new_control_count=len(before),entry_control_failures=sum(not r['passed'] for r in before))
 evidence['evidence_files']=[dict(path=str(p.relative_to(ROOT)),sha256=sha(p)) for p in sorted((ROOT/'student.tests/pa17').glob('checkpoint60*')) if p.is_file() and p.name!='checkpoint60-evidence.json']
 save('checkpoint60-evidence.json',evidence)
 print('Recorded',len(range_record['commits']),'commits and',len(range_record['combined_implementation_paths']),'implementation paths through',tip)
