#!/usr/bin/env python3
"""Verify implementation188 handoff evidence and bind it to committed sources."""
import collections,hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[2];scratch=pathlib.Path(sys.argv[1]).resolve()
out=root/'student.tests/pa29/evidence188';out.mkdir(exist_ok=True)
entry='c1caa0fc6b2985a633c4db587097ac5c08c30c75'
implementation='bf8bc00f66e6ef9c9a0fec7da71d2486ef4ba1c7'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def digest(s):return hashlib.sha256(s.encode()).hexdigest()
def save(name,v):(out/(name+'.json')).write_text(json.dumps(v,indent=2)+'\n')
common=json.loads((scratch/'common-final/performance.json').read_text());owner=json.loads((scratch/'owner-final/performance.json').read_text())
assert len(common['runs'])==224 and len(owner['runs'])==216 and len(owner['launchers'])==8
assert sha(root/'dev/cppgm++')==common['binaries']['B']['sha256']==owner['binaries']['B']['sha256']
assert common['binaries']['A']['sha256']==json.loads((root/'student.tests/pa29/evidence187/source-binding.json').read_text())['binaries']['B']['sha256']
for name,d in [('common',common),('owner',owner)]:
 assert all(v['status']==0 for v in d['runs'])
 for v in d['binaries'].values():assert sha(pathlib.Path(v['path']))==v['sha256']
 save(name+'-performance',d)
 old=json.loads((out/('preliminary-'+name+'-performance.json')).read_text())
 assert old['images']==d['images']
 assert len(old['runs'])==len(d['runs'])
 for v in old['binaries'].values():assert sha(pathlib.Path(v['path']))==v['sha256']
for name,images in common['images'].items():
 assert images['A']==images['B'],name
 for mode in ['compile','runtime']:
  rows=[x for x in common['runs'] if x['workload']==name and x['mode']==mode]
  assert ''.join(x['label'] for x in rows)=='AAAA'+'ABBA'*6
for name,v in owner['inputs'].items():
 assert digest(v['source'])==v['sha256']
 if name.startswith('names'):
  assert owner['images'][name]['A']==owner['images'][name]['B']
  for mode in ['compile','runtime']:
   rows=[x for x in owner['runs'] if x['workload']==name and x['mode']==mode]
   assert ''.join(x['label'] for x in rows)=='AAAA'+'ABBA'*6
 else:assert v['entry_rejection']['status']!=0
scaling=[]
for x in owner['runs']:
 name=x['workload']
 if not name.startswith('contextual') or x['mode']!='compile':continue
 n=owner['inputs'][name]['N'];c={k:v for row in x['phase_counters'] for k,v in row.items()}
 expected=dict(tokens=45*n+139,parsed_nodes=80*n+220,nodes=80*n+220,
 semantic_template_initializer_binding_work=2*n,semantic_template_binding_work=38*n,semantic_template_bindings=8*n,
 semantic_specializations=0,template_body_transitions=0,instructions=73,operands=118,native_instructions=104,native_functions=4,text_bytes=460)
 assert all(c[k]==v for k,v in expected.items()),(name,c)
 scaling.append(dict(workload=name,trial=x['trial'],asserted_counters=expected))
assert len(scaling)==24
assert len({json.dumps(v,sort_keys=True) for k,v in owner['images'].items() if k.startswith('contextual')})==1
save('scaling',dict(final_observations=440,preliminary_observations=440,final_launchers=8,preliminary_launchers=8,checks=scaling,equivalent_pairs=7,program_images_unchanged_by_refinement=True))
checks=json.loads((scratch/'controls-final/checks.json').read_text());assert len(checks)==61
assert all((x['status']==0)==x['expected_success'] for x in checks)
assert sum(not x['expected_success'] for x in checks)==22
save('controls',dict(commands=[dict(command=x['command'],status=x['status'],expected_success=x['expected_success'],stdout_sha256=digest(x['stdout']),stderr_sha256=digest(x['stderr'])) for x in checks],
 raw_path=str(scratch/'controls-final/checks.json'),raw_sha256=sha(scratch/'controls-final/checks.json'),files={str(p.relative_to(root)):sha(p) for p in sorted((root/'student.tests/pa29/controls188').glob('*.cpp'))}))
artifacts={str(p.relative_to(scratch)):dict(bytes=p.stat().st_size,sha256=sha(p)) for p in sorted((scratch/'controls-final').iterdir()) if p.suffix in ['.ast','.lowir','.canonical']}
save('inspection',dict(artifacts=artifacts,precedence_assertions=5,unemitted_coroutine_symbols=True,unreachable_throw_tail_has_no_call=True))
prior=(scratch/'prior-final.log').read_text();stage=(scratch/'stage-final.log').read_text();through=(scratch/'through-final.log').read_text();audit=(scratch/'file-audit-final.log').read_text()
assert 'ALL TESTS PASSED SUCCESSFULLY! (4538 / 4538)' in prior
assert 'TEST SUMMARY: 395 / 403 TESTS PASSED' in stage
assert 'TEST SUMMARY: 4933 / 4941 TESTS PASSED' in through
assert 'File audit passed for pa29 with 4 warning(s).' in audit
failures=re.findall(r'^(pa29/tests/[^:]+): ERROR: (.*)$',stage,re.M);assert len(failures)==8
assert failures==re.findall(r'^(pa29/tests/[^:]+): ERROR: (.*)$',through,re.M)
old=json.loads((root/'student.tests/pa29/evidence187/remaining.json').read_text());by_name={x['test']:x for x in old['failures']}
fixed=sorted(set(by_name)-{x[0] for x in failures});assert fixed==['pa29/tests/compile/700-hosted-coroutine-contextual-operators-compile.t']
remaining=[]
for name,diagnostic in failures:
 v=by_name[name];v['diagnostic']=[diagnostic];v['diagnostic_provenance']='Implementation188 final PA29 and root-through reports.'
 p=(root/name).with_suffix('.my.stdout')
 if name.startswith('pa29/tests/compile/') and p.exists():v['compiler_diagnostic']=p.read_text()
 remaining.append(v)
save('remaining',dict(total=403,passed=395,failed=8,entry_failures=9,fixed=fixed,unfinished_implementation=5,independent_contract_questions=3,failures=remaining))
coverage_path=root/'student.tests/pa29/evidence186/coverage.json';coverage=json.loads(coverage_path.read_text());assert len(coverage['files'])==1707
for path,d in coverage['files'].items():assert sha(root/path)==d,path
inputs=subprocess.check_output(['git','ls-files','pa29/tests'],cwd=root,text=True).splitlines();assert sum(p.endswith('.t') for p in inputs)==403
assert not subprocess.check_output(['git','diff',entry,'--','pa29/tests','pa29/scripts','pa29/Makefile','TESTING_AND_REFERENCES.md'],cwd=root)
save('coverage',dict(entry=entry,unchanged=True,total_stage_inputs=403,contract_harness_files=1707,predecessor_manifest=str(coverage_path.relative_to(root)),predecessor_manifest_sha256=sha(coverage_path),compared_all_hashes=True))
paths=subprocess.check_output(['git','diff','--name-only',entry,implementation,'--','dev'],cwd=root,text=True).splitlines();sources={}
for path in paths:
 data=subprocess.check_output(['git','show',implementation+':'+path],cwd=root);assert hashlib.sha256(data).hexdigest()==sha(root/path);sources[path]=sha(root/path)
assert not subprocess.check_output(['git','diff',implementation,'--','dev'],cwd=root)
save('source-binding',dict(entry=entry,implementation=implementation,binaries=common['binaries'],files=sources))
validation=json.loads((scratch/'validation.json').read_text());assert [v['status'] for v in validation]==[0,2,2,0]
for v in validation:assert sha(pathlib.Path(v['path']))==v['sha256']
save('validation',dict(checks=validation,priorThroughTests='pass:4538/4538',fileAudit='pass',stageProgress='pass:9 -> 8 original failures; coverage unchanged',stageTests='395/403; exit 2',rootThrough='4933/4941; only PA29 fails'))
# Render the four-dimensional report directly from retained observations.
compiler_sizes={}
for label,path in [('entry',scratch/'entry-cppgm++'),('initial',scratch/'preliminary-cppgm++'),('final',scratch/'final-cppgm++')]:
 words=subprocess.check_output(['size',str(path)],text=True).splitlines()[1].split();compiler_sizes[label]=dict(text=int(words[0]),data=int(words[1]),bss=int(words[2]),sha256=sha(path))
save('performance-summary',dict(common=common['summary'],owner=owner['summary'],compiler_sizes=compiler_sizes))
lines=['# Implementation188 performance evidence','',
 'Frozen entry `c1caa0fc` versus final source `bf8bc00f`; the first implementation',
 '`e52f4586` is retained separately. O0, `-c --stats`, the same hashed inputs,',
 'host g++ link, CPU 0 affinity, wall time and `/usr/bin/time` peak RSS. Each',
 'equivalent workload has four A/A samples then six ABBA blocks, separately for',
 'compilation and checked execution. Corrected-only workloads have eight repeats',
 'per mode and size. The 880 observations and 16 startup samples are all retained.',
 '',
 '[Final common](../student.tests/pa29/evidence188/common-performance.json),',
 '[final owner](../student.tests/pa29/evidence188/owner-performance.json),',
 '[preliminary common](../student.tests/pa29/evidence188/preliminary-common-performance.json),',
 '[preliminary owner](../student.tests/pa29/evidence188/preliminary-owner-performance.json).',
 'Input hashes, compiler hashes, raw counters, every sample and A/A ranges are in',
 'those records. No outlier was discarded. All generated programs check runtime',
 'inputs/results; the loops cannot become a constant-folded/dead workload.',
 '',
 '## Equivalent correct inputs','',
 'Times are medians in milliseconds; RSS is maximum KiB. Ratios are B/A paired',
 'block medians with min–max block spread. Text is executable .text bytes.',
 '',
 '| Workload | Compile A/B ms | RSS A/B KiB | Compile ratio [spread] | Runtime A/B ms | Runtime ratio [spread] | Text A=B |',
 '|---|---:|---:|---|---:|---|---:|']
def ratio(x):return f"{x['paired_ratio_median']:.3f} [{x['paired_ratio_range'][0]:.3f}, {x['paired_ratio_range'][1]:.3f}]"
for d,names in [(common,list(common['summary'])),(owner,[x for x in owner['summary'] if x.startswith('names')])]:
 for name in names:
  c=d['summary'][name]['compile'];t=d['summary'][name]['runtime'];im=d['images'][name]['B'];text=im.get('executable_text_bytes',im.get('text_bytes'))
  lines.append(f"| {name} | {1000*c['A']['median_s']:.2f}/{1000*c['B']['median_s']:.2f} | {c['A']['peak_rss_kib']}/{c['B']['peak_rss_kib']} | {ratio(c)} | {1000*t['A']['median_s']:.2f}/{1000*t['B']['median_s']:.2f} | {ratio(t)} | {text} |")
lines+=['','All seven object and executable A/B pairs are byte-identical. The same images',
 'also survive the source refinement unchanged. Common inputs exercise templates,',
 'memory/loops, floating calls, exceptions and pruning. The names family exercises',
 'complete-class lookup of later ordinary contextual identifiers, comma lists and',
 'enumerators, with 600/1200/2400 unused classes and a checked 3,000,000-step loop.',
 '',
 'The preliminary names-family paired compile medians were '+', '.join(f"{json.loads((out/'preliminary-owner-performance.json').read_text())['summary'][name]['compile']['paired_ratio_median']:.3f}" for name in ['names600','names1200','names2400'])+'.',
 'The final implementation uses interned IDs for contextual recognition and skips',
 'duplicate declarator indexing. These remove avoidable work. Remaining category',
 'lookups and complete-class entries are necessary to preserve identifier meaning.',
 'Final paired spreads and A/A ranges show substantial wall-time variation. These',
 'samples do not establish a speedup; runtime variation occurs between identical',
 'executables and cannot demonstrate a generated-code regression or improvement.',
 '',
 '## Corrected-only capability scaling','',
 'The entry compiler rejects every contextual input, so it supplies no correct',
 'A performance oracle. Time is median [min,max] milliseconds; RSS is peak KiB.',
 '',
 '| Templates | Compile ms [spread] | RSS | Runtime ms [spread] | Text |',
 '|---:|---|---:|---|---:|']
for name in ['contextual600','contextual1200','contextual2400']:
 c=owner['summary'][name]['compile'];t=owner['summary'][name]['runtime']
 def duration(x):return f"{1000*x['median_s']:.2f} [{1000*x['range_s'][0]:.2f}, {1000*x['range_s'][1]:.2f}]"
 lines.append(f"| {owner['inputs'][name]['N']} | {duration(c)} | {c['peak_rss_kib']} | {duration(t)} | {owner['images'][name]['text_bytes']} |")
lines+=['','All 24 final compiler samples satisfy these existing telemetry counters:',
 '`tokens=45N+139`, `parsed_nodes=nodes=80N+220`, initializer bindings `2N`,',
 'template binding work `38N`, template bindings `8N`, zero specializations/body',
 'transitions, 73 LowIR instructions, 118 operands, four native functions and',
 '104 native instructions. Emitted code/text and images are identical at all N.',
 'This verifies source-proportional retention/binding without instantiating unused',
 'coroutines; it does not claim that actual coroutine runtime is implemented.',
 '',
 '## Acceptance and limits','',
 f"Compiler text is {compiler_sizes['entry']['text']:,} → {compiler_sizes['final']['text']:,} bytes (+{compiler_sizes['final']['text']-compiler_sizes['entry']['text']:,}, {(compiler_sizes['final']['text']/compiler_sizes['entry']['text']-1)*100:.3f}%).",
 'That growth implements required parsing, lookup and diagnostics. Program text',
 'does not grow. No new optional optimizer, retained optimization body, global',
 'cache or work/growth allowance is introduced. Existing 1,000,000-step/512-depth',
 'constant evaluation, 0x70000000 native frame/data, 4096 alignment and course',
 'timeouts remain. No self-hosting claim is made; PA34 retains that owner.',
 '',
 'Under spec §9, correctness costs and later-stage work do not create new exit',
 'gates. Historical blanket 15%/zero-growth targets remain self-selected',
 'diagnostics; no mandated limit, correctness rule or coverage is relaxed.',
 'The refinement and all preliminary evidence remain visible. The measurements',
 'support bounded stage-scoped semantic work, not an optimization-profit claim.','']
(root/'pa29/performance188.md').write_text('\n'.join(lines))
records=[root/p for p in ['pa29/plan.md','pa29/handoff188.md','pa29/performance188.md','student.tests/pa29/check188.py','student.tests/pa29/validate188.py','student.tests/pa29/analyze188.py','student.tests/pa29/performance188.py']]
save('manifest',dict(entry=entry,implementation=implementation,stage_base='2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543',last_reviewed='2df00585bd10d4e2e068934394dffc8adb0a47ed',observations=880,launchers=16,checks=dict(priorThroughTests='pass',fileAudit='pass',stageProgress='pass',stageTests='395/403; eight original failures remain'),files={str(p.relative_to(root)):sha(p) for p in [*sorted(out.glob('*.json')),*records] if p.name!='manifest.json'}))
print('880 observations, 24 final scaling checks, 61 controls, 1707 unchanged contract paths; original failures 9 -> 8')
