# Implementation 216 evidence

Entry: `335618379eb20ac285cc1df5fb498f498afccccf`. Implementations:
`94eb1dae`, `2691df5a`, `2bace8a0`. This closes the contextual-call, exact-scalar,
private-storage and EH-join group. It is an incomplete implementation handoff,
not an independent audit or stage certification. Both review markers remain
unchanged. No course fixture, reference, comparison rule or bundle was changed.

The root course census moves **208/219 -> 213/219**. The five resolved fixtures
are folded exception callees, O1/O2 builtin infinity, landing cleanup inlining
and initializer-list backing. Six course failures remain: one independent
pointer-congruence question and five source ABI/lifecycle/identity requirements.
Three source-debug comparison failures remain required work. See the compact
[plan](../../../pa32/plan.md) for the exact boundary and ledger.

`binding.json` binds implementation/scripts, spec/contract/fixture identities,
all evidence, frozen binaries and retained artifacts. `checks.json` preserves
every command, full log path, digest and exit status, including expected current
stage/debug failures. Run `python3 student.tests/pa32/context_verify.py`.

## Ownership, proof and limits

- `floating_fold` consumes typed scalar operands and the reader's per-format
  literal table. It preserves the destination representation and signed zero.
  Only exact integer/float and float/float conversions are folded; unknown
  rounding, NaN payloads/signaling and denormal comparison behavior retain
  executable operations. Arithmetic is not speculated. Scalar dirty use lists
  propagate facts within existing work limits, with no IR growth.
- `constant_objects` owns readonly-prefix summaries for immutable fixed
  definitions. Imported, weak or object-named globals decline. It accounts for
  implicit alignment padding, explicit packed layouts and zero initialization.
  Relocations/unsupported representations stop the proof. It scans data items
  once per invocation, at most 16 bytes per integer item, never expands zero
  runs or copies complete object buffers. Calls require the recorded Strlen
  identity, declaration, compatible typed signature, effects, unwind, return and
  runtime-role metadata. The text adapter decodes the established runtime name;
  optimization uses its typed identity, not string recognition.
- `inline_policy` admits small parents using already-admitted leaf summaries in
  call-graph order. `Expander` owns immutable input bodies and a site-local
  constant map. Whole-body definition counts exclude mutable carriers. Typed
  parameter copies normalize literal arguments. Ordinary reachability must
  prove every retained call nonthrowing before an EH-bearing candidate is
  admitted; unknown paths keep calls. The cloner retains original debug/ABI
  facts; scheduled scalar/CFG cleanup exposes dead regions before region
  retirement. Handler-stack inconsistencies conservatively decline expansion.
- A proof spends at most **4096/site**, from the existing **32768/caller** and
  **32*(I+O+P+S+F+1)/unit** allowances. Context work is observable in telemetry.
  Depth is <=64; clone growth remains <=1536/caller, 2048 for a single-use
  candidate. Mandatory expansion retains its separate published caps. No
  fixed-point whole-program reinlining is introduced.
- `merge_forward_blocks` joins a forward jump only with its unique ordinary
  successor, preserving handler roots and the EH instruction sequence. It
  checks source definition order and repairs phi predecessor labels. Work and
  storage are O(I+O+B), with no IR growth.
- The private-object owner reuses its exact address census to retire final
  integer/pointer writes only when every use stays within an unread, unescaped
  <=64-byte home. Unknown offsets, escaping addresses, volatile/atomic or
  observable floating stores retain storage. The final census is linear and
  does not restart splitting/inlining.
- The source nested-catch correction pops the parent's registration before the
  catch-all miss edge joins its source catch entry. It preserves the original
  dispatch skeleton and balances the same state as a real parent landing.
- Local-slot cleanup and native parameter-slot selection now retain floating
  stores that perform format conversion, and the conservative x87 store path.
  A dead memory result does not remove rounding or exception flags. MIR shows
  the actual `fptrunc.f80.f32` consumed by encoding. These necessary correctness
  costs also apply at O0; the former incorrect erasure is not a valid runtime
  performance baseline for that reducer.

All new data has site/function/invocation ownership. The production path stays
on typed LowIR and direct MIR/ELF. Existing source/template/ABI/debug traces,
normal and debug object replay, and native execution tests exercise that path.
No frontend side channel, serialized production transport, global retry,
unbounded growth, reference delegation or host compilation was added.

## Checks and observations

The final check set contains earlier **5178/5178**, current **213/219**, through
**5391/5397**, passing file audit (four inherited header warnings), direct debug
**5/5**, and normal/debug object replay **25/25** each. It explicitly runs all
inherited personal semantic reducers, loops/memory/range bounds, source/template
traces and audit214 reducers, plus the new contextual and floating tests.

New executable coverage includes 31 ordinary floating cases at four levels
through native/direct/replayed execution; binary16/binary128 object paths;
dynamic rounding and signaling/inexact exception flags; constant/unknown and
mutable site arguments; readonly layout and builtin-ABI guards; real nested
source unwinding at all levels; identical debug objects; forward-join phi repair;
and conservative private/escaped/volatile/out-of-bounds/floating stores.

`bounds.json` records 100/500/1000 sites with
44,161/216,161/431,161 total optimizer operations and
2,000/10,000/20,000 site-proof operations. A 1200-call caller exhausts exactly
32,768 shared operations, leaves residual calls and passes structural/object
validation. This is measured scaling, in addition to the static reservation
and linear-census proofs.

Performance uses frozen baseline/final binaries, CPU 2, A/A calibration and six
ABBA blocks. Compilation and executable execution are measured separately with
wall time and peak RSS. The scripts compile every workload and runtime driver
with our compiler; the host compiler command only links objects. Runtime inputs
and checked sums/catch counts keep the work live. Every observation, spread,
scheduling/cold-start outlier, flag, input digest and output digest is retained.

The final acceptance set is `affected.json`, `common-o0.json`, `common-o1.json`,
`common-o3.json` and `selfhost.json`, **924 observations**. The corresponding
`initial-*` files preserve the **924** earlier observations before the readonly
layout correction. They are historical evidence, not final compiler evidence.
No observation is silently discarded. Final observations are remeasured rather
than inferred from intermediate binaries.

Affected checks cover checked access, floating conditions, initializer-list
parent access and real landing cleanup. Common fixed workloads cover a
2400-specialization frontend, loops, calls, memory, floating work, exceptions
and pruning at O0/O1/O3; the compiler component is an O0 object with no runnable
entry. The compact plan reports final paired medians and text/RSS values.

Acceptance is stage-scoped: repeatable checked-access/initializer-list benefits
pay for bounded cloning and extra cleanup; floating simplification reduces
compiler work and native text; required landing cleanup improves text with no
runtime-speed claim. Common runtime differences stay within measured noise.
Compiler work and memory costs are disclosed, including the checked-access
ratio above inherited diagnostic targets. Those ratio targets are diagnostics
under spec.md; mandated fixture bounds, work/growth limits, correctness and
coverage remain intact. Later allocation/self-hosting goals add no PA32 gate.

Independent audit still owns accumulated review since `40151904`, including
these proofs and acceptance decisions and the preceding range/helper work.
Remaining source and pointer requirements remain implementation work, not
questions this evidence waives.
