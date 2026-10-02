#!/usr/bin/env python3
"""Audit frozen handoffs at their actual source commits; retain every sample."""
import hashlib,json,os,pathlib,statistics,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
E=ROOT/'student.tests/pa32'
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT)
def sha(data):return hashlib.sha256(data).hexdigest()
def path(p):
 p=pathlib.Path(p)
 if str(p).startswith(('/tmp/pa32-212/','/tmp/pa32-213/')):
  p=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/p.relative_to('/tmp')
 return p

def measurement(data):
 groups={}
 for r in data['runs']:
  assert r.get('status',0)==0
  groups.setdefault((r.get('workload'),r.get('mode')),[]).append(r)
 for (workload,mode),rows in groups.items():
  assert len(rows)==28
  for block in range(7):assert ''.join(r['label'] for r in rows if r['block']==block)==('AAAA' if block==0 else 'ABBA')
  s=data['summary']
  if workload:s=s[workload]
  if mode:s=s[mode]
  ratios=[statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='B')/statistics.mean(r['wall_s'] for r in rows if r['block']==b and r['label']=='A') for b in range(1,7)]
  assert s['paired_ratios']==ratios and s['paired_ratio_median']==statistics.median(ratios)
  assert s['paired_ratio_range']==[min(ratios),max(ratios)]
  aa=[r['wall_s'] for r in rows if r['block']==0];assert s['AA_range_s']==[min(aa),max(aa)]
  for label in 'AB':
   if label not in s:continue
   samples=[r for r in rows if r['block'] and r['label']==label]
   assert s[label]['median_s']==statistics.median(r['wall_s'] for r in samples)
   assert s[label]['range_s']==[min(r['wall_s'] for r in samples),max(r['wall_s'] for r in samples)]
   assert s[label]['peak_rss_kib']==max(r['peak_rss_kib'] for r in samples)
 return len(data['runs'])

def main():
 rows=[]
 for stage,commit in [(211,'a7fe2af0'),(212,'cec5b40a'),(213,'86d08255')]:
  folder=E/f'evidence{stage}';b=json.loads((folder/'binding.json').read_text())
  entries={}
  for row in git('ls-tree','-r',commit).decode().splitlines():
   h,p=row.split('\t',1);entries[p]=h.split()[0]
  for name,digest in b['sources'].items():
   resolved=name
   while entries[resolved]=='120000':resolved=os.path.normpath(os.path.join(os.path.dirname(resolved),git('show',commit+':'+resolved).decode()))
   assert sha(git('show',commit+':'+resolved))==digest,(stage,name)
  for name,digest in {**b['artifacts'],**b['binaries']}.items():assert sha(path(name).read_bytes())==digest,(stage,name)
  for check in json.loads((folder/'checks.json').read_text()):assert sha(path(check['log']).read_bytes())==check['sha256']
  count=0;files=[]
  for file in sorted(folder.glob('*.json')):
   d=json.loads(file.read_text())
   if not isinstance(d,dict) or not d.get('runs') or not d.get('summary'):continue
   for binary in d['binaries'].values():assert sha(path(binary['path']).read_bytes())==binary['sha256'],binary
   count+=measurement(d);files.append(file.name)
  assert count=={211:588,212:924,213:3024}[stage],(stage,count)
  assert git('rev-parse',commit+':pa32/tests').decode().strip()==b['contract_tree']
  rows.append(dict(stage=stage,commit=git('rev-parse',commit).decode().strip(),sources=len(b['sources']),artifacts=len(b['artifacts']),samples=count,lanes=files))
 dest=pathlib.Path(sys.argv[1]);dest.write_text(json.dumps(rows,indent=2)+'\n')
 print(json.dumps(rows))
if __name__=='__main__':main()
