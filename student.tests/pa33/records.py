#!/usr/bin/env python3
"""Bind/verify implementation evidence without advancing independent review markers."""
import hashlib,json,os,pathlib,re,shutil,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2]
out=root/'student.tests/pa33/evidence219'
art=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'pa33-219'
base='676b6e328c7a05334b15f1c9ee1ec30c783e9aff'
code='cbf6f115'
def git(*args):return subprocess.check_output(['git',*args],cwd=root,text=True).strip()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text())
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n')
lanes={'affected':'performance2','rejected-suffix':'strings3','suffix-ab':'suffix-ab','common-o0':'common-o0','common-o2':'common-o2','selfhost':'selfhost'}
checks=[
 ('prior','n=33; if [ "$n" -le 1 ]; then echo "===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====="; else make test-report-through-pa$((n - 1)); fi','prior-final.log','(5397 / 5397)'),
 ('stage','make test-pa33','stage-final.log','(57 / 57)'),
 ('debug','make -C pa33 test-debuginfo','debug-final.log','PASS (1/1)'),
 ('file','perl scripts/cppgm_file_audit.pl --stage pa33 --paths dev/src','file-final.log','File audit passed for pa33'),
 ('through','make test-report-through-pa33','through-final.log','(5454 / 5454)'),
 ('personal','python3 student.tests/pa33/check_native.py','personal-final.log','PASS (4 levels)'),
 ('budgets','python3 student.tests/pa33/check_budgets.py','budgets-final.log','PASS (4 levels, 4 negative boundaries)')]
def check_logs():
 result=[]
 for name,command,path,marker in checks:
  p=art/path;s=p.read_text();assert marker in s,(name,s)
  assert not re.search(r'ERROR:|\bFAIL\b|\*\*\*.*Error',s),(name,s)
  if name in ['stage','through']:
   assert 'native driver programs: PASS (18/18)' in s
   assert 'PA24 native contract properties: PASS (5/5)' in s
  if name=='debug':assert s.count('PASS (5/5)')==2
  result.append(dict(name=name,command=command,exit_code=0,log=str(p),sha256=sha(p)))
 return result

def report():
 lines=['# Frozen compiler and runtime measurements','',
 'B/A paired medians, with the range of all six ABBA ratios in brackets. Compiler and runtime wall times are separate sample medians; compiler RSS is the maximum in paired blocks. Every input, compiler and checked image has a retained hash. A/A calibration and every observation remain in the linked JSON. No course timing gate is introduced.','']
 for name in lanes:
  d=read(out/(name+'.json'));summary=d['summary']
  if name=='selfhost':summary={'component':{'compile':summary}}
  lines += ['## '+name,'','[All observations]('+name+'.json'+')','','| Workload | Compiler B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |','| --- | --- | --- | --- | --- | --- | --- |']
  for workload,modes in summary.items():
   images=d['images'] if name=='selfhost' else d['images'][workload]
   def ratio(s):return '%.3f [%.3f–%.3f]'%(s['paired_ratio_median'],*s['paired_ratio_range'])
   def pair(s,key,scale=1):return '/'.join(('%.2f'%(s[k][key]*scale)) if scale!=1 else str(s[k][key]) for k in 'AB')
   c=modes['compile'];r=modes.get('runtime');text='/'.join(str(images[k].get('object_text_bytes',images[k].get('text_bytes'))) for k in 'AB')
   lines.append('| '+' | '.join([workload,ratio(c),pair(c,'median_s',1000),pair(c,'peak_rss_kib'),ratio(r) if r else 'N/A',pair(r,'median_s',1000) if r else 'N/A',text])+' |')
  lines.append('')
 (out/'performance.md').write_text('\n'.join(lines))

if sys.argv[1]=='bind':
 assert not git('diff',code,'--','dev'),'unmeasured production changes'
 for lane,path in lanes.items():shutil.copyfile(art/path/'performance.json',out/(lane+'.json'))
 report();write(out/'checks.json',check_logs())
 shutil.copyfile(art/'common-commands.json',out/'common-commands.json')
 sources=set(git('ls-files','dev').splitlines())
 sources.update(str(p.relative_to(root)) for p in (root/'student.tests/pa33').glob('*') if p.is_file())
 sources.update(['spec.md','AGENTS.md','TESTING_AND_REFERENCES.md','pa33/README.md','pa33/plan.md'])
 binding=dict(stage_base=base,last_reviewed_commit=base,implementation_commit=git('rev-parse',code),
  sources={p:sha(root/p) for p in sorted(sources)},
  evidence={p.name:sha(p) for p in sorted(out.iterdir()) if p.is_file() and p.name!='binding.json'},
  artifacts={str(p):sha(p) for p in sorted(art.rglob('*')) if p.is_file() and p.name!='verify.log'},
  fixture_trees={f'pa{i}/tests':git('rev-parse',f'HEAD:pa{i}/tests') for i in range(1,34)},
  unfinished_implementation=[],independent_review=['Whole-stage architecture/performance and accumulated ABI/debug audit remains required; implementation does not advance review markers.'],
  entry_status={'prompt_passed':47,'prompt_total':73,'reported_mir_failures':10,'controls_stopped_at_first_failure':True},
  final_status={'mir_behavior':57,'native_controls':5,'native_driver_modes':18,'debug':11,'prior':5397,'through':5454})
 write(out/'binding.json',binding)
else:
 b=read(out/'binding.json');assert b['stage_base']==base and b['last_reviewed_commit']==base
 assert not git('diff',b['implementation_commit'],'--','dev')
 assert not git('diff',base,'--','scripts',*[f'pa{i}/tests' for i in range(1,34)])
 for key,prefix in [('sources',root),('evidence',out),('artifacts',pathlib.Path('/'))]:
  for p,digest in b[key].items():assert sha(prefix/p)==digest,p
 assert check_logs()==read(out/'checks.json')
 for tool in ['cppgm++','lowir2native']:
  assert sha(root/'dev'/tool)==sha(art/'final'/tool)==sha(art/'candidate2'/tool)
 for path,tree in b['fixture_trees'].items():assert git('rev-parse','HEAD:'+path)==tree
 for lane in lanes:
  d=read(out/(lane+'.json'))
  for binary in d['binaries'].values():assert sha(pathlib.Path(binary['path']))==binary['sha256']
  assert all(row['status']==0 for row in d['runs'])
  assert len(d['runs'])>=28
  summaries=d['summary'] if lane!='selfhost' else {'component':{'compile':d['summary']}}
  for modes in summaries.values():
   for s in modes.values():assert len(s['paired_ratios'])==6
 if '--clean' in sys.argv:assert not git('status','--short')
 print('PA33 implementation evidence: verified; independent review remains pending')
