# PA25 implementation140

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: 4530fe939e95a45ff6a46d5e53c76807b5bdd352

Target: **PA25 full-stage**. Phase: **implementation**. Entry HEAD:
`4ecb9b413ab864cc1a43b2636ca78e67f6ce8ef8`, **86/101** (15 failures).
The preceding implementation completed class execution and its evidence (progress);
process inspection confirms no prior compiler/check process is live. Preserve [audit138](audit.md) and its review markers.

## Design and completed group

Production remains streaming source -> canonical semantic facts -> typed LowIR
-> function-local MIR -> native object -> indexed linker -> ELF.

| Owner | Data flow and bounded work |
|---|---|
| Native class runtime | Retained definition/fixup edges demand typed runtime roles. One finite support program per link supplies allocation/deallocation, termination, class RTTI address points and pointer casts. No symbol-spelling inference, source replay, external runtime or textual phase transport. Existing object version 2 and definition ownership remain intact. |
| Cast semantics | Recorded ABI hierarchy, offset and access entries feed a typed LowIR walker. Distinct subobject addresses determine ambiguity; public reachability is tracked separately. Virtual paths coalesce by address. Compiler construction has constant size; runtime traverses inheritance paths, with stack proportional to depth and 80 bytes of per-cast state. The optional ABI hint is conservatively ignored. |
| Static construction | Declaration-owned constant plans include final vptr identities for complete objects and nested members, active unions and arrays. Base subobjects do not overwrite derived vptrs. Named evaluator destinations preserve self-pointer relocations without declaring mutable objects constexpr. Field ordering is O(fields log fields); emission is linear in data bytes. |
| Native ABI | Mapping extents live in an aligned allocation prefix and are released by munmap. Syscall MIR records actual argument registers; leaf routines use caller-saved registers. Lowering retains legacy O0 presentation only for the explicit LowIR view; production void-pointer casts avoid an unreachable helper, and pointer-cast calls carry a non-unwinding boundary. |

The initial runtime scope expanded to fix static polymorphic initialization,
private/ambiguous cast interactions, construction-time casts, mutable self
pointers and demand from discarded weak definitions. Required class cases are
now all passing. No fixture, reference, comparison rule or coverage changed.

## Validation and performance

- `make test-pa25`: **86/101**, the original 27 failures reduced to **15**.
- Through PA24: **4152/4152**, all 24 stages pass.
- Explicit controls: **67 class**, **61 driver**, **19 scalar** plus 96 wide
  operand pairs, **122 audit138**, **49 statement**, typed runtime IR validation,
  MIR inspection and object roundtrip; 11 source-to-ELF trace commands plus four
  ABI/data-flow assertions.
- File audit passes with four inherited header warnings; diff whitespace clean.
- [Validation139](../student.tests/pa25/validation139.json) pins final binaries,
  source/fixture inventories, exact failure sets, logs and traces.
- [Performance139](../student.tests/pa25/performance139.md) preserves 504 new
  A/A and ABBA observations. Final template compile median **0.25293 s**, B/A
  **0.998 [0.963, 1.016]**, peak **15692 KiB**. Equivalent executable pairs are
  byte-identical. New class/cast/allocation baselines include runtime and text
  size; their final/final comparisons calibrate noise and claim no speedup.
  Required O0 work is bounded; no optional optimization is added. The inherited
  15% timing/RSS and zero optional text-growth targets are
  diagnostics under spec section 9, not additional exit gates. Correctness,
  coverage, complexity and mandatory native bounds remain required.

## Remaining implementation and independent review

**Unfinished implementation:** 14 source-EH/handler/function-try cases and the
one exact floating-calculator comparison; [remaining139](../student.tests/pa25/remaining139.json)
names all 15. Source exceptions need coordinated
handler matching, payload ownership, nested catch/rethrow state, lifetime cleanup
and function-try semantics. The same owner must supply throwing bad-cast,
bad-typeid and allocation-failure services; the explicit
[class exception boundary](../student.tests/pa25/class-exception-boundary.cc)
remains unresolved. Preserve the unused-dependent-local reducer as separate
frontend completion work. The floating oracle and million-input comparison are
unchanged; [rounding136](../student.tests/pa25/rounding136.md) records why its
excess precision is not proven erroneous.

**Independent review:** assess the accumulated implementation against the spec,
including RTTI access/ambiguity, static destination identity, native support
demand and performance evidence. No finding is waived and no new independent
architecture investigation is a prerequisite invented by this implementation.

## Handoff ledger

| Boundary | Evidence / next owner |
|---|---|
| audit138 | Reviewed through the preserved marker; 74/101; see audit.md and validation138.json. |
| implementation139 | Nonthrowing class execution and constant polymorphic initialization complete; 74/101 -> 86/101, 12 original failures fixed, no new failures. Required prior/file checks pass; final performance, trace and validation records linked above. |

The remaining source-EH group needs a new coordinated handler-frame/payload
protocol: PA24's 80-byte records always pop on transfer, while source cleanup
pads retain their region; matching selectors and nested caught payloads are
absent. Ten required cases stop at native clause selection, and four function-try
cases stop earlier in semantic statement handling. Extending the completed
nonthrowing group cannot supply these facts through additional runtime symbols;
it requires coordinated semantic, lowering, native-frame and runtime work.
That state-machine implementation and the separate floating-evaluation policy
are the concrete next boundaries, preserved as implementation work. This handoff does not certify
PA25: stage and through-PA25 reports must both pass before advancement.

## Active implementation140

Finish the coordinated source-exception group: semantic handler/function-try
facts -> typed LowIR clauses and cleanup edges -> native handler records ->
payload ownership, matching and catch/rethrow services. Native selection must
consume indexed clause facts once, retain cleanup registrations until eh.end,
and preserve parameter/this values on exceptional edges. Runtime storage owns
each thrown payload and nested catch state until final destruction. Compiler
work follows clauses, frames and emitted IR; runtime matching follows only
required type/base edges. Validate all 14 required EH failures together, explicit
nested/rethrow/lifetime/cross-TU controls, typed IR/MIR traces, prior-through
and file audit. Extend through the class failure services sharing this protocol.
The floating oracle remains a separate unfinished semantic-policy group.
Frozen entry binary: artifacts/pa25-140/compiler-A; compare correct common
workloads with A/A + ABBA and record new EH runtime/size baselines. No optional
optimization or unsupported inherited performance gate is introduced.
