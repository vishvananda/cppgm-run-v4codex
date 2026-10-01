# PA29 compact plan — implementation176 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Audit entry: `914e1a0a07e40884c91c0b967b0421eea9ef0d48`.
Last reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.
Implementation176 entry: `2b7a513297b2e9719fcd5686d3c1c97ee7565b13`.
Implementation code: `207743ae`, `61756cf8`, `1b19e9ac`.

## Design/spec alignment and completed group

[Implementation176](implementation176.md) completes hosted declaration identity
and emission: explicit-instantiation exclusion for functions/static data/nested
classes, direct-member overrides, late definition attributes, and inline-variable
identity across TUs. Related work extends through static members, variable
templates, scalar TLS, dynamic initialization, reference-temporary identity and
ordered destruction, including mixed host/student objects.

Syntax attributes flow into canonical declaration facts and indexed pattern
edges. Demand consumes those facts; typed LowIR/ABI records drive weak ELF data,
guards and support objects. Class completion retains inline member initializer
recipes. Value/storage/type demand computes each required initializer once,
including deferred array-bound deduction. Constant-only queries emit no storage.
No grammar replay, semantic cloning, text transport, global retry or new optimizer
pass was added. Work follows source declarations, lexical/pattern edges, demanded
facts and emitted operations; all records retain TU/function release boundaries.

## Validation and performance

PA29 **380/403**: **25 → 23 failures**, two existing course fixtures fixed, no new
failures. PA1–28 **4538/4538**; through PA29 **4918/4941**. All **403** course inputs
and **1,707** contract/harness paths are unchanged. **25** focused controls and
**48** inspection commands plus symbol/demand/telemetry assertions pass. File
audit passes with the same four inherited header warnings.
[Evidence](../student.tests/pa29/evidence176/validation.json) records commands,
statuses, hashes and the exact delta; [coverage](../student.tests/pa29/evidence176/coverage.json)
checks every tracked contract path against entry.

[Performance176](performance176.md) applies spec §9 to **PA29/O0**: frozen A/A and
six ABBA blocks measure compiler latency/RSS and checked runtime/text on four
inherited workloads (224 samples). Executable text is unchanged; paired median
compiler ratios are 0.9948–1.0154, runtime ratios 0.9903–1.0117, and peak compiler
RSS grows at most 1.64%. Affected 600/1200/2400-specialization inputs retain 48
samples with checked runtime and linear demand/IR/text counters. The 2400-function
O0 runtime slowdown is disclosed. Entry rejects the affected semantics, so no
invalid affected speedup ratio is claimed. Optional transform work/growth budgets
remain zero. Historical blanket 15%/zero-growth targets remain diagnostic;
mandated capacities/timeouts, correctness and coverage are preserved.

## Remaining implementation and independent review

The [remaining ledger](../student.tests/pa29/evidence176/remaining.json) retains
extended syntax/types/layout **17**, template demand/hosted ABI **5**, and legacy
trait **1**. Numeric/complex representations, decomposition and control-flow
extensions, deduction guides, zero-length arrays, static receivers, char-traits
conversion shims and required force-inlining remain unfinished implementation.
Through-PA29 success is required before PA30.

[Audit174](audit.md) remains the last independent review. [Audit170](audit170.md)
retains the char-traits/alignment/dependent-offset/convertible-index reducers and
the unresolved nothrow-default-construction and nothrow-invocable contract
questions. Implementation176 adds a reduced nested-member ABI-tag question:
GCC and this compiler retain the tag, Clang drops it, and the unchanged fixture
expects suppression. C++11 does not settle this extension encoding. All three
contract questions remain counted failures, without correction or waiver.
The new implementation also requires independent review; this handoff does not
certify whole-stage architecture, performance or correctness.

## Handoff ledger and boundary

The preceding goal turn was progress: committed and validated implementation175.
No live process required resuming at entry. This turn completed declaration
exclusion and inline storage, then extended the same ownership through lazy
initializers, omitted array bounds, shared lifetime-extended temporaries and host
interop after focused controls exposed those related defects.

No known required PA29 defect remains in this completed group. The remaining
implementation needs new syntax/numeric representations, overload/trait rules or
an actual force-inline transform; it cannot be repaired by further extending
these declaration/storage facts. The nested ABI-tag discrepancy needs independent
contract resolution, not a source-specific naming exception. General nontrivial
hosted TLS lifetime work remains later hosted-runtime scope. These ownership and
contract boundaries end this implementation handoff. Stage base and Last reviewed
markers are preserved; independent review and whole-stage completion remain open.
