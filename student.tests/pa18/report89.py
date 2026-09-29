#!/usr/bin/env python3
"""Render the final PA18 performance record, preserving earlier observations."""
from pathlib import Path
import json, statistics

ROOT = Path(__file__).resolve().parents[2]
FILES = ('loop89-performance.json', 'loop89-performance-ordering-repeat.json', 'loop89-performance-final.json', 'loop89-performance-complete.json')
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

text = f'''# PA18 final audit performance

Acceptance: **PA18/O0 LowIR**, spec §9. Frozen handoff `7000a4de` is compared
with reviewed code `{d['commits'][1][:8]}`. The [final record](../student.tests/pa18/{FILES[-1]})
contains **{count(d)} observations across {len(d['workloads'])} workloads**. The
[initial record](../student.tests/pa18/{FILES[0]}),
[ordering repeat](../student.tests/pa18/{FILES[1]}) and
[scalar-frame repair run](../student.tests/pa18/{FILES[2]}) remain: **{sum(map(count, records))} observations**
in this audit. Earlier [performance 86](performance86.md) and
[performance 87](performance87.md), including their unsuccessful observations,
remain unchanged. [benchmark89.py](../student.tests/pa18/benchmark89.py) and
[report89.py](../student.tests/pa18/report89.py) reproduce the measurements and tables.

The corpus retains every audit-86 and boundary-87 source, plus six pack-prefix
scaling inputs and a checked runtime loop. Each comparable workload has a
warmup per binary, four A/A observations and four ABBA blocks. Invalid entry
implementations retain their failures and get six final-only observations.
Compiler and executable timings are separate wall times with peak RSS from
`/usr/bin/time`; all native results are checked. Flags, sources, hashes, CPU
affinity, observations, spread and preflight telemetry are retained. No test
suite ran during timing; telemetry is disabled in timed compiler invocations.

The native execution boundary is the explicitly invoked supplied
`lowir2native-ref -O0`, bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Compiler size is actual `.text`; the sectionless native ELF metric is its
**loadable payload**, including data, not isolated native `.text`.
Times are median milliseconds; RSS is peak KiB. Startup-sized rows are
diagnostics. Startup A/B: **{pair(d['startup'], 'median_wall_s', 1000)} ms**.

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
text += '''
## Findings, work bounds and disposition

All 47 comparable workloads have byte-identical LowIR. Their native outputs
are also identical wherever the source provides an executable entry point;
the common-loop-float-1500 input measures compilation only.
Seven prefix workloads were rejected by the entry compiler, so their final
costs are reported without a speedup claim. The executable loop uses a volatile
bound and checks a nonconstant sum through the deduced function pointer.
Existing calls, memory, floating-point, array, lifetime and class-result runtime
workloads retain the same checked code and sizes.

The first measurements exposed one avoidable cost: forming an explicit-prefix
frame for scalar explicit arguments even when no pack prefix existed. The final
repair creates a frame only when an explicitly supplied pack needs it, in both
ordinary and target deduction; calls without arguments need no deduction frame.
The final pack-bound, audit-effects and
ellipsis-query counters remove the extra frames and argument tuples recorded
by the initial run. This is a demand/ownership repair, not an optional optimizer.

The initial ordering-600 compiler ratio was 1.135 (0.989–1.176); its 2400 row
was 1.003. The retained targeted repeat on the same initial binaries gives
0.997 (0.986–1.005) and 1.005 (0.984–1.006), respectively. This resolves the
smaller-row timing concern without deleting any observations. The final tables
retain all spread and disclose the final costs; no precise speedup is claimed.

| Family | Size | Candidates | Expansion lanes | Body transitions | Instructions |
|---|---:|---:|---:|---:|---:|
'''
for family in ('prefix-address', 'prefix-nested'):
    for n in (150, 600, 2400):
        output = next(o for o in d['workloads'][family+'-'+str(n)]['outputs'] if o['binary'] == 1)
        t, ir = output['telemetry']
        text += f"| {family} | {n} | {t['semantic_candidate_work']} | {t['semantic_pack_expansion_lanes']} | {t['template_body_transitions']} | {ir['instructions']} |\n"
text += '''
Work tracks actual specializations, lanes and emitted instructions. Expansion
parameter discovery is cached once per pattern; each demanded function body
transitions once. Frames and canonical arguments are TU-owned, with no syntax
replay, whole-program retry or new growth allowance. Existing O0 named-result
summaries retain their eight-wrapper bound, empty-helper omission its one-head
check and zero-growth policy, and constant evaluation its work/depth budgets.
Their earlier profitability evidence and required array-copy costs remain in
performance 82 and 86; this audit preserves their output exactly.

PA18 mandates no numerical compiler latency/RSS ceiling. Historical +15%,
+16 MiB and 5.5× targets remain diagnostics, not exit gates. Required prefix
deduction costs have bounded work; the identified unnecessary frame work is
removed. No unprofitable optional transform, runtime regression hidden by IR
size, or relaxed correctness/comparison requirement is accepted. Native
optimization/debug, hosted aggregate-varargs retrieval and self-hosting remain
later-stage ownership. Stage-scoped performance acceptance is satisfied.
'''
(ROOT/'pa18/performance89.md').write_text(text)
print('Rendered', count(d), 'final observations;', sum(map(count, records)), 'retained observations.')
