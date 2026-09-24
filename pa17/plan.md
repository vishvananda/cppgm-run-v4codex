# PA17 compact plan — implementation handoff, loop 57

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `e14b96fa9d4b3376e5922b8ad30093c3c0b0c759`

Target: **PA17 full-stage**. Implementation remains incomplete; do not advance.
Loop 57 entered clean at `69152966`, **324/343** (19 failures). Previous turn
was verified progress: committed audit fixes and validation. Code `794b150e`
now passes **330/343**: six original failures closed, no new failures, all 343
course inputs unchanged. PA1–PA16 pass **2266/2266**; through PA17 is
**2596/2609**. **493/493 personal controls** pass (465 inherited, 28 new).
File audit passes with the same three header-division warnings. No course test,
reference, bundle revision or comparison rule changed.

| Completed behavior owner | Data flow / spec alignment |
|---|---|
| Scalar and object transfer | Semantic conversions retain final specialized member immediates; special-member facts retain memberwise actions around trivial empty subobjects. Nontrivial/volatile/reference/union behavior stays explicit. Lowering consumes selected actions and typed conversions. |
| Base adjustment | Canonical entity-pair paths cache reachability, ambiguity and misses separately from demanded layout; flat index/vector tails live for one TU. Qualified calls retain receiver→qualifier→declaration provenance. Constant addresses and runtime conversions share offsets. Candidate failure needs no exception or premature layout. |
| Automatic initialization | Related investigation closed both array failures: short wide-integer arrays use bounded stores, evaluated class materializations retain initialization actions. Required constexpr facts and established scalar images remain independent. No grammar replay or IR roundtrip. |

[Handoff trace and ownership](transfer-handoff.md), [performance](transfer-performance.md)
and the [evidence manifest](../student.tests/pa17/transfer-evidence.json) bind the
source tip, checks, unchanged coverage, trace and frozen observations. The first
performance preflight found a fixed-base template query demanding layout; its
reducer and interrupted campaign are preserved, and the corrected owner passes.

Performance: 18 frozen workloads; A/A calibration plus four ABBA blocks; latency,
peak RSS, runtime and code/data sizes recorded together. Compiler .text +3,072 B;
maximum observed extra peak RSS +2,324 KiB. Sparse transfer runtime B/A **0.361**,
code −2 B. Eight-lane wide-array B/A **0.967**, code +9 B, data −64 B. Two-lane
arrays have no proven runtime gain; code/data shrink. Compiler-cost/noise
limitations are disclosed. Base-path work grows 1,202→4,802 for 4× source.
Eight-lane expansion and existing bounded array fallbacks remain enforced.
PA17/O0 has no mandated numeric ceiling; historical +15%, +16 MiB and 5.5×
diagnostic targets remain diagnostic under spec.md §9. No requirements are waived.

| Unfinished implementation owner | Failures | Next work |
|---|---:|---|
| Query/candidate demand and closure entities | 4 | Recursive ADL/class-selection dependencies, ambiguous-operator expected failure, closure callable/body/lifetime ownership. |
| Static storage and initialization | 7 | Static/member definition publication, local guards, static/dynamic reference and function addresses, constant reads. |
| Exception cleanup scheduling | 2 | Required call-region boundaries and cleanup ordering. |

Handoff boundary: all four transfer/adjustment failures and the two related
automatic-array failures are closed. Remaining static cases need declaration and
storage-demand facts established before these consumers; changing transfer
lowering cannot correctly resolve their publication/guard/relocation ownership.
Recursive query/closure demand and EH scheduling have distinct state machines.
Those 13 cases remain implementation work. This boundary closes a broad owner
and its neighboring initialization behavior; six passing additions alone would
not constitute progress.

Independent review remains open for base-graph key validity across pattern and
concrete publication, qualifier provenance on both retained/substituted call
paths, and the recorded O0 work/growth evidence. These are review questions,
separate from the 13 unfinished cases; neither category is waived.

| Loop / phase | Handoff ledger |
|---|---|
| 56 / checkpointAudit | `c43e8eb6..e14b96fa`; accumulated review complete; 324/343, same 19 failures; earlier 2266/2266; 465 controls; file audit/performance accepted. [Audit](audit.md) and [evidence](checkpoint56-performance.md) preserved. |
| 57 / implement | `69152966..794b150e`; transfers/adjustments plus automatic initialization; 330/343, six original failures closed; earlier 2266/2266; 493 controls; file audit/performance pass. Three implementation owners and independent review remain. |

Run `python3 student.tests/pa17/verify_transfer.py`. Historical verifiers describe
their frozen tips. Review markers above are preserved for Ralph's next audit.
