# PA12 implementation and final audit plan

Stage base: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`.
Audited entry: `18ef3757`; final implementation: `0768b868` (following `b7a15e3e`).
Target: **pa12 full-stage**. Phase: **audit complete**.
Result: **257/257 PA12**, **1584/1584 through-stage**, **12/12 stages**.

## Final design/spec alignment

The [whole-stage audit](audit.md) independently reconstructs source/cursor,
shared parser/semantic graph, canonical identities, indexed lookup/demand,
complete cache keys, typed lowering and ownership/release points. PA12 owns
nonvirtual C++11 class semantics and O0 LowIR; the supplied PA8 backend is an
external execution boundary. Native MIR/ELF and self-hosting remain later stages.

| Owner | Final design and validation |
| --- | --- |
| Class members and transfers | Selected member/default/delegation facts, prepared layout-unit actions, separate parameter/result ABI and constructor entries; no semantic replay in lowering. |
| Values and lifetimes | Destination-based transfers, full-expression regions, function-owned return cleanup and bounded destructor suffix sharing; guards follow successful materialization. |
| Lists and references | Complete qualified target type through list plans/helpers. Reference projections retain the complete temporary; conditional alternatives share a lexical lifetime with per-object guards. Non-extending expressions stop the projection walk. |
| Allocation and zeroing | Prepared typed zero-plan IDs reach runtime array loops; member-pointer null values and volatile scalar initialization retain required representation/access semantics. |
| Parameter and scalar facts | ABI/body queries are independent of emission. Sparse object observations invalidate private scalar truth proofs; final conversion/store precedes observable cleanup. Unknown conditions retain shared cleanup. |

No unreviewed PA12 handoff remains. The audit repaired local subobject-reference
lifetimes and conditional cleanup, static reference over-extension, qualified
list temporary stores, and typed heap-array zeroing across their semantic and
lowering owners. Five new execution reducers, work/IR properties and sanitizer
runs cover these paths. No new reference/fixture/comparator edit was needed.

## Performance acceptance and work limits

[Final evidence and inherited acceptance ledger](final-audit-performance.md)
links all 15 checkpoint reports and 39 historical campaigns, plus two fresh
frozen comparisons. It retains A/A calibration, two ABBA blocks, every outlier,
compiler latency/peak RSS, executable runtime/payload and hashes. Common final
A/B outputs are identical. Incorrect old paths have absolute B-only measurements.

- Local array expansion remains capped at eight total nested elements. Zero
  plans share typed child actions, cap padding expansion at eight and use loops
  beyond the bound. Heap extents 19 and 1,000,000 both produce 25 instructions.
- Destructor suffixes inline at most eight actions (28 duplicated tail actions),
  then share blocks. Classifiers use at most three bytes per AST node; scalar
  transfer proof adds one lazy byte per examined node. Member-pointer values
  occupy at most 16 bytes per value/boundary.
- Conditional reference alternatives, source visits and output grow linearly:
  depth 32/128 gives 33/129 alternatives, 98/386 visits and 600/2328 instructions.
  Facts have explicit class/type/source/context owners; no global retry or
  whole-graph copy was added. Function scratch is released per function.
- Final fixes add 576 compiler text bytes (.059%); large common compile pairs
  are mixed, with stable memory and identical native images. A noisy template
  wall-time loss is retained; a focused frozen repeat records a .496-second
  row with .25 seconds child CPU, matching ordinary rows' CPU cost. Full-stage
  text grows 194944 bytes (24.81%); the large calls corpus costs 2.17% latency and
  0.81% peak RSS (initial campaign: 4.35% / 6.00%). These costs and noisy runtime
  observations remain disclosed.
- The inherited 128 KiB text-growth target and feature 4/6/8 KiB/5% targets are
  diagnostic review signals, not mandated PA12 gates. The whole-stage text miss
  is reclassified with evidence, preserving all observations and required work.
- Unprofitable optional scalar-copy prefixes and dynamic scalar consumption were
  removed. Required ABI, storage-prefix, object-boundary and constant-condition
  ordering costs remain documented without speed claims. The proven native
  code/data placement constraint belongs to PA24. Smaller IR never substitutes
  for measured runtime profit, and no positive-runtime/self-hosting gate is added.

## Audited handoff ledger

| Implementation group (documentation/evidence commits also reviewed) | PA12 passing |
| --- | ---: |
| Stage entry | 61/257 |
| `4e209677`, `55d15ba8`, `8a83045c`: members, delegation, unions | 92/257 |
| `73664568`, `4ea8394f`: transfer actions and measured prefix correction | 123/257 |
| `cc2198c9` through `1a7867fd`: values, conditional storage and shared records | 169/257 |
| `02f2f154`, `77145afa`, `f9c8e6c3`, `70556b3d`: conversions/static references | 202/257 |
| `d2db8706` through `aee24d98`: allocation, aggregate transfers, aliases | 226/257 |
| `64ecedf7`, `3ec8d0ce`: lists and independent result ABI | 234/257 |
| `8bf45f86`, `e98cc21e`: full-expression and condition cleanup | 240/257 |
| `951799ed`, `4cefbabe`: effects, retained boundaries, bounded suffixes | 248/257 |
| `ce2d8363`, `8cfcc3fe`: member pointers and assignment widths | 249/257 |
| `b5645333`, `574bf3f0`, `b1e936e8`: typed zeroing and defined reducers | 250/257 |
| `260e0b35`: terminal return ownership | 252/257 |
| `14dac876`, `3789b6a3`, `9e442405`: transfer proof, guards, native layout control | 253/257 |
| `c61ecc10`: constructor storage-unit order | 254/257 |
| `8a5a370d`, `b699f183`, `1eae0974`, `2d693f2b`, `eb0a4251`: conversion boundaries and measured elision | 255/257 |
| `f3e7ce93`, `51fbedef`: independent parameter/body query and transport cost | 256/257 |
| `7e88ba59`, `67947601`, `18ef3757`: scalar consumption, removed optional regression | 257/257 |
| `b7a15e3e`, `0768b868`: lifetime projections, cv-list and typed heap-zero ownership repairs | 257/257 |

`3da4de09` is the sole reference correction: four bit-field retypes, with
[reduced C++11/LowIR proof and bundle revision](reference-corrections.md).
The audit reran the good/bad IR controls and reviewed the exact four-site diff.
All earlier group handoffs, including the post-checkpoint parameter/scalar
owners, are now covered by the independent final review.

## Exit validation and evidence integrity

Required file audit and `make test-report-through-pa12` pass. PA12 includes
13/13 behavioral/query controls; inherited stages pass 1327/1327. The supplied
state's 1608 total is a reporting mismatch: entry, final and provided Ralph log
all show 1584 tests. Coverage and comparison rules are unchanged; 196 stage-base
failures were removed.

All 72 personal sources, four explicit property scripts (including
`audit_check.py`) and strict good/bad bit-field IR controls pass. The actual final
compiler passes all personal/audit checks under ASan/UBSan. Frozen hashes and
native results verify; two historical A output pairs overwritten in scratch
reproduce with their original hashes in separate storage. All 6488 historical
wall observations and 934 historical output hashes remain verifiable.

Logs/frozen artifacts: `$RALPH_ARTIFACT_DIR/pa12-final-audit/`. Reproduction
commands and exact measurements are in the linked audit/performance documents.
Implementation and evidence commits contain no generated objects/logs/outputs.
Remaining PA12 work: **none**; later-stage surfaces are explicitly scoped above.
