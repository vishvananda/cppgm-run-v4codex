# PA14 full-stage audit plan

Phase: **audit complete** for `pa14 full-stage / O0 LowIR`. PA13 base
`8af3c149454e4e43e441206e6978f4d1300e079b`; audit entry `78c2f13e`;
last checkpoint `7f401fa8`; final implementation `d359539c`. PA15 has not started.

The final [audit](audit.md) consolidates independent architecture reconstruction,
representative end-to-end traces, ownership fixes, keys/invalidation, pipeline
budgets, standards proofs, validation, performance acceptance and every handoff.
Historical checkpoint text and measurements remain in [implementation.md](implementation.md)
and [performance.md](performance.md); their open-status statements describe the
named earlier revision and are superseded by this final audit.

## Completed work

- Read the handout, spec, testing/reference rules, stage commits, source and plan;
  trace streaming source through one retained graph, canonical semantic facts,
  demanded specializations, object/lifetime/ABI owners and direct typed LowIR.
- Complete body/lifetime publication, fixed statement/initializer recipes,
  copy/direct/list policy and existing-destination ownership across consumers.
- Close source default initialization and query-only operator/call/condition/list
  obligations, including deletion/access, local class properties, casts,
  `decltype`, default arguments, block extern identity and declaration parsing.
- Fix the exact declarator-to-source recipe handoff; verify source M work versus
  concrete KM use, terminal queries and absence of query-induced body/actions.
- Freeze equivalent correct A/B inputs, outputs, flags and binaries; measure
  compiler and native latency/RSS/text separately with A/A and ABBA, retaining
  all observations and repeating affected/noisy workloads without concurrent work.
- Preserve mandatory correctness, coverage, comparison and complexity limits.
  Reclassify unsupported inherited numerical or historical-layout targets as
  diagnostics under the spec's PA14/O0 acceptance rule, with evidence retained.

## Final acceptance and ledger

Both required commands pass:

```sh
perl scripts/cppgm_file_audit.pl --stage pa14 --paths dev/src
make test-report-through-pa14
```

The authoritative report is **314 PA14 + 1621 prior = 1935/1935**, **14/14 stages**;
the supplied 1959 count is stale. [Validation](../student.tests/pa14/source-obligations-validation.json)
records 79 passing groups, unchanged hashes for 1,266 fixtures/references,
release/sanitizer and entry parity, all inherited controls and new source,
branch, property and lifetime checks. No reference or bundle was revised.
The [final command record](../student.tests/pa14/final-audit-closure.json) retains
the final implementation identity and both successful exit checks.

[Final performance](../student.tests/pa14/source-obligations-performance.md):
1,120 new observations, **19,726 cumulative**, all verified by
`python3 student.tests/pa14/verify_performance.py`. The affected K=128 operator
shape shows about 5% lower compiler latency in both campaigns. Native text
sizes are unchanged; no native speedup is claimed. All compiler text/record
costs, memory increases and timing outliers remain disclosed. PA14 has no
mandated numerical latency/RSS/text cap and adds no optional optimizer.

| Continuation | Cumulative observations |
|---|---:|
| Source/signature/demand/virtual/lifecycle | 14,896 |
| Default slots, dependencies, access and elision | 15,848 |
| Body/lifetime publication and statements | 16,380 |
| Initializer recipes | 16,996 |
| Copy/direct/list modes | 17,724 |
| Existing destination ownership | 18,606 |
| Final source obligations and concrete consumers | **19,726** |

All five last-checkpoint handoffs are [closed](../student.tests/pa14/source-obligations-handoff.json)
under both builds; no unaudited PA14 handoff remains. Later template tiers,
student native backend/ELF, optimization levels and self-hosting retain their
handout boundaries. Raw evidence and lossless probe archives are under
`$RALPH_ARTIFACT_DIR/pa14-final-audit/`; the [journal](../student.tests/pa14/source-obligations-journal.json)
retains preliminary failures and their corrections. The final source and audit
are committed with no generated objects, logs or `.my*` outputs.
