#!/usr/bin/env python3
"""Publish all-sample scaling, a frozen manifest, and performance tables."""
import hashlib,json,os,pathlib,statistics,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
evidence=root/'student.tests/pa29/evidence177'
art=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'implementation177'
c=json.loads((evidence/'common-performance.json').read_text());a=json.loads((evidence/'selection-performance.json').read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
keys=['parsed_nodes','nodes','instructions','text_bytes']
series={}
for n in [600,1200,2400]:
 rows=[{k:v for obj in row['counters'] for k,v in obj.items()} for row in a['runs'] if row['n']==n and row['mode']=='compile']
 assert len(rows)==8
 series[n]={k:rows[0][k] for k in keys}
 assert all(all(row[k]==series[n][k] for k in keys) for row in rows)
formulas={}
for key in keys:
 slope=(series[1200][key]-series[600][key])/600
 intercept=series[600][key]-slope*600
 assert series[2400][key]==slope*2400+intercept,(key,series)
 formulas[key]=dict(slope=slope,intercept=intercept)
(evidence/'scaling.json').write_text(json.dumps(dict(all_samples_pass=True,formulas=formulas,observations=series),indent=2)+'\n')
manifest=dict(entry_commit='d69d57fc28bfc308f8223e3090fb9e9ab37c4823',code_commit='8288199b',artifacts=str(art),binaries={},scripts={})
for label in ['entry','final']:
 p=art/label;manifest['binaries'][label]=dict(path=str(p),bytes=p.stat().st_size,sha256=sha(p))
assert manifest['binaries']['final']['sha256']==sha(root/'dev/cppgm++')
for file in ['test177.py','inspect177.py','validate177.py','performance177.py','report177.py']:
 p=root/'student.tests/pa29'/file;manifest['scripts'][str(p.relative_to(root))]=sha(p)
p=root/'student.tests/pa27/performance147_common.py';manifest['scripts'][str(p.relative_to(root))]=sha(p)
for file in ['controls.json','inspection.json','validation.json']:
 d=json.loads((evidence/file).read_text());assert d['compiler_sha256']==manifest['binaries']['final']['sha256']
(evidence/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
lines=['# Performance177 — PA29 / O0','',
'This group implements required scope, branch-demand and lifetime semantics.',
'Optional optimization work and code-growth budgets are **zero**. No speedup is',
'claimed. Entry rejects the affected selection syntax, so comparisons use common',
'correct inputs; affected inputs supply scaling and checked-output evidence.','',
'## Frozen protocol','',
'The [manifest](../student.tests/pa29/evidence177/manifest.json) freezes entry/final',
'binaries, scripts, hashes and flags. [Common raw evidence](../student.tests/pa29/evidence177/common-performance.json)',
'retains AAAA calibration plus six ABBA blocks for compiler latency/RSS and',
'separately measured executable runtime (224 samples). Host linking is outside',
'both timers. Workloads contain 2,400 demanded templates plus checked loops,',
'calls, memory, floating point, exceptions or unused functions. Runtime inputs',
'and checks prevent timing a folded/dead workload. Flags are `-O0 -c --stats`.',
'All observations, including outliers and phase/work counters, are retained.',
'No affinity or hardware-counter dependency was added. Builds and correctness',
'suites completed before timing; external system activity is uncontrolled.','',
'| Workload | Compile A/B median s | Compile paired B/A [range] | Compiler peak RSS A/B KiB | Runtime A/B median s | Runtime paired B/A [range] | Text A/B bytes |',
'|---|---:|---:|---:|---:|---:|---:|']
for name,s in c['summary'].items():
 q=s['compile'];r=s['runtime'];im=c['images'][name]
 lines.append('| %s | %.4f/%.4f | %.4f [%.4f–%.4f] | %d/%d | %.4f/%.4f | %.4f [%.4f–%.4f] | %d/%d |'%(name,q['A']['median_s'],q['B']['median_s'],q['paired_ratio_median'],*q['paired_ratio_range'],q['A']['peak_rss_kib'],q['B']['peak_rss_kib'],r['A']['median_s'],r['B']['median_s'],r['paired_ratio_median'],*r['paired_ratio_range'],im['A']['executable_text_bytes'],im['B']['executable_text_bytes']))
 assert im['A']['executable_text_bytes']==im['B']['executable_text_bytes']
 assert im['A']['object_sha256']==im['B']['object_sha256']
 assert im['A']['executable_sha256']==im['B']['executable_sha256']
lines+=['','| Workload | Compile A/A range s | Runtime A/A range s |','|---|---:|---:|']
for name,s in c['summary'].items():lines.append('| %s | %.4f–%.4f | %.4f–%.4f |'%(name,*s['compile']['AA_range_s'],*s['runtime']['AA_range_s']))
lines+=['',
'All four A/B objects and executables are byte-identical, as verified by hashes.',
'Compiler paired medians range from 0.9325 to 1.0360; peak compiler RSS grows at',
'most 0.22%. The wide calibration/block spreads do not establish a repeatable',
'compiler regression or speedup. Runtime medians include a 5.18% exception and',
'2.57% pruning increase; identical images and wide A/A spreads rule out changed',
'generated instructions as their cause. All slower samples remain in the record.']
lines+=['','## Affected semantics','',
'[All affected observations](../student.tests/pa29/evidence177/selection-performance.json)',
'contain eight compiler and eight runtime samples at 600/1200/2400 specializations',
'(48 samples). Each specialization has a constexpr alias initializer selecting',
'one of an ordinary if initializer or switch initializer. Runtime executes',
'24 million calls with a `strtol` seed of 17 and varying loop inputs. Python',
'independently computes the printed checksum and every run checks it.',
'Flags are `-std=c++11 -O0 -c --stats`. Entry rejection diagnostics are retained.','',
'| Specializations | Compile median [range] s | Compiler peak RSS KiB | Runtime median [range] s | Runtime peak RSS KiB | Executable text bytes |',
'|---|---:|---:|---:|---:|---:|']
for n,s in a['summary'].items():
 q=s['compile'];r=s['runtime'];lines.append('| %s | %.4f [%.4f–%.4f] | %d | %.4f [%.4f–%.4f] | %d | %d |'%(n,q['median_s'],*q['range_s'],q['peak_rss_kib'],r['median_s'],*r['range_s'],r['peak_rss_kib'],a['inputs'][n]['text_bytes']))
lines+=['','Launcher median **%.5f s**, range **%.5f–%.5f s**.'%(statistics.median(a['launcher_s']),min(a['launcher_s']),max(a['launcher_s'])),
'[All-sample scaling checks](../student.tests/pa29/evidence177/scaling.json) retain exact linear relationships:','']
for key,f in formulas.items():lines.append('- `%s = %gN %s %g`.'%(key,f['slope'],'+' if f['intercept']>=0 else '-',abs(f['intercept'])))
lines+=['',
'Executable text is `141N + 160` bytes. The half-instruction slope averages the',
'two equally represented constexpr arms. Compiler wall time is non-monotonic',
'across sizes, so it is not used to prove scaling; the all-sample counters supply',
'that evidence. Runtime rises at 2,400 functions despite a fixed total call count.',
'This is an O0 generated-program limitation, without a profiler-based attribution',
'or an affected speedup claim. The shortest medians exceed 80× median launcher',
'cost. These results preserve the semantic requirement and expose later optimizer',
'work rather than imposing an unsupported new percentage gate.']
lines+=['','## Acceptance and boundaries','',
'No optional optimizer work/growth was introduced. Semantic work follows actual',
'statements, canonical declarations and demanded specialization facts; lowering',
'follows selected statements and lifetime actions. Common executable text is',
'unchanged. Necessary semantic costs and later optimization work are separate',
'from the current PA29/O0 acceptance. Timings do not certify whole-stage design.','',
'The inherited unsupported blanket 15% latency/RSS and zero-growth gates remain',
'diagnostic under spec §9, as documented in [performance176](performance176.md).',
'No historical measurements were removed. Mandatory capacities remain generated',
'elements 1,048,576, constexpr steps 1,000,000, call depth 512, packed source-site',
'capacity 2^31−1, native frame/data capacity 0x70000000 and alignment 4096.',
'Course timeouts, correctness and coverage are unchanged. Heavy hosted runtime,',
'optimization and self-hosting remain later-stage work.','',
'Reproduction uses the manifest binaries:','', '```sh',
'python3 student.tests/pa27/performance147_common.py OUT_COMMON ENTRY FINAL',
'python3 student.tests/pa29/performance177.py OUT_AFFECTED ENTRY FINAL',
'python3 student.tests/pa29/test177.py OUT_CONTROLS FINAL',
'python3 student.tests/pa29/inspect177.py OUT_INSPECTION',
'python3 student.tests/pa29/validate177.py OUT_GATES','```','']
(root/'pa29/performance177.md').write_text('\n'.join(lines));print('manifest, scaling, tables published')
