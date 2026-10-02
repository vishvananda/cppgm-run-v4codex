# PA30 compact implementation plan — implementation200

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`.
Target: **PA30 full-stage**. Phase: **implementation handoff; stage incomplete**.
Implementation entry HEAD: `377d92a00e728199f86381f88f239a88904b829d`.
Code tip: `37b6729d`. Both review markers are preserved.

## Design/spec alignment

[Design200](design200.md) records owners, data flow, complexity and validation.
Selected aggregate subobjects establish contextual destructor access and precise
member demand; root storage retains its own lifetime rules. Nested template
bodies wait for their enclosing complete-class context, with source-pattern
binding distinct from concrete body checking. Integer-sequence queries consume
concrete member-template identity without mistaking provenance for dependence.
Existing canonical facts, indexed queues, per-TU lifetimes, direct typed LowIR
and ELF emission remain the production path. No new implementation sources,
text bridge, global retry or optional optimizer was added.

## Validation and performance

[Reports](../student.tests/pa30/evidence200/validation.json): earlier PAs
**4941/4941**, file audit passes (four inherited warnings), PA30 **148/153**,
through30 **5089/5094**. [Delta](../student.tests/pa30/evidence200/stage-delta.json):
**ten existing failures fixed; 15 → 5**, no regression or coverage reduction.
Ralph's cached entry 138/154 disagrees with the authoritative 138/153 primary
log and inventory; the discrepancy is preserved, as in audit198/implementation199.
All course fixtures, references, sidecars and comparison rules are unchanged.
Explicit controls: **50** current + **88** implementation199 + **107** accumulated
controls; **81** source/LowIR/object trace commands pass.
[Verifier](../student.tests/pa30/verify200.py) binds current sources, binaries,
fixtures, reports and measurements. Previous goal turn is classified as progress:
its committed changes/reports were present; no inherited process was live.

[Performance200](performance200.md): frozen entry/final binaries; A/A and six
ABBA blocks; all observations; compiler latency/RSS and checked runtime/text;
affine semantic work at N=64,256,1024; ten repaired hosted inputs, maximum
**2.509 seconds / 177060 KiB**. Common A/B programs are byte-identical.
The **45-second** limit is mandatory. Unsupported historical blanket percentage
and zero-growth targets remain diagnostics under spec §9, with all prior
measurements preserved. No optional optimization benefit is claimed.

## Remaining implementation groups

| Required failures | Owner and next work |
|---:|---|
| 2 | Packed SIMD: random reaches unsupported `__builtin_ia32_packsswb`. Requires typed signatures, saturation/lane semantics, and subsequent header operations. No stubs or unused-result invention. |
| 1 | Local object-use ownership: reject odr-use of an enclosing automatic local from an ordinary local-class member; preserve constants, unevaluated uses and valid lambda capture paths. |
| 1 | Control flow: reject reachable non-void fallthrough while preserving main, infinite loops, returns, jumps and exception paths. |
| 1 | Allocation exception redeclaration: resolve the hosted replacement-new dynamic-spec mismatch through semantic rules or a documented standard/contract proof for any reference correction. Still an implementation obligation. |

Also unfinished: general vector subscripting (`pending199/` and
`evidence199/pending.json`). No known defect is relabeled as an audit question.
Do not advance until the full root through30 report passes.

## Handoff ledger

| Work | State and evidence |
|---|---|
| Checkpoints195–197 | Reviewed/repaired by [audit198](audit.md); all historical measurements and reference proof remain. |
| Implementation199 | Committed allocation/access, vector and parser repairs; independent delta review remains pending on Ralph's schedule. |
| `6dcff26a`, `e171a838`, `37b6729d` | Completed all six prerequisite and four constructor/call failures. Destruction demands, complete-class queues and concrete alias identity have focused positive/negative/runtime/LowIR validation and final reports. |
| Incomplete handoff boundary | Extended initial prerequisite work through all related constructor/call failures and scalar-new review correction. Remaining SIMD arithmetic, automatic-object odr-use, CFG reachability and allocation exception compatibility require separate semantic proofs/owners; relaxing these completed facts cannot resolve them. See design200. |
| Independent review | Implementation199/200 deltas await Ralph's audit schedule. This is separate from the explicitly unfinished implementation above; neither is waived. |

This handoff returns implementation control; it does not certify the whole stage.
