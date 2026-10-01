#!/usr/bin/env python3
"""Publish reproducible performance tables and assert all-sample demand scaling."""
import hashlib,json,os,pathlib,statistics,subprocess
root=pathlib.Path(__file__).resolve().parents[2]
evidence=root/'student.tests/pa29/evidence176'
art=pathlib.Path(os.environ['RALPH_ARTIFACT_DIR'])/'implementation176'
c=json.loads((evidence/'common-performance.json').read_text());a=json.loads((evidence/'declaration-performance.json').read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
checks=[]
for row in a['runs']:
 if row['mode']!='compile':continue
 values={k:v for obj in row['counters'] for k,v in obj.items()};n=row['n']
 expected=dict(inline_variable_initializers=n,inline_variable_hits=n,parsed_nodes=28*n+339,nodes=103*n+339,instructions=23*n+56,text_bytes=136*n+337)
 for k,v in expected.items():assert values[k]==v,(n,k,values[k],v)
 checks.append(dict(n=n,sample=row['sample'],counters=expected))
assert len(checks)==24
(evidence/'scaling.json').write_text(json.dumps(dict(all_samples_pass=True,formulas=dict(inline_variable_initializers='N',inline_variable_hits='N',parsed_nodes='28N+339',nodes='103N+339',instructions='23N+56',native_text_bytes='136N+337',executable_text_bytes='136N+577'),samples=checks),indent=2)+'\n')
manifest=dict(entry_commit='2b7a513297b2e9719fcd5686d3c1c97ee7565b13',code_commit='1b19e9ac',artifacts=str(art),binaries={},scripts={})
for label,file in [('entry',art/'entry'),('final',art/'final')]:manifest['binaries'][label]=dict(path=str(file),bytes=file.stat().st_size,sha256=sha(file))
assert manifest['binaries']['final']['sha256']==sha(root/'dev/cppgm++')
for file in ['test176.py','inspect176.py','validate176.py','performance176.py','report176.py']:
 p=root/'student.tests/pa29'/file;manifest['scripts'][str(p.relative_to(root))]=sha(p)
p=root/'student.tests/pa27/performance147_common.py';manifest['scripts'][str(p.relative_to(root))]=sha(p)
manifest['controls']=json.loads((evidence/'controls.json').read_text())['compiler_sha256']
manifest['inspection']=json.loads((evidence/'inspection.json').read_text())['compiler_sha256']
assert manifest['controls']==manifest['inspection']==manifest['binaries']['final']['sha256']
manifest['preliminary_validation']=str(art/'preliminary/validation/validation.json')
(evidence/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
lines=['# Performance176 — PA29 / O0','',
'This change supplies required declaration, demand and lifetime semantics. No optional',
'optimization pass or speedup is claimed. The entry compiler cannot produce a correct',
'executable for the affected workload; those rejected compilations are retained.',
'Spec §9 therefore permits equivalent comparisons on common correct workloads and',
'scaling/correct-output evidence for the new capability.','',
'## Frozen protocol','',
'The [manifest](../student.tests/pa29/evidence176/manifest.json) freezes entry/final',
'binaries, hashes and scripts; final implementation is `1b19e9ac`.',
'Both compiler and executable timing use wall time separately from host linking.',
'The [common raw evidence](../student.tests/pa29/evidence176/common-performance.json)',
'contains AAAA noise calibration and six ABBA blocks per workload/mode (224 samples).',
'Flags are `-O0 -c --stats`; the inherited inputs contain 2,400 demanded templates',
'plus checked loops/calls/memory, floating point, exceptions or unused functions.',
'All samples, outliers, RSS, phase counters, input/image hashes and linker version',
'are preserved. No affinity or hardware-counter dependency was introduced.',
'Builds and correctness suites finished before measurement; external scheduling',
'and concurrent documentation activity were not controlled.','',
'| Workload | Compile A/B median s | Compile paired B/A [range] | Compiler peak RSS A/B KiB | Runtime A/B median s | Runtime paired B/A [range] | Executable text A/B bytes |',
'|---|---:|---:|---:|---:|---:|---:|']
for name,s in c['summary'].items():
 q=s['compile'];r=s['runtime'];im=c['images'][name]
 lines.append('| %s | %.4f/%.4f | %.4f [%.4f–%.4f] | %d/%d | %.4f/%.4f | %.4f [%.4f–%.4f] | %d/%d |'%(name,q['A']['median_s'],q['B']['median_s'],q['paired_ratio_median'],*q['paired_ratio_range'],q['A']['peak_rss_kib'],q['B']['peak_rss_kib'],r['A']['median_s'],r['B']['median_s'],r['paired_ratio_median'],*r['paired_ratio_range'],im['A']['executable_text_bytes'],im['B']['executable_text_bytes']))
 assert im['A']['executable_text_bytes']==im['B']['executable_text_bytes']
lines+=['','| Workload | Compile A/A range s | Runtime A/A range s |','|---|---:|---:|']
for name,s in c['summary'].items():lines.append('| %s | %.4f–%.4f | %.4f–%.4f |'%(name,*s['compile']['AA_range_s'],*s['runtime']['AA_range_s']))
lines+=['','Executable text is unchanged on every common workload. Paired compiler medians',
'range from 0.9948 to 1.0154 and runtime medians from 0.9903 to 1.0117; their spreads',
'and calibration do not establish a repeatable regression or speedup. Peak compiler',
'RSS grows at most 1.64% here. These are observations, not a new percentage gate.','',
'## Affected semantics and scaling','',
'[All affected observations](../student.tests/pa29/evidence176/declaration-performance.json)',
'contain eight compiler and eight runtime samples at 600/1200/2400 specializations',
'(48 samples). Each class has an excluded member body and an excluded inline static',
'object initialized by a non-constexpr call. An extern-template class declaration',
'coexists with the required local body/data demand. Runtime checks initialization',
'counts and computes 24 million member calls from a `strtol` seed and varying loop',
'inputs. Every run checks its printed checksum against independent Python arithmetic.',
'Flags are `-std=c++11 -O0 -c --stats`, and the runtime seed is `17`.','',
'| Specializations | Compile median [range] s | Compiler peak RSS KiB | Runtime median [range] s | Runtime peak RSS KiB | Executable text bytes |',
'|---|---:|---:|---:|---:|---:|']
for n,s in a['summary'].items():
 q=s['compile'];r=s['runtime'];lines.append('| %s | %.4f [%.4f–%.4f] | %d | %.4f [%.4f–%.4f] | %d | %d |'%(n,q['median_s'],*q['range_s'],q['peak_rss_kib'],r['median_s'],*r['range_s'],r['peak_rss_kib'],a['inputs'][n]['text_bytes']))
lines+=['', 'Launcher median is **%.5f s**, range **%.5f–%.5f s**. The shortest compiler'% (statistics.median(a['launcher_s']),min(a['launcher_s']),max(a['launcher_s'])),
'and runtime medians exceed 20× that median. At the same total call count the',
'2,400-function program runs materially slower than the 1,200-function program.',
'This is disclosed as an O0 generated-code limitation; no cache/profiler attribution',
'or runtime improvement is claimed. Entry-to-final affected ratios are invalid',
'because entry rejects these required semantics.','',
'[All-sample counter checks](../student.tests/pa29/evidence176/scaling.json) prove',
'**N initializer computations**, **N completed-fact hits**, **28N+339 parsed nodes**,',
'**103N+339 occurrence nodes**, **23N+56 instructions**, and **136N+337 native text',
'bytes** in all 24 compiler samples. Executable text is **136N+577 bytes**.',
'Work and storage follow declarations, demanded initializers and actual emitted',
'operations. No unused member initializer is computed merely to complete a class.',
'Controls also verify that constant-only queries emit no inline member objects and',
'`--stats` does not change emitted object bytes.','',
'## Acceptance and budgets','',
'Optional optimization work and growth budgets remain **zero**. Required dynamic',
'objects add one eight-byte initialization guard; required temporary destructor',
'registration adds its own eight-byte guard. These identities and emitted helpers',
'are linear in demanded objects/lifetime actions, with no optional whole-program',
'pass or fixed-point search. The new facts have one owner and one terminal result.',
'Necessary semantic costs are distinguished from later optimizer/allocation and',
'heavy hosted-runtime work. No avoidable regression was established on equivalent',
'correct inputs. The stage requires these semantics regardless of speedup.','',
'The inherited unsupported blanket **15% latency/RSS** and **zero-growth** targets',
'remain diagnostic, following [performance175](performance175.md) and spec §9.',
'No measurements or mandatory limits were removed: generated elements remain',
'**1,048,576**, constexpr steps **1,000,000**, call depth **512**, packed source-site',
'capacity **2^31−1**, native frame/data capacity **0x70000000**, alignment **4096**,',
'and all course timeouts. PA30–34 retain heavier hosted-runtime, optimization and',
'self-hosting work. Correctness and coverage remain mandatory.','',
'Reproduce with the manifest’s frozen binaries:','', '```sh',
'python3 student.tests/pa27/performance147_common.py OUT_COMMON ENTRY FINAL',
'python3 student.tests/pa29/performance176.py OUT_AFFECTED ENTRY FINAL',
'python3 student.tests/pa29/test176.py OUT_CONTROLS FINAL',
'python3 student.tests/pa29/inspect176.py OUT_INSPECTION',
'python3 student.tests/pa29/validate176.py OUT_GATES','```','']
(root/'pa29/performance176.md').write_text('\n'.join(lines))
print('scaling, manifest and report published')
