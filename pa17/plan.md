# PA17 compact plan — implementation handoff, loop 58

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `e14b96fa9d4b3376e5922b8ad30093c3c0b0c759`

Target: **PA17 full-stage**. Implementation remains incomplete; do not advance.
Loop 58 entered clean at `411ad00e`, **330/343** (13 failures). The previous
turn was verified progress; no live process remained from it. Code `f284514b`
passes **333/343**: three original failures closed, no new failures and unchanged
343-case coverage. PA1–PA16 pass **2266/2266**; through PA17 is **2599/2609**.
All **530 personal controls** pass (493 inherited, 37 new). File audit passes
with the same three header-division warnings. No reference, bundle revision,
course input or comparison rule changed.

| Completed owner | Data flow and spec alignment |
|---|---|
| Recursive candidate defaults | Deduction → full canonical argument tuple, including holes → active `(template head, tuple)` fact → default/signature substitution → specialization. Recursive demand excludes only the active candidate. No global generation, retry scan or exception-based ordinary failure. |
| Operator/call/member queries | Substituted canonical query → compact failure or selected call/conversions → partial selection and retained semantic facts → typed LowIR. Failed `decltype` stays a failure sentinel. Fixed source errors and class-body side effects remain hard errors. |
| Deleted function ownership | Declaration entity → concrete function specialization → selected call/address check. Explicit specialization replaces primary deletion. Definition ordering and unselected deleted overloads are covered. The flag fits the existing 120-byte entity. |
| Related default holes | Missing defaults retain later deduced parameters, including distinct subsequent bindings and later default declarations. Active marks expire with the candidate; they are not negative results reused across publication. |

[Ownership, standard anchors and boundary](query-handoff.md),
[performance evidence](query-performance.md), and the
[evidence manifest](../student.tests/pa17/query-evidence.json) bind this handoff.
The source→query→specialization→LowIR→supplied-backend trace is retained with
hashes and a checked runtime result. The compiler does not invoke the backend.

Performance: 16 final workloads, A/A calibration and four ABBA blocks on
common correct inputs; newly accepted inputs have final-only costs. Compiler
.text +5,312 B; entity size remains 120 B; maximum extra observed RSS +1,676 KiB.
Common compiler paired medians 0.9975–1.0209. Common runtime binaries and sizes
are identical. Recursive inputs at 4× scale produce 4× candidate/cycle work;
canonical query computations equal query counts. Both preliminary campaigns
remain preserved. No optimization benefit is claimed and no optional transform
is added. PA17/O0 has no mandated numeric ceiling; historical +15%, +16 MiB and
5.5× targets remain diagnostic under spec.md §9. Work is bounded by candidates,
head widths and canonical queries; no generated-code expansion is introduced.

| Unfinished implementation owner | Failures | Next work |
|---|---:|---|
| Closure entities | 1 | Semantic lambda identity, call operator/body binding, class value and lifetime/lowering facts. |
| Static storage and initialization | 7 | Definition publication, local guards, static/dynamic reference and function addresses, constant reads. |
| Exception cleanup scheduling | 2 | Call-region boundaries and cleanup order. |

Handoff boundary: all three recursive/ambiguous query failures and the neighboring
deleted-function/default-hole defects are closed. The lambda case has only parser
support; creating its missing semantic entities and callable/lifetime facts is a
separate implementation. Storage publication and cleanup scheduling have distinct
owners upstream/downstream of query selection. Further changes within this query
owner cannot provide those missing facts. All ten remain implementation work.

Independent review remains open for active candidate keys across member/partial
heads, query failure-cache validity, and immediate-context restoration. Preserve
the previous base-graph key, qualified receiver provenance, and O0 work/growth
review questions as well. Neither review nor unfinished implementation is waived.

| Loop / phase | Handoff ledger |
|---|---|
| 56 / checkpointAudit | `c43e8eb6..e14b96fa`; accumulated review complete; 324/343; earlier 2266/2266; 465 controls; file audit/performance accepted. [Audit](audit.md) preserved. |
| 57 / implement | `69152966..794b150e`; transfers/adjustments and automatic initialization; 330/343, six failures closed; earlier 2266/2266; 493 controls; [evidence](transfer-handoff.md) preserved. |
| 58 / implement | `411ad00e..f284514b`; query/candidate demand and adjacent deletion/default ownership; 333/343, three failures closed; earlier 2266/2266; 530 controls. File audit and frozen performance evidence pass; independent review and ten implementation cases remain. |

Run `python3 student.tests/pa17/verify_query.py`. Historical verifiers describe
their frozen tips. Review markers above are preserved for Ralph's next audit.
