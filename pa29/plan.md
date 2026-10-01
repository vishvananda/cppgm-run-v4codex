# PA29 compact plan — checkpoint audit162

Target: **PA29 full-stage**. Phase: **checkpointAudit complete; implementation unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Previous review: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Audit entry: `9662716b8a8aa0bef94f5a293d7900b28c42701c`, **338/403**, 65 failures.
Current: **338/403**, the identical 65 failures; all 403 fixtures and sidecars,
references, comparison and discovery rules are unchanged in this reviewed range.

## Reviewed and corrected

[Audit162](audit.md) covers every commit since the previous review, all three
accepted handoffs (traits159, invocation160, atomics161), and their combined
ownership paths. Code fix `cce8634c` closes four findings: atomic bool compound
operations, scalar reference snapshots, recorded alignment at atomic builtin
selection, and atomic identity versus cv-qualification in conversions/casts.
Native atomics require sufficient recorded alignment; unsupported scalar and
generic storage uses the existing libatomic ABI. Queries and lowering retain
selected facts, with no new source parsing, semantic lookup or global retry.

## Validation and performance

- `make test-pa29`: **338/403**, exit 2; zero new failures, none removed.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4876/4941**, exit 2, only PA29 fails.
- File audit: pass, the same four inherited substantial-header warnings.
- Explicit controls: **49/49** new; inherited **34/34, 47/47, 46/46, 45/45**.
  Native/LowIR roundtrips, ABI, cache sharing and telemetry equality pass.
- [Validation](../student.tests/pa29/evidence162/validation.json),
  [coverage](../student.tests/pa29/evidence162/coverage.json) and
  [performance162](performance162.md) retain the evidence. Frozen cumulative
  and audit-only A/A+ABBA comparisons have identical common objects/executables;
  new combined ownership workloads scale with demand. All 1,088 preliminary
  and final performance observations remain. No speedup claim is made.

No new optional optimization is introduced; new optional work/growth budgets
are zero. Historical blanket 15%/zero-growth targets remain diagnostics under
spec §9, with all measurements and mandated limits preserved. PA29 excludes
runtime vector lowering; broader headers/runtime, optimizer/allocation and
self-hosting retain PA30–34 owners. Required PA29 behavior is not waived.

## Broad remaining work

The unchanged [fixture ledger](../student.tests/pa29/evidence161/remaining.json)
and [failure-set check](../student.tests/pa29/evidence162/stage-delta.json) retain
all unfinished behavior. Owner labels are not root-cause proof.

| Owner / failures | Work |
|---|---|
| Assembly: 6 | Constraints, operands, clobbers and effect recipes, including locked updates/fences. |
| Extended syntax/types/layout: 37 | Numeric/complex types, vector width, unused-wrapper validation, designated initialization, folds, lambdas and bindings. |
| Template demand/hosted ABI: 19 | Packs/aliases/context keys, pretty-function, extern/inline emission and naming. |
| Structured intrinsic operands: 2 | Evaluation-mode/address semantics and source-location facts. |
| Legacy trait contract: 1 | Independent proof for the forward-declared std-trait oracle question. |

Also retain code-alignment placement, dependent offsetof ABI signatures and
class-convertible designator indices in their owning groups. Extended-float
suffix recognition still does not provide extended precision. The reserved-name
nothrow-invocable fixture with an explicitly false primary remains a contract
question within the template group. Neither question permits library-name
shortcuts or an unsupported reference correction.

Avoid further small handoffs for individual symptoms. The three broad groups
were useful, but separate trait followups and repeated evidence boundaries
increased review overhead; atomic identity/storage changes needed cross-owner
controls at the initial handoff. Complete each next owner group through its
shared semantic, lowering and validation paths. A full through-PA29 pass remains
required before advancement. The records commit follows the reviewed code tip
without further code edits.
