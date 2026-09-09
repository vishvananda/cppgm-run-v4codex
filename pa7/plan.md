# PA7 final plan and ledger

Target: **PA7 full-stage**. Phase: **complete**.
Stage base commit: `c02f4ea09653422aa87156c33defbea28a08cb76`.
Last reviewed commit: `9cceb7913`.
Entry: **0/186**. Final: **186/186**; PA1–7 **684/684**, **7/7 stages**.

## Final Spec Alignment

The independent [audit](audit.md) reconstructs source -> streamed tokens -> one
source graph -> canonical types/entities/scopes -> recorded expression,
conversion, constant and demand facts -> deterministic semantic output. There
is no second tree, token replay, textual transport or reference/host semantic
implementation. General class/template/constant-evaluation closure and native
optimization remain later milestones; executable runtime/text and ABI/debug
encoding are N/A at PA7.

| Completed work | Authoritative evidence |
| --- | --- |
| Spec/handout/history and whole-stage architecture | All implementation handoffs since the PA6 base reviewed independently; source/fact/ownership traces in the audit. |
| Correctness and semantic handoff fixes | `2fd1e8333`: value/target identity, conversions, arithmetic/cv ranking, constants, pointer/cast legality, control scopes and unevaluated demand. `9cceb7913`: cast/builtin forms stay on their originating nodes. |
| Independent validation | 63 new probes (pre-audit binary fails 49), original 29 plus multi-TU isolation, expanded typed API, ASan/UBSan/leak checks and inherited personal/API checks pass. Rejections require exit 1; crashes or sanitizer reports do not count as valid rejection. |
| Required exit gates | `make test-pa7` 186/186; `make test-report-through-pa7` 684/684; file audit 79 files passes on final source. |
| Compiler cost and budgets | Complete fixed-corpus coverage plus a verified longer AST group and direct PA7 comparison. All 1,154 audit observations retained, including failed/superseded timing. |
| Consolidation | Plan, independent audit, performance evidence, rerun controls and review ledger complete; intended changes committed and clean status checked at final handoff. |

## Performance contract and result

Frozen A/B binaries and inputs, A/A calibration, B/B probes, two ABBA blocks,
separate telemetry, output equivalence and 1x/4x sources remain the protocol.
Unchanged budgets: paired wall <=10% + A/A noise; RSS <=20% +1 MiB; stage host
text growth <=50%; 4x source <6x wall / <5x RSS +1 MiB; samples >20x startup.

[performance.md](performance.md) records both passing coverage and the noisy
rejected samples. The short final AST-declaration failure is superseded by a
verified comparison of the same source group with 16 TUs/run. Source/binary/
host/CPU identities and every original output/work check still verify.

The direct PA7 campaign (8 TUs/run) ranges from 1.13% faster to 5.29% slower;
large loop/memory/floating-point compiler latency rises 4.91–5.29% to retain
required conversion facts. That regression and 5.28% maximum RSS growth fit the
unchanged budgets. Host .text is 358,790 bytes (+29.20% from PA6, +2.54% from
pre-audit PA7). PA7 wall scaling is 4.037–4.073x. No speedup or executable
optimization claim is made; all noise and telemetry overhead remain disclosed.

## Handoff ledger

`c2837a0e6`, `ec0c55093`, `ec6b78425`, `d5ab80eb4` and `682532c07` were inspected,
not accepted as independent proof. Their remaining ownership defects are fixed
in `2fd1e8333` and `9cceb7913`. The final evidence commit records the architecture,
63 independent probes, full validation and all timing observations. No earlier
PA7 handoff remains unaudited and no PA7 work remains. No advance to PA8 occurred.
