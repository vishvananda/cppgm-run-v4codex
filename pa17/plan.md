# PA17 compact plan — implementation handoff, loop 59

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `e14b96fa9d4b3376e5922b8ad30093c3c0b0c759`

Target: **PA17 full-stage**. Implementation remains incomplete; do not advance.
Loop 59 entered clean at `7cc89281`, **333/343** (ten failures). Previous turn:
verified progress; no live compiler/test process remained. Code `6a1a51a8` with
proved reference corrections `3c7eeea2` passes **340/343**: seven original failures
closed, none added, unchanged 343-case coverage. Earlier PAs pass **2266/2266**;
through PA17 is **2606/2609**. All **567 personal controls** pass (530 inherited,
37 added); file audit passes with the same three header-division warnings.

| Completed owner | Design/spec alignment |
|---|---|
| Static member definition ownership | Retained prototype → canonical head/type signature → definition link → entity-keyed storage demand. Duplicate/mismatched/nonstatic definitions reject, including undemanded templates. No global retries or token replay. |
| Dependent signature identity | Memoized signature-only query normalization retains bound entities and canonical head ordinals. Original queries retain access environments for concrete substitution; signature keys are not instantiation inputs. |
| Read publication | Ordinary O0 and substituted uses establish their constant snapshots at their respective source/instantiation boundaries. Member lowering consumes the recorded read fact. PA14 instantiated constant reads remain preserved. |
| Address initializers | Typed single-function addresses and target-selected overload families feed shared initializer recipes, concrete demand and static relocation emission. No pointer-to-unknown recovery or lowering-time resolution. |

[Ownership and boundary](storage-handoff.md), [reference proofs](storage-references.md),
[performance](storage-performance.md), and [evidence manifest](../student.tests/pa17/storage-evidence.json)
bind this handoff. Six oracle corrections preserve all source/status fixtures,
coverage and comparison rules. The bundle stays pinned at `c2f713cd70d0`; reduced
reference executions expose delayed reference binding and unwanted dormant-member
initialization. Native source→typed LowIR→supplied backend trace is checked.

Performance: 14 workloads; frozen binaries/inputs, A/A calibration and four ABBA
blocks for common correct inputs; new inputs have final-only costs. Compiler
.text +2,752 B, entity records unchanged at 120 B, maximum extra observed RSS
624 KiB. Common paired compiler medians 0.9696–1.0246. Unchanged runtime binaries
are identical; changed O0 static reads add eight code bytes and measure 1.0206×
runtime. This disclosed contract cost is not an optimization claim. At 4× scale,
static fact/signature work is 4× and latency is 3.91–3.99×. PA17/O0 mandates no
numeric ceiling; historical +15%, +16 MiB and 5.5× targets remain diagnostic under
spec.md §9. No optional transform or unbounded work/growth is added.

| Unfinished implementation owner | Failures | Required next work |
|---|---:|---|
| Closure entities | 1 | Semantic lambda identity, call operator/body binding, class value and lifetime/lowering facts. |
| Exception cleanup scheduling | 2 | Call-region boundaries and temporary/automatic cleanup order. |

Handoff boundary: the storage-definition/initialization/read group and its
neighboring duplicate-definition, dependent-signature and function-address
problems are closed. The three remaining failures require closure construction
or exception-region scheduling facts; further storage/relocation changes cannot
supply those distinct state machines. They remain unfinished implementation.

Independent review remains open for signature-only context normalization,
source/instantiation read snapshots and address-query access provenance. Preserve
active candidate keys across member/partial heads, query failure-cache validity,
immediate-context restoration, base-graph keys, qualified receiver provenance,
and O0 work/growth questions from previous handoffs. Neither review nor remaining
implementation is waived.

| Loop / phase | Handoff ledger |
|---|---|
| 56 / checkpointAudit | `c43e8eb6..e14b96fa`; review complete; 324/343; prior 2266/2266; 465 controls; [audit](audit.md) preserved. |
| 57 / implement | `69152966..794b150e`; transfers/adjustments and automatic initialization; 330/343; prior 2266/2266; 493 controls; [evidence](transfer-handoff.md) preserved. |
| 58 / implement | `411ad00e..f284514b`; query/candidate demand and deletion/default ownership; 333/343; prior 2266/2266; 530 controls; [evidence](query-handoff.md) preserved. |
| 59 / implement | `7cc89281..3c7eeea2`; storage definitions/read facts/address queries plus six proved oracle corrections; 340/343; prior 2266/2266; 567 controls; frozen performance and file audit pass. Three implementation cases and independent review remain. |

Run `python3 student.tests/pa17/verify_storage.py`. Historical verifiers describe
their frozen tips. Review markers are preserved for Ralph's next audit.
