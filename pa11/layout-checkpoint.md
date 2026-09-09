# PA11 layout and initializer checkpoint

Correctness: **273/302**, 39 baseline failures fixed with no regressions from
88e4ebe1. Earlier assignments: **1025/1025**. Eleven personal programs validate
and execute; six rejection checks pass. Six reference edits have reduced proofs
and the pinned bundle revision in [reference corrections](reference-corrections.md).

## Frozen evidence

A: 88e4ebe1, SHA256 `14fe0b2503b191016f0c2a3bdbbd70c6d518f86432dbf3c615b5381fe4bacbc1`.
B: 3b8fc5a3, SHA256 `5ad6bb599da4cadcb0490547195043481054b97c139578dbea77486f39af3cab`.
The [protocol](../student.tests/pa11/performance-protocol.md#layout-and-initializer-continuation-campaign)
fixes flags, source generators, CPU affinity and AAAA/ABBA/ABBA order. Compiler
wall/peak RSS and native execution are measured separately; telemetry is separate.
All common LowIR outputs are equivalent under the unchanged course comparison;
semantic dumps are byte-identical. All native programs return zero.

Compiler `.text`: **672,966 → 723,590 bytes (+50,624, 7.52%)**. The additions
implement required semantics; no optional optimizer or speedup is proposed.

## Common compiler corpus

| Group | Median A/B wall (ms) | Max A/B RSS (KiB) | B/A ABBA blocks |
| --- | ---: | ---: | ---: |
| calls-1 | 403.33 / 399.81 | 73,644 / 73,628 | 0.9904 / 0.9990 |
| memory-float-1 | 343.81 / 344.68 | 67,504 / 67,456 | 1.0082 / 1.0130 |
| references-1 | 16.02 / 16.02 | 6,528 / 6,532 | 0.9970 / 0.9856 |
| template-semantics-1 | 68.02 / 68.72 | 12,512 / 12,580 | 1.0141 / 1.0029 |
| calls-4 | 1627.81 / 1617.00 | 281,780 / 281,780 | 0.9435 / 0.9845 |
| memory-float-4 | 1372.10 / 1371.53 | 241,688 / 241,884 | 0.9572 / 0.9477 |
| references-4 | 48.69 / 48.47 | 12,888 / 12,932 | 1.0010 / 0.9930 |
| template-semantics-4 | 261.92 / 259.13 | 37,100 / 37,116 | 0.9907 / 0.6656 |
| references-8000 | 116.16 / 113.41 | 24,596 / 24,816 | 0.9744 / 0.9862 |

Median differences span -2.37% to +1.02%; the largest RSS increase
is 220 KiB. The common campaign contains one-sided wall spikes in calls-1,
memory-float-1, calls-4, memory-float-4 and template-semantics-4. The first two
have 32.5%/73.1% A/A spread; several later A-side spikes distort paired ratios.
Repeat exactly those five groups with unchanged binaries/inputs/order. The
[60-observation follow-up](../student.tests/pa11/layout-repeat-performance.json)
has B/A blocks from 0.9890 to 1.0096. Template A/A calibration remains noisy
(49.3%); this is retained rather than interpreted as an improvement.

The [initial common campaign](../student.tests/pa11/layout-initial-common-performance.json)
is retained in full under its original binary hash. Its memory-float-4 RSS
increase was 14,136 KiB; the final compiler does not reproduce it. A separate
[telemetry diagnostic](../student.tests/pa11/layout-memory-diagnostic.json)
preserves equal work/IR counts and pool capacities for that initial pair.
No allocation cause or memory optimization benefit is claimed.

## New behavior and bounded ranges

A lacks these semantics, so the following are B-only AAAA observations, not
speed comparisons. The full [behavior data](../student.tests/pa11/layout-behavior-performance.json)
includes hashes, every wall/RSS observation, separate telemetry and native checks.

| Input | Median wall (ms) | Max RSS (KiB) | Initializer actions / instructions |
| --- | ---: | ---: | ---: |
| layouts-1000 | 125.75 | 28,748 | 4,000 / 38,004 |
| initializers-1000 | 168.29 | 34,536 | 8,000 / 56,004 |
| layouts-4000 | 502.34 | 101,492 | 16,000 / 152,004 |
| initializers-4000 | 690.95 | 132,904 | 32,000 / 224,004 |
| zero-range-32 | 5.82 | 4,740 | 3 / 10 |
| zero-range-1000000 | 5.78 | 4,740 | 3 / 10 |
| string-padding-32 | 5.63 | 4,740 | 1 / 27 |
| string-padding-1000000 | 5.58 | 4,740 | 1 / 27 |
| volatile-range-32 | 5.58 | 4,740 | 3 / 31 |
| volatile-range-1000000 | 5.60 | 4,740 | 3 / 31 |

Fourfold family growth gives 3.99× / 4.11× wall and 3.53× / 3.85× RSS.
Layout families have 4N initializer actions and 38N+4 instructions; initializer
families have 8N actions and 56N+4 instructions. Their wall ranges are
0.1252–0.1268 / 0.5013–0.5085 seconds and 0.1663–0.1825 / 0.6777–0.6978 seconds.

Range timings are startup-scale diagnostics. Raising bounds from 32 to one
million keeps the semantic action counts and instruction counts unchanged:
ordinary omitted ranges use a bulk zero, string padding uses a bulk zero after
the explicit characters, and volatile omitted ranges use scalar volatile stores
in a counter loop. This preserves the eight-element expansion budget. `_Pragma`
spelling scratch is released after processing its directive.

## Generated programs

| Common runtime | Median A/B (s) | A/B text payload (bytes) |
| --- | ---: | ---: |
| calls-long | 0.479850 / 0.476264 | 206 / 206 |
| memory-long | 0.279528 / 0.282433 | 434 / 434 |
| floating-long | 0.331782 / 0.331749 | 230 / 230 |

Each common A/B executable is byte-identical. The supplied sectionless backend
is unchanged; the payload after the ELF entry is the text proxy for these
programs without static data.

The new 48-million-iteration bit-field runtime has median **0.387292 s**
(range 0.387041–0.387367), **393 payload bytes**, and 256 KiB observed RSS.
A volatile loop bound and checked checksum exercise assignment, signed reads
and adjacent-field preservation. There is no valid A speed baseline.

## Acceptance and remaining work

The spec’s stage-scoped rule applies: inherited numeric ratio/RSS/text targets
are diagnostics, not extra exit gates. No historical observation is erased.
Correctness, coverage, proportional work and the documented expansion bound
remain required. The identified scratch-lifetime and padding-growth defects
were corrected before the final campaign. Final hashes, all observations,
native outcomes and range IR bounds were verified explicitly.

The remaining 29 failures require constructor conversion/inheritance and
constant-evaluation facts, ABI/TLS entry identities, declaration/parser context,
or boundary/value-category decisions. Their owners and concrete data-flow
boundaries are recorded in [the plan](plan.md).
