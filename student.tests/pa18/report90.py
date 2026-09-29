#!/usr/bin/env python3
"""Render the final PA18 performance record, preserving earlier observations."""
from pathlib import Path
import json, statistics

ROOT = Path(__file__).resolve().parents[2]
FILES = ('loop90-performance.json',)
records = [json.loads((ROOT/'student.tests/pa18'/p).read_text()) for p in FILES]
assert all(d.get('finished_utc') for d in records)
d = records[-1]

def count(data):
    groups = [data['startup']] + [w[k] for w in data['workloads'].values() for k in ('compiler', 'runtime') if k in w]
    return sum(len(g['warmups'])+len(g['observations']) for g in groups)

def pair(m, key, scale=1):
    return ' / '.join(f"{m[str(i)][key]*scale:.2f}" if str(i) in m else '—' for i in (0, 1))

def ratio(m):
    r = m.get('paired_b_over_a')
    return f'{statistics.median(r):.3f} ({min(r):.3f}–{max(r):.3f})' if r else 'final only'

def payload(w):
    sizes = {o['binary']: o.get('native', {}).get('payload_bytes') for o in w['outputs']}
    return ' / '.join(str(sizes[i]) if sizes.get(i) is not None else '—' for i in (0, 1))

text = f'''# PA18 audit 90 performance

Acceptance is **PA18/O0 LowIR**, spec §9. Frozen entry `{d['commits'][0][:8]}`
is compared with repair `{d['commits'][1][:8]}` over **{len(d['workloads'])} workloads**,
with **{count(d)} observations** in [the full record](../student.tests/pa18/loop90-performance.json).
[benchmark90.py](../student.tests/pa18/benchmark90.py) and
[report90.py](../student.tests/pa18/report90.py) reproduce this evidence.
All 54 audit-89 sources and 34 distinct audit-82 sources remain unchanged;
eleven nonfinal-pack workloads are added. The added inherited corpus directly
checks the named-result optimization, retained calls and signature/query costs.
Prior [performance 89](performance89.md), including its unsuccessful observations,
and inherited profitability evidence remain unchanged.

Binaries, source hashes, flags, CPU affinity and all observations are retained.
Each comparable workload has one warmup per binary, four A/A calibration samples,
and four wall-time ABBA blocks. Entry rejections receive six final-only samples.
Compilation and execution are measured separately with telemetry disabled;
preflights validate LowIR and check native results. No validation or build runs
concurrently with timing. Startup A/B: **{pair(d['startup'], 'median_wall_s', 1000)} ms**.
Startup-sized compiler rows are diagnostic; they do not establish a speedup.

The supplied `lowir2native-ref -O0` is used only by the explicit harness,
bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. Compiler size is `.text`.
The sectionless native ELF exposes **loadable payload including data**; this is
a disclosed code-size proxy, not an isolated native `.text` measurement.
Times are milliseconds; RSS is peak KiB; ranges show all paired block ratios.

| Workload | Compiler A / B ms | Paired B/A (range) | A / B KiB | A / B native payload bytes |
|---|---:|---:|---:|---:|
'''
for name, work in d['workloads'].items():
    m = work['compiler']
    text += f"| {name} | {pair(m, 'median_wall_s', 1000)} | {ratio(m)} | {pair(m, 'peak_rss_kib')} | {payload(work)} |\n"
a, b = [r['text_bytes'] for r in d['binaries']]
text += f'\nCompiler `.text`: **{a:,} → {b:,} bytes** ({b-a:+,}, {100*(b-a)/a:+.3f}%).\n\n'
text += '| Workload | Runtime A / B ms | Paired B/A (range) | A / B native payload bytes |\n|---|---:|---:|---:|\n'
for name, work in d['workloads'].items():
    if 'runtime' in work:
        text += f"| {name} | {pair(work['runtime'], 'median_wall_s', 1000)} | {ratio(work['runtime'])} | {payload(work)} |\n"

assert all(w['comparison'] == 'exact' for w in d['workloads'].values() if len(w['outputs']) == 2)
text += '\nA/A calibration is retained per workload in the full record. Representative compiler noise:\n\n'
text += '| Workload | A/A max/min | Paired B/A (range) |\n|---|---:|---:|\n'
for name in ('ordering-2400','constant-reference-600','cast-reference-2400','retained-result-600','audit-conversion-lookup-2400','wide-array-600'):
    m = d['workloads'][name]['compiler']
    lo, hi = m['aa_range_s']
    text += f"| {name} | {hi/lo:.3f} | {ratio(m)} |\n"

repeats = sorted((ROOT/'student.tests/pa18').glob('loop90-repeat-*.json'))
text += '\nTargeted repeats retain the same binaries, flags and source hashes; every observation is kept.\n\n'
text += '| Repeat record / workload | A / B ms | Paired B/A (range) | A/A max/min |\n|---|---:|---:|---:|\n'
repeat_count = 0
for path in repeats:
    r = json.loads(path.read_text())
    assert r.get('finished_utc')
    assert [b['sha256'] for b in r['binaries']] == [b['sha256'] for b in d['binaries']]
    repeat_count += count(r)
    for name, w in r['workloads'].items():
        assert w['source_sha256'] == d['workloads'][name]['source_sha256'] and w['comparison'] == 'exact'
        m = w['compiler']; lo, hi = m['aa_range_s']
        text += f"| [{path.stem}](../student.tests/pa18/{path.name}) / {name} | {pair(m, 'median_wall_s', 1000)} | {ratio(m)} | {hi/lo:.3f} |\n"
initial = json.loads((ROOT/'student.tests/pa18/loop90-performance-initial.json').read_text())
text += f'\nThe full run retains {count(d)} observations, the repeats {repeat_count}, and the interrupted initial record {count(initial)} completed observations.\n'

text += '''
## Findings and budgets

All 89 comparable inputs retain exact LowIR and executable bytes wherever a
native entry exists. Thus the earlier loops, calls, memory, floating point,
array, lifetime and named-result optimization workloads retain their code.
The one-element default runtime control already worked at entry; the other
ten newly supported inputs have final-only costs; comparing their time with
an entry rejection would not be a meaningful speedup. The new runtime loop
uses a volatile bound and checks its sum. Runtime observations on identical
executables are noise diagnostics, not claims that deduction speeds up code.

| Family | Size | Candidates | Expansion lanes | Body transitions | Instructions |
|---|---:|---:|---:|---:|---:|
'''
for family in ('nonfinal', 'nonfinal-target', 'nonfinal-list'):
    for n in (150, 600, 2400):
        output = next(o for o in d['workloads'][family+'-'+str(n)]['outputs'] if o['binary'] == 1)
        t, ir = output['telemetry']
        text += f"| {family} | {n} | {t['semantic_candidate_work']} | {t['semantic_pack_expansion_lanes']} | {t['template_body_transitions']} | {ir['instructions']} |\n"
text += '''
The initial run on `34b49cab` was intentionally interrupted after the related
nonfinal-template-list defect was confirmed. Its completed observation groups
remain in [the initial record](../student.tests/pa18/loop90-performance-initial.json);
the unfinished group is not used for a claim. Final acceptance uses the complete
run on `fe4a11f0`, with the broader corrected corpus.

The apparent compiler slowdowns do not persist consistently. Constant-reference,
retained-result, conversion-lookup and wide-array repeats have paired medians
0.996, 0.984, 1.001 and 0.974. The cast-reference repeat remains noisy
(0.768–1.249); its further same-binary confirmation has A/A max/min 1.010 and
paired ratios 0.971–0.996. The primary cast run also shows the same A binary
moving from roughly 260 ms during calibration to 346–673 ms in ABBA blocks.
Prefix-nested-150 repeats at 1.014 (0.919–1.028), with A/A max/min 1.043.
These observations support no precise compiler speedup or consistent regression;
all initial/repeat samples remain visible. Source review finds no added global
work or optional transform: the required parameter/default work stays local and
proportional. The implementation growth is 7,296 compiler text bytes (0.366%);
comparable generated programs do not grow. Stage-scoped acceptance is satisfied.

Candidate-local parameter views are allocated only for nonfinal packs; work is
linear in source parameters plus explicitly supplied lanes. A completed function
owns one default-position slice, and defaults retain their existing lazy fact
states. There is no new optional optimization, retry, syntax replay or output
growth allowance. Existing O0 named-result summaries inspect at most eight
wrappers, empty-helper omission checks one action head and adds no code,
constant evaluation retains its million-step/512-depth limits, and required
array expansion retains its bounded policy. Original profitability measurements
remain in performance 82/86; this run verifies their emitted code is preserved.

PA18 has no mandated numeric compiler latency/RSS ceiling. Historical +15%,
+16 MiB and 5.5× diagnostic targets remain preserved measurements, not exit gates.
Necessary costs of newly accepted semantics are disclosed, with proportional
work and conservative rejection of failed deductions. Native selection,
allocation, optimization/debug, hosted varargs and self-hosting are later-stage
ownership, not additional PA18 exit gates.
'''
(ROOT/'pa18/performance90.md').write_text(text)
print('Rendered', count(d), 'observations.')
