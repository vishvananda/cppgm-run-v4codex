# PA17 implementation handoff — loop 61

Implementation tip: `7815c1508915aca259a8b554d485ac8f36b7d39f`; entry:
`9efd570fd4a0489a39d17fdabd0b05bf062604a6`. PA17 improves **340 → 343 / 343**;
earlier assignments pass **2266/2266**, cumulative **2609/2609**. File audit
passes with the same three inherited header-ownership warnings. No course
fixture, reference, status or comparison rule changed in this turn.

## Completed semantic owners

- Full-expression regions protect live local cleanup and activate temporary
  suffixes before subsequent calls. Nonthrowing class calls retain the course's
  structural regions. The two original cleanup failures now pass.
- Each captureless lambda is keyed by its parsed occurrence and substitution
  context, not its token anchor. The ordinary canonical class/call-operator graph
  owns its signature, body, temporaries and ABI identity. Retained template
  signatures publish the actual parameter types, including packs and dependent
  trailing returns. This closes the chained nonprimary member-template failure.
- `auto` declarations use typed deduction over a canonical pending object, so
  local shadowing cannot accidentally resolve self-initialization to an outer
  object. Function-address selection checks deletion/access before emission.
- Captureless conversion adapters are typed semantic functions emitted only on
  demand: conversion → thunk → the same checked operator body. Hidden result
  and class parameter conventions use the existing LowIR ABI. No body cloning,
  syntax fabrication, grammar replay or native frontend delegation is used.
- The closure ABI receiver is distinct from lexical `this`; unevaluated member
  context queries use the enclosing member, including nested/template lambdas.

The governing rules are [N3485](../doc/n3485.txt) §5.1.2 paragraphs 2–7 and
19–20 (unique class/prvalue, strict single-return inference, const/mutable
operator, function-pointer conversion, lexical body context and special members),
§7.1.6.4 (`auto` deduction), §3.3.2 (point of declaration) and §8.4.3 (deleted
functions). Lambda ABI numbering follows
[Itanium ABI §5.1.8](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-closure-types):
the first signature in an enclosing context is unsuffixed; subsequent equal
signatures use the ABI discriminator. The inspection fact adapter adds `first`
and an ellipsis signature flag while preserving the existing PA9 numeric forms.
There are **no reference corrections** in this handoff.

Work/storage bounds: one closure/class/operator record and body check per complete
occurrence/context; source bodies bind once, concrete signatures use existing
substitution facts. Two adapter identities per closure emit at most two short
bodies, proportional to parameters, when demanded. Flat indices own fact lookup;
ABI numbering sorts the closure vector once, **O(L log L)**. Expression effects
are memoized, and cleanup suffixes are shared, proportional to expressions and
lifetime actions. These are translation-unit owned structures; no persistent
cache or cross-product scan is introduced.

## Validation and handoff ledger

`make test-pa17`, `make test-report-through-pa16`,
`make test-report-through-pa17`, and
`perl scripts/cppgm_file_audit.pl --stage pa17 --paths dev/src` pass.
All **704 PA17 personal checks** pass: 610 inherited, 71 closure/placeholder,
10 cleanup native checks, five region predicates, and eight ABI checks (seven
exact fact roundtrips plus source numbering/native execution). On the frozen
entry, 51/71 closure controls and four/five region predicates fail; the native
cleanup results remain correct while region placement violates the required
contract. The existing PA9 direct-API, 117 status/output/roundtrip, valid/rejected
probe and final-boundary suites also pass.

The increments are `2827d328` (cleanup), `bb1d961d` (closures and consumers),
and `7815c150` (lexical unevaluated member context). A final evidence commit
records these results and strengthens the region predicates to check that both
operand calls occupy one region. Required behavior in both original groups is
complete. Captures and initializer-list deduction were not added to the
implemented surface. **Independent full-stage review remains Ralph's next
phase**, including accumulated stage findings; this implementation handoff does
not certify that audit. Review markers in [plan.md](plan.md) remain unchanged.

## Performance protocol and acceptance

Run `student.tests/pa17/handoff61_benchmark.py A B WORK OUT` with the two frozen
compilers. [Final raw observations](../student.tests/pa17/handoff61-performance.json)
include all sources/hashes, binary hashes, build flags, backend bundle/hash,
commands, wall/user/system time, RSS, context switches, telemetry, native output
hashes and sizes. The [first run](../student.tests/pa17/handoff61-prelexical-performance.json)
at `bb1d961d` is preserved in full; the final run includes the lexical correction.
Both use one warmup each, four A/A observations, then four ABBA blocks; rejected
entry cases receive six final-only observations. Timing uses one allowed CPU,
with no concurrent test or build jobs. Every compared native executable returns
its checked result; common LowIR and executable outputs are byte-identical.
Closure inputs rejected by entry are never used for speedup comparisons.

PA17/O0 mandates no numeric latency/RSS/text ceiling. No optional optimization
is introduced. The explicit work/growth bounds above apply; historical +15%,
+16 MiB and 5.5× self-selected targets remain diagnostics under spec.md §9, not
extra exit gates. The evidence records necessary semantic costs and rejects
unsupported general speedup claims. Native backend implementation and
self-hosting measurements remain owned by later assignments; the supplied
LowIR backend runs the source-generated IR here.

Compiler build: `g++ -std=gnu++11 -Wall -O3`, course test-runner entry wrapper;
measured driver flags `--emit-lowir -O0`. Telemetry/validation is collected
separately from timed compilation. Compiler `.text` increases from
1,791,558 to 1,813,958 bytes (+22,400, 1.25%).

| Compiler workload | Entry/final median ms | Entry/final peak RSS KiB | Median paired B/A (block range) |
|---|---:|---:|---:|
| common-partials-1500 | 111.97 / 112.49 | 22412 / 22748 | 0.9988 (0.9914–1.0055) |
| common-partials-6000 | 458.54 / 460.97 | 72916 / 73232 | 1.0063 (0.9964–1.0168) |
| common-loop-float-1500 | 152.84 / 153.65 | 30336 / 30140 | 1.0036 (0.9969–1.0148) |
| common-loop-float-6000 | 632.54 / 611.38 | 103536 / 103644 | 0.9409 (0.8708–0.9999) |
| regions-600 | 60.22 / 63.40 | 15912 / 15752 | 1.0471 (0.4132–1.0618) |
| closures-600 | rejected / 116.64 | — / 23320 | final only |
| regions-2400 | 231.13 / 245.19 | 47244 / 47628 | 1.0619 (1.0572–1.1823) |
| closures-2400 | rejected / 472.54 | — / 74056 | final only |

Empty-source startup is about 5.6 ms. The runtime sources' 6 ms compilation
measurements are retained as diagnostics, not compiler speedup evidence. Common
frontend rows span 0.9409–1.0063 in the final run, versus 0.9955–1.0773 in the
first run. The large loop row reverses direction and includes scheduling noise;
there is no repeatable general compiler gain or loss. All raw block observations,
including the regions-600 entry outlier, remain present; no samples were removed.

The larger affected cleanup case costs about 6.2% compiler latency (+384 KiB
peak RSS) to produce the required full-expression structure. This is consistent
with the first run's 5.7% cost. At 600 → 2400 instances, final closure latency
scales 4.05× and RSS 3.18×; region latency scales 3.87× and RSS 3.02×. Closure
counts are exactly 600/2400; body checks 1801/7201; template source binding work
stays 35. Region `full_expression_work` is 19212/76812. The observed work and
storage follow the declared bounds.

Executable timings include volatile loop bounds and checked effects/results.
Since this backend emits sectionless ELF, native “text” below means executable
payload minus typed global data (code plus alignment); raw payload/data/file
sizes are also retained. Compiler text uses the actual `.text` section.

| Runtime workload | Entry/final median seconds | Paired B/A (block range) | Entry/final code + alignment bytes |
|---|---:|---:|---:|
| runtime-calls | 0.7195 / 0.7201 | 0.9838 (0.9500–1.0057) | 206 / 206 |
| runtime-memory | 0.4171 / 0.4172 | 1.0009 (0.9932–1.0056) | 434 / 434 |
| runtime-floating | 0.4968 / 0.4976 | 1.0002 (0.9948–1.0163) | 230 / 230 |
| runtime-regions | 1.0385 / 0.7721 | 0.7452 (0.7360–0.7521) | 1132 / 1020 |
| runtime-closures | rejected / 0.6765 | final only | — / 316 |

Common runtimes have identical binaries; observed ratios are timing variation.
The required region grouping repeats its runtime benefit: B/A **0.7400** in the
first run and **0.7452** in the final run, while code shrinks **112 bytes**. It
avoids repeated runtime entry/exit of cleanup regions around adjacent calls.
This affected benefit accompanies the disclosed compiler/IR growth; it does not
excuse a correctness or coverage reduction. New closure runtime has no valid
entry counterpart. No self-imposed historical performance gate is used to block
or certify this handoff.

[Evidence manifest](../student.tests/pa17/handoff61-evidence.json) binds the
implementation tree, unchanged course trees and all artifacts. Run
`python3 student.tests/pa17/verify_handoff61.py` to check the frozen records;
that verifier supplements the explicit test commands above.
