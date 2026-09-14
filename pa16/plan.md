# PA16 compact plan — implementation 42

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `7c39a6edbfa43c226036b8a92fe236722ac85dcc`

Target: **PA16 full-stage**. Entry HEAD `fe4dbd88712f3b87c478084e1b48887315c13e10`,
**146/154** passing. Previous goal turn: progress (object/address implementation
and native/performance evidence). Both review markers remain unchanged.

## Design/spec alignment

Owner/data flow: declaration checking publishes an initializer plan; the TU
semantic constant checker classifies its persistent value and volatility once;
lowering consumes that plan as typed readonly data with size/alignment, scalar
payloads, symbol identities and addends. One distinct automatic slot receives
one copy. The existing interner shares only data images, never object identity.
Ordinary automatic scalar arrays now use this path whenever their complete
initializer is known. Nonconstant, volatile and class-lifetime paths keep their
selected execution. Constant local references use static relocation data; the
special startup binding queue has been removed. Program-owned typed function-ID
sequences now combine multiple TU startup/shutdown bodies into one hook per
role. Startup follows input order and shutdown reverses it; one-TU output
retains its existing form. Coordinator work/storage is O(TU hooks), with at
most two coordinator functions and one call per participating TU. Per-TU helpers
use non-legacy names so serialized LowIR cannot reacquire a singleton role.

Complexity: classification is O(explicit initializer actions), memoized by plan
identity. Omitted array ranges stay sparse. Hash/equality work follows emitted
data items; images and indexes release with the lowering TU. No new source
replay, semantic string keys, broad invalidation or optional optimizer. Existing
512-call / 1,000,000-step constexpr limits remain. Typed object/address execution,
constructor/conversion selection and indexed subobject paths from implementation
41 remain unchanged; see [object evidence](object-performance.md).

## Remaining implementation groups

| Owner | Unfinished requirement / next evidence |
| --- | --- |
| Class-result ABI and full-expression emission | One course failure: `400-constexpr-dependent-nonliteral-result-instantiation`. The compiler uses a caller-owned indirect result for a nontrivial empty class; the oracle uses a direct result. The compiler also retains extra EH regions when activating returned temporaries. The destructor already has `unwind=no`; this is not missing exception metadata. Resolve the boundary and cleanup policy together across PA12/16; do not replace unrelated correct oracle instructions. |
| Constant evaluator member-pointer values | A newly recorded [reducer](../student.tests/pa16/initialization/member_pointer_pending.cpp) is valid C++11 but rejected as a nonconstant constexpr array initializer. The scalar storage path cannot classify this until the evaluator represents member-pointer constants and their conversions/projections. This is unfinished implementation, not an excluded test or an audit waiver. |

The seven original static-initialization mismatches are resolved. The eight PA16
and sixteen PA10–15 oracle revisions have [reduced standard/contract proofs](reference-corrections.md).
Original source inputs, status sidecars, coverage and complete comparison rules
are preserved. The automatic-array change resolves the previously documented
store/copy conflict; earlier historical plans no longer describe it as blocked.

## Performance acceptance

[Initializer evidence](initialization-performance.md) retains **364** initial frozen A/A
and ABBA observations for the storage increment. Final lifecycle-inclusive
measurement and required-check refresh are in progress. Common LowIR/executables are identical;
large common compiler timings remain near entry. Sharing arrays saves 10,196 KiB
at 4,000 functions. Affected runtime pairs improve for strings, 2- and 40-element
arrays; 8-element arrays are near noise. Compiler text is unchanged and all
code/data growth is disclosed. No avoidable common regression is evidenced.
[Storage](storage-performance.md), [objects](object-performance.md) and
[audit](audit-performance.md) retain all earlier observations. PA16/O0 has no
mandated numeric latency/RSS/text ceiling. Historical percentage/RSS/scaling
targets remain diagnostic under the stage-scoped spec, not inherited exit gates.
Correctness, work/resource limits and coverage remain mandatory. No optional
optimizer is introduced; runtime differences apply to the measured workloads.

## Handoff ledger

| Checkpoint | Result |
| --- | --- |
| Implementation 41, through `f1497ca2` | Typed objects, addresses, constructor/conversion execution, pointer/literal identity; 146/154 course tests; 173 native / 75 rejection controls. Historical evidence remains in `object-performance.md` and `audit.md`. |
| `9e967c33` initialization increment | Ordinary automatic scalar-array data/copies; static relocation for local references; 24 narrowly edited oracles with proofs and before/after hashes. Explicit controls: 32 native, 2 rejection, seven observed reference startup failures. |
| Program lifecycle increment | Found and fixed duplicate singleton roles when multiple TUs need initialization/finalization. Both source orders pass typed validation, LowIR roundtrip and native constructor/destructor LIFO checks. |
| Through report after corrections | **2265/2266**, with all **2112/2112** prior tests and **153/154** PA16 tests passing. Required `make test-pa16` exits 2 with that sole comparison; `make test-report-through-pa15` exits 0; file audit exits 0 with three inherited header warnings. **205 native / 77 rejection** personal controls pass; the separate pending member-pointer reducer still fails and is not included in those passing counts. |

Implementation handoff boundary: persistent initializer classification and
storage emission are coherent for the represented constant-value families.
Further storage rewrites cannot resolve either the separate result-lifetime ABI
contract or the missing member-pointer value algebra. Both remain required
implementation, with concrete reproducers/comparisons above. This is an
incomplete implementation handoff, not assignment certification or advancement.
The [checkpoint](../student.tests/pa16/initialization-checkpoint.json) and
[verifier](../student.tests/pa16/verify_initialization.py) bind code, coverage,
revisions, raw measurements, required checks and pending evidence.
Independent review is still due for the accumulated changes since the preserved
review marker, including constant cache/lifetime completeness, query adapters,
ABI policy and these reference proofs. Review does not replace unfinished work.
