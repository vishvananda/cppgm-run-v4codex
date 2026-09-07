# PA4 final plan and audit ledger

Stage base commit: `a682ffe75533c8aed941f46f6131c9e8af22f93d`
Last reviewed commit: `54f4824ac0d3f5fabcdeb92a44a76125f643182a`
Independent audit entry: `60ef1b9df`; target: **PA4 full-stage**; phase: **complete**.
Independent architecture review, fixes, benchmarks and all exit checks pass.
No PA5 work is started.

## Spec Alignment

| Owner | Final design and evidence |
| --- | --- |
| Source/directives (§1–2, §8) | Immutable TU sources, compact identities, streaming file/directive boundaries, per-file conditionals, live PA3 evaluation, presumed locations and flat device/inode once state. Primary sources reset the TU. |
| Macro cursor (§1–2, §5, §8–9) | Dense definitions/prebound parameters → indexed raw argument slices → once-per-used-argument prescan → iterative rescan with token-local ancestry/paint. Stable task slabs and reusable depth-local buffers eliminate allocation churn while preserving course recursion. |
| Post-token handoff (§1, §10) | Structured shared PA2 cursor decodes literals directly and keeps one lookahead. CLI rendering is only a view. Direct API trace checks a macro-generated template declaration, pasted canonical name, condition selection, literal value and physical/presumed locations. |
| Later surfaces (§3–7, §9) | Semantic template demand, typed semantic graph/LowIR/MIR, ELF, optimization levels, generated runtime/text and actual self-hosting have no PA4 surface. No proxy is claimed to implement or benchmark them. |

[Independent audit](audit.md) records ownership/release, complete representative
traces, legality, invalidation, complexity, applicable spec clauses and every
handoff since PA3. No PA4 behavior group or related implementation is deferred.

## Findings and changes

- `c558c57d2`: replace per-prescan task construction/destruction with an inline
  root, stable 32-frame slabs and reusable argument/output buffers. Preserve
  slice addresses and clear invocation-local readiness/results after use.
- `54f4824ac`: actual allocation interception found the remaining temporary
  delimiter-index stack; retain it with capture storage. The new assertion
  failed before this fix and passes after it. Add the full declaration trace.
- Add reuse tests for empty/variadic arguments, counters, raw/expanded uses,
  changing definitions/arity and function-name lookahead. Pool capacities are
  storage only, never an expansion cache; release all with the expander.

## Performance and budgets

Budgets were fixed before each campaign (initial stage: `c90cf1e62`; independent
pooling audit: the plan in `c558c57d2`): paired latency <=10% plus measured noise,
RSS <=15% plus 1 MiB, host text growth <=15%, fourfold input <6x time / <5x RSS.
Require affected-workload benefit beyond noise in both blocks. Pipeline work
retains <=3n captured tokens for n nested calls, once-per-used-argument prescan,
O(maximum depth) frames, geometric buffers and <=128 KiB counter spelling.

[Final evidence](../student.tests/pa4/final-audit-performance.md): frozen entry A
`60ef1b9df` vs final B `54f4824ac`, eight fixed inputs, two A/A pairs, B/B,
two ABBA blocks, equivalent outputs, 120 observations plus eight startup probes.
The verifier recomputes every budget and checks the actual final binary hash.
The [intermediate pooling campaign](../student.tests/pa4/performance-pooled.md)
retains another 128 observations; all earlier stage campaigns remain intact.

Nested latency: 0.108902 → 0.094080 s; paired gains 12.35–13.90% vs 3.94% noise;
RSS 4568 → 4804 KiB. Repeated-argument use improves 5.74–5.78%. Disclosed
long-chain regression: 1.42–1.84%, RSS +74 KiB; other inconclusive groups are not
claimed as wins. Host text 171378 → 168528 bytes (-1.66%); 4x source growth
3.9663x latency / 3.0752x RSS; fastest input 22.2x startup. All budgets pass.
Generated-program runtime/text: **N/A**. No generated-code improvement claimed.

Three 20,000-deep API invocations capture 180,000 tokens using 625 slabs; after
two warm-ups, the third invocation makes zero allocation calls. On the fixed
600-deep, 128-repeat benchmark, 76,800 prescans require only 19 slabs, 600
argument-buffer growths and 1,200 output-buffer growths. These prove bounded
work/storage; measured wall time establishes the optimization's benefit.

## Validation and ledger

- Course PA4: 105/105; fresh root through report: **205/205, four stages pass**.
- Required file audit: **43 files pass**. No fixture, reference, harness,
  comparator, coverage or timeout was changed; source registration is complete.
- Personal/API: PA4 168 cases and the declaration/location/identity/depth/
  actual-allocation checks; PA1 64 cases/API; PA2 316 cases/API plus 7,062
  independent integer cases; PA3 72 invocations/15,045 results/API all pass.
- Final ASan/UBSan with leak detection: 168 personal cases, the complete API
  checks (including zero warmed allocations), and all 105 course cases pass.
  Final ordinary through report, file audit, performance/hash verification and
  whitespace/fixture checks pass; intended source and audit changes are committed.
- `babb8e9d2`, `28279a9d0`, `c90cf1e62`, `957b47c37`, `1af70fc0d`, `60ef1b9df`:
  all checkpoint handoffs independently reviewed, including directive ordering,
  slice ownership, parameterless boundaries and suffix locations.
- `c558c57d2`, `54f4824ac`: cohesive full-path allocation fixes committed;
  final audit/evidence consolidation closes the ledger with a clean worktree.
