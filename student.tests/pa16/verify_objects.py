#!/usr/bin/env python3
"""Verify the incomplete PA16 object/address handoff without changing fixtures."""
from pathlib import Path
import hashlib,json,re,subprocess
ROOT=Path(__file__).resolve().parents[2]
CHECKPOINT=ROOT/'student.tests/pa16/objects-checkpoint.json'
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def run(*args):return subprocess.check_output(args,cwd=ROOT,text=True).strip()
def inventory(paths):
 names=subprocess.check_output(['git','ls-files','-z','--',*paths],cwd=ROOT).decode().split('\0')
 return {name:sha(ROOT/name) for name in names if name}
def fingerprint(data):return hashlib.sha256(json.dumps(data,sort_keys=True,separators=(',',':')).encode()).hexdigest()
def failures(text):return sorted(set(re.findall(r'^(pa16/tests/[^:]+\.t): ERROR:',text,re.M)))
def score(text):
 values=re.findall(r'(?:TEST SUMMARY:|ALL TESTS PASSED SUCCESSFULLY! \()(\d+)\s*/\s*(\d+)',text)
 if not values:values=re.findall(r'(\d+)\s*/\s*(\d+)\s+TESTS PASSED',text)
 return tuple(map(int,values[-1]))
def verify():
 c=json.loads(CHECKPOINT.read_text())
 assert fingerprint(inventory(['dev']))==c['implementation_sha256']
 fixtures=inventory([f'pa{i}/tests' for i in range(1,17)])
 assert len(fixtures)==c['contract_files'] and fingerprint(fixtures)==c['contract_sha256']
 assert not run('git','diff','--name-only',c['entry_commit'],'--',*[f'pa{i}/tests' for i in range(1,17)])
 for path,digest in c['artifacts'].items():assert sha(path)==digest,path
 for path,digest in c['harnesses'].items():assert sha(ROOT/path)==digest,path
 assert sha(ROOT/'dev/cppgm++')==c['compiler_sha256']
 base=Path(c['artifact_directory']);entry=(base/'entry-tests.log').read_text();final=(base/'stage-final.log').read_text()
 assert score(entry)==(93,154) and score(final)==(146,154)
 before=set(failures(entry));after=set(failures(final))
 assert len(before)==61 and len(after)==8 and len(before-after)==54 and len(after-before)==1
 assert sorted(after)==c['remaining_failures'] and sorted(after-before)==c['new_output_mismatches']
 assert score((base/'prior-final.log').read_text())==(2112,2112)
 assert score((base/'through-final.log').read_text())==(2258,2266)
 assert 'File audit passed for pa16' in (base/'file-audit-final.log').read_text()
 rows=json.loads((base/'controls/results.json').read_text());assert len(rows)==34 and all(r['passed'] for r in rows)
 total=0
 for name in ['performance.json','scalar-fast-performance.json','literal-demand-performance.json']:
  p=json.loads((base/name).read_text())
  for b in p['binaries']:assert sha(b['path'])==b['sha256']
  for w in p['workloads'].values():
   assert sha(w['source_path'])==w['source_sha256']
   for output in w['outputs']:
    assert sha(output['path'])==output['sha256']
    if 'native' in output:assert sha(output['native']['path'])==output['native']['sha256']
   if w['comparison']=='exact':
    assert len({o['sha256'] for o in w['outputs']})==1
    if 'runtime' in w:assert len({o['native']['sha256'] for o in w['outputs']})==1
   else:assert w['entry_probe']['exit_code']!=0
   for key in ['compiler','runtime']:
    if key in w:total+=len(w[key]['observations'])+len(w[key]['warmups'])
 p=json.loads((base/'scalar-fast-control.json').read_text())
 for w in p['workloads'].values():total+=len(w['observations'])+len(w['warmups'])
 assert total==804
 print('PA16 object handoff: 54 old failures resolved, 1 new output mismatch (net 53); coverage unchanged; prior-through and file audit pass; 804 observations verified.')
if __name__=='__main__':verify()
