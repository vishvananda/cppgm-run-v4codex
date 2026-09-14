#!/usr/bin/env python3
"""Render every final paired campaign without dropping calibration or outliers."""
from pathlib import Path
import json, statistics
ROOT = Path(__file__).resolve().parent

def render():
    main = json.loads((ROOT/'source-obligations-performance.json').read_text())
    repeat = json.loads((ROOT/'source-obligations-noise.json').read_text())
    text = '''# Final source-obligation performance review

The frozen comparison is destination checkpoint A against final source-obligation
B. Both produce correct equivalent results on every timed input. Invalid and
newly accepted controls are correctness evidence, not timing baselines. The main
campaign has 47 compiler and 14 executable workloads; the repeat has 15 compiler
and four executable workloads: **1,120 observations, 19,726 cumulative**. All 37
parent compiler workloads remain. Raw data: [main](source-obligations-performance.json),
[repeat](source-obligations-noise.json), [layout](source-obligations-layout.json).

Builds use g++ C++11, -Wall -O3 and TEST_RUNNER_ENABLE; the tested compiler and
supplied backend use O0. Binaries, flags, inputs, outputs, backend and harnesses
are hash-frozen. Each campaign checks outputs before timing, warms A/B once,
records four A/A observations and two ABBA blocks on CPU 0. Agent builds, tests,
verification and compression did not overlap timing. Every sample is retained.
The main ran 2026-09-13 23:43:51–23:48:45 UTC; the repeat 23:48:57–23:51:27.

A SHA-256: `cd924522f01a357ba41c67ff7f349324be297e55b98f82dac12e86220cc3044c`.
B SHA-256: `f971551f64fd4cffe0f4effb5f5391ed818845c05a56ef26cb106e26c0baf115`.

## Acceptance and interpretation

The affected fixed-operator shape K=128/M=8 improves compiler latency in all
four blocks: main −5.18%/−5.00%, repeat −5.09%/−4.88%, against A/A ranges of
0.87% and 1.07%. Candidate visits fall 2,184→152 and conversion work 7,458→473.
This supports a workload-specific compiler benefit from source selection reuse.
It does not establish native runtime improvement or a general compiler speedup.
The N=64,000 and Q=4,000 operator shapes have mixed blocks across campaigns.

The new default shape retains M source recipes and KM concrete uses. The
operator shape retains 2M operator/cast recipes and 2KM uses, plus M initializer
recipes and KM uses. N unrelated declarations and Q repeated calls do not
multiply source work. Default candidate visits increase by one per shape for
the newly required declaration-property selection. The initial implementation
missed the declarator key and recorded zero default reuse; it was corrected
before any timing. At K=128, the source-default and source-operator shapes keep
the same entities and scopes as A; the prior KM destination saving remains.

Compiler .text grows 1,371,014→1,424,966 bytes (+53,952, 3.94%). MemberFacts grows
120→124 bytes for declaration properties and Analyzer 6,552→6,760 for typed
indexes/context/counters. The other 16 measured main records are unchanged;
ConversionObject/Conversion/ValueInitialization/UserConversion stay 52/28/8/80.
These are semantic ownership costs, with no optional transform or growth pass.

Peak RSS is not uniformly lower. Demand rises 346,224→346,910 KiB in the main
and 346,100→346,838 in the repeat. It now retains 128 required condition recipes
and 128 extra conversion records while conversion work falls by 872. Member
property record growth also applies to demanded declarations. The exact RSS
attribution is not isolated. The declaration workload's median RSS reverses
from −3,646 to +4,000 KiB across campaigns with almost unchanged semantic
counts (one extra fact, two queries, six lookup visits). This is not evidence
of a stable memory saving. Operator K=128 RSS also reverses: −126 then +326 KiB.
All spreads below remain part of acceptance; no measured increase is hidden.

Large compiler wall outliers remain: source-default Q=4,000 +75.55% in one main
block, source-operator Q=4,000 +13.46%, return K=1 −15.59%, and body −7.86%.
Their repeats, where present, do not reproduce these magnitudes. Declaration
repeat blocks are +1.72%/+3.69% with 9.83% A/A spread. The timing cause is not
isolated. Compiling the tiny runtime sources takes about 5–7 ms and is startup
sensitive; those rows support no fine latency claim.

All 14 native text payload sizes are unchanged; 12 complete executable images
are identical. Destructor-runtime and source-default-runtime have different
declaration/emission order, equal course-canonical LowIR and equal text size.
Their checked outcomes, calls and cleanup remain intact. No native speedup is
claimed. Source-operator runtime has +11.42%/−6.60% main and +9.10%/−0.48%
repeat blocks despite identical executable bytes; the main A/A spread is 13.48%.
Source-default runtime has +0.57%/−0.99%, then −1.97%/−6.52%, with 20.91% and
3.19% calibration spreads. These do not prove a repeatable generated-code
regression or improvement. All native outliers and RSS remain below and in JSON.

PA14/O0 acceptance requires correct bounded semantic work and removal of
avoidable costs. Fixed selections are reused; concrete preparation follows
required uses and retains effects, access/deletion, object identity and cleanup.
Queries/default properties do not demand bodies or create runtime objects.
Complete typed keys and terminal success/failure states avoid global invalidation.
No optional executable optimization is added, so there is no profitability
policy or speculative code expansion to justify. Equivalent typed IR retains
its provenance and ABI; missing facts fail at the owning boundary. Native
selection, register allocation, encoding, optimized levels and self-hosting
remain later-stage work. There is no mandated PA14 numerical latency/RSS/text
cap. Historical self-selected targets remain diagnostics; none overrides these
stage-scoped rules or weakens a language, coverage, comparison or work bound.

## All observations summarized

Medians and ranges below use the four observations per binary within the two
ABBA blocks, excluding warmups and calibration. Each percentage is a block's
mean B/A wall change. A/A shows all four calibration observations as min–max
milliseconds and range/min percent. RSS is each process's peak, in KiB. The
linked JSON also retains warmups, CPU time and context switches. Calibration
measures observed variation; it is not a bound on later wall stalls.

'''
    for label, data in [('Main', main), ('Repeat', repeat)]:
        for phase in ['compiler', 'runtime']:
            text += f'### {label} {phase}\n\n'
            text += '| Workload | A / B median ms [range] | Paired change % | A/A ms (spread %) | A / B peak RSS KiB [range] |\n|---|---|---|---|---|\n'
            for name, row in data['workloads'].items():
                if phase not in row:
                    continue
                c = row[phase]
                obs = c['observations'][4:]
                def values(field, binary):
                    return [r[field] for r in obs if r['binary'] == binary]
                def describe(xs, scale, digits):
                    return f'{statistics.median(xs)*scale:.{digits}f} [{min(xs)*scale:.{digits}f}–{max(xs)*scale:.{digits}f}]'
                walls = ' / '.join(describe(values('wall_s', b), 1000, 2) for b in [0, 1])
                rss = ' / '.join(describe(values('rss_kib', b), 1, 0) for b in [0, 1])
                pairs = ' / '.join(f'{(p-1)*100:+.2f}' for p in c['paired_b_over_a'])
                lo, hi = c['aa_range_s']
                text += f'| {name} | {walls} | {pairs} | {lo*1000:.2f}–{hi*1000:.2f} ({(hi/lo-1)*100:.2f}) | {rss} |\n'
            text += '\n'
    text += '### Native text and output identity\n\nThe supplied sectionless backend metric is payload after ELF entry, as in the\nshared harness; compiler text uses the .text section. Loops, calls, memory,\nfloating point, virtual dispatch and lifecycle workloads check live results.\nThe new default runtime checks four million calls and checksum 18,000,000;\nthe new operator runtime checks sixteen million effects and checksum 40,000,000.\n\n| Workload | A → B text bytes | Complete image equal |\n|---|---:|---|\n'
    for name, row in main['workloads'].items():
        if 'runtime' in row:
            a, b = [o['native'] for o in row['outputs']]
            text += f"| {name} | {a['text_bytes']} → {b['text_bytes']} | {'yes' if a['sha256']==b['sha256'] else 'no; equivalent canonical LowIR'} |\n"
    text += '\nThe [validation](source-obligations-validation.json) contains 79 passing groups,\n1935/1935 course tests, both builds’ 242 source controls and four branch programs,\n349 entry/current and 349 release/sanitizer comparisons, all 35 existing PA14\nnative programs and inherited demand/lifetime/ABI probes. Repeated property\nqueries perform no body/action demand and concrete finish queries remain stable.\nAll 1,266 fixture/reference hashes are unchanged. The [proofs](source-obligations-proofs.json)\nrecord 171 incorrect entry outcomes; the [handoff](source-obligations-handoff.json)\ncloses all five previously open cases under both builds. The [journal](source-obligations-journal.json)\nretains preliminary failures, corrected evidence collisions and lossless probe\narchives. No preliminary source-obligation timing was used or discarded.\n'
    return text

if __name__ == '__main__':
    (ROOT/'source-obligations-performance.md').write_text(render())
