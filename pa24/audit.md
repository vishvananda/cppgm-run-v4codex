# PA24 checkpoint audit130

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: 9f3f9b5caaec9677af0e8431b51cacebcf823dce

This first audit covers `bde9eb3e..5bbf5325` in full and the validated audit fix
`9f3f9b5c`. The entry was clean. The preceding goal turn was verified progress:
its commits and saved test output established a new wide-value implementation
boundary; no build/test process was live at this audit's entry. This checkpoint
review is complete; PA24 remains incomplete under the preserved failure floor.

## Range and independent review

Every commit and the combined 58-file entry delta were inspected, not merely the
latest handoff. The native implementation files were read through their selection,
ABI, encoding and dump consumers, together with all shared LowIR changes,
source-set registration, personal generators and performance workloads.
`review-inventory.json` records the complete commit/file inventory.

- `f9ff5dd4`: scalar selection, encoding, layout, ELF, CLI and shared validation.
- `7d1be282`: parameter promotion, placement, phi transfers, fixed bulk selection,
  builtin ownership and initial independent/performance harnesses.
- `9d8113eb`: phi-edge deduplication, symbol-address retention, narrow parameter
  normalization and literal/debug/native evidence; this evidence commit also
  changed production code and was reviewed accordingly.
- `2425715e`: floating payloads, SSE/x87 operations and conversions, XMM/call
  placement, mixed ABI and variadic register-save/overflow state.
- `ae72c874`: bounded carried reloads, incoming-parameter CFG flow, XMM reuse and
  floating truth/branch semantics.
- `c88bfaeb`: telemetry work counters and measured floating/scalar handoff.
- `7e9d4569`: complete integer payloads, object/wide fragments, shared ABI cursor,
  padded homes, partial tails, register rollback and large-copy dependencies.
- `7def93ba`: wide division/conversion, implicit width consumption, atomic and
  object effects, variadic boundaries and completed numerical controls.
- `5bbf5325`: accumulated plan and frozen wide-value validation/performance.

No fixture, reference, harness, comparison rule or reference-bundle manifest
changed in this range or during the audit. Bundle source remains
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; no reference correction exception
was needed. The inventory's combined hash covers all 2403 tracked PA24 test files.

## Findings and fixes

[The independent runner](../student.tests/pa24/audit130.py) retains reduced inputs
and optional MIR/executable artifacts. Its **21** controls produce **11** failures
on frozen entry and **zero** on reviewed code. These are general LowIR computations
with independent numeric/memory checks, not expected fixture answers.

| Defect | Owning repair and evidence |
|---|---|
| Implicit narrow RHS widening overwrote an already loaded wide LHS in r10 | `wide.cpp` prepares both typed operands before fixed carriers. Add/sub/and/xor reducers failed at entry; all arithmetic controls now pass. The OR control independently remains correct. |
| A scalar slot consumed as i128 was read as a two-word object | `selection.cpp::value` applies the same typed widening/truncation rules to slots and temporaries. The reduced i8 slot plus i128 addition now returns four without reading adjacent bytes. |
| Reusing a dead incoming rcx/rdx for a longer-lived result ignored future fixed effects | `placement.cpp` checks the new complete interval against the existing clobber fact, and uses the shared `clobbers` owner instead of a second effect inventory. Division and variable-shift reducers preserve their live result. |
| A folded indexed store destroyed its index while loading or widening its value | `move` and Store selection choose a nonoverlapping value scratch; implicit wide stores first capture the complete address. Same-width, narrow-to-wide and i128 reducers now write the selected element. FP-converting stores and composed indexed-load comparisons remain valid controls. |
| Reload carrying admitted byte multiplication whose encoder loads through r11 | `carry.cpp` retains the frame form when that hidden effect lacks a proof. The high-pressure reducer previously corrupted a live carried scalar; it now passes. Bounds and the required positive carry control remain intact. |

The additional controls check argument address dependencies, sign-preserving
floating negation, x87 rounding-word restoration and full-width integer plus
near-halfway decimal payload roundtrips. Existing independent suites cover wide
carry/borrow/multiply/restoring division, signed/unsigned predicates, shifts,
NaN comparisons, FP conversions, variadic overflow, object tails/rollback, atomics,
parallel phi cycles and retained parameter homes. All continue to pass.

The fixes are necessary correctness work. They introduce no new optional
transform, blanket spill policy, repeated analysis or global invalidation.
Unknown byte-multiply effects conservatively preserve valid frame traffic.

## Architecture trace and ownership

The new [source trace](../student.tests/pa24/trace130.cpp) executes correctly with
argc 1 and 2. `Pair::left/right` accesses and `Accumulator<6>::apply` reach this
compiler's own native ELF. The invalid dependent `dormant` member is never emitted.
`trace.lowir`, `trace.mir`, `trace-frontend.stats`, `trace-elf.txt` and
`trace-disassembly.txt` retain the inspected path.

1. `lowering/driver.cpp` connects immutable preprocessor buffers and interned
   identifiers through streaming post/syntax cursors to
   `Parser::translation_unit(&sem)` and `Analyzer::consume`. Parsed source nodes
   and attached semantic facts share identity; there is no second syntax tree.
2. Entity/Type/Scope IDs and flat indexes own lookup and selected call/conversion
   facts. `template_type_facts.cpp` interns complete parent-linked substitution
   frames. `substitute_type` immediately reuses nondependent types;
   `template_expression.cpp` inherits fixed expressions. Occurrences retain the
   parsed pattern rather than replaying grammar or cloning complete bodies.
   `Analyzer::finish` advances demand queues/cursors with active/success/failure
   state; unused members are not instantiated by class completion.
3. Typed layout records place the Pair fields at 0/8 and Accumulator's field at
   0. Lowering consumes selected `sum`/`apply` calls, the member receiver and
   explicit integer conversions. `FunctionBuilder` creates typed LowIR directly;
   `apply` is emitted once with its canonical specialization ABI identity and
   constant six. Manglings are exported spellings, not semantic lookup keys.
4. Today's source tool explicitly writes LowIR and PA24 explicitly reads it;
   this trace tests that documented adapter boundary. Inside production lowering
   and the native backend there is no serialize/reparse transport. The later
   integrated source/native driver is not claimed complete. External LowIR gets
   full validation once; unchanged typed native phases do not revalidate it.
5. Native `Workspace` has unit-sized dense identity/index tables. `Selector`
   owns local value states, intervals, frame homes, move schedules and flat MIR.
   An ABI cursor shared by caller/callee handles whole-object register rollback,
   stack offsets, hidden result pointers and padded fragments. Incoming-home CFG
   flow uses six monotonic bits and deduplicated enqueues, not global retries.
6. The MIR view and encoder consume the same selected operands, call facts,
   frame/preserve policy and debug locations. Reserved rax/r10/r11 and xmm14/15
   scratch are excluded from ordinary placement as appropriate. Fixed effects
   are now shared by parameter and reuse decisions. SSE and x87 conversions,
   two-word operations and cmpxchg16b emit native instructions directly.
7. Function selector state and MIR die after encoding. Encoder label storage is
   reused at the unit/high-water lifetime, alongside code/data, compact symbol
   offsets and fixups; it is not cleared/reallocated per function. Typed fixups
   patch only after final layout. ELF fields are written explicitly in little
   endian, with separate RX/RW segments. No assembler, host or reference compiler
   implements output. Runtime builtin identity comes from explicit ABI metadata.

The trace emits exactly three functions and 190 native code bytes. Debug/MIR
inspection and the integration check establish that requesting a dump does not
change executable bytes. Raw reducer disassembly confirms that the indexed
address is saved before widening and that the retained division operand has
moved out of rdx. Shared LowIR format tests include **21** idempotent canonical
roundtrips followed by native execution, including values beyond 64 bits and
f32/f64 literals that would be wrong after double rounding.

## Optimization legality, profit and budgets

A useful retained fact is the complete interval of a scalar result across one
adjacent single-predecessor/single-successor edge. `analyze_instruction` and
`folds` extend intervals through aliases and deferred address consumers;
`single_edge` permits retention only when the CFG and effects prove it safe.
`allocate` selects a surviving carrier, now including the new interval's fixed
clobbers. Final encoding omits the frame traffic while preserving the same value.
Joins, backedges, unknown clobbers and pressure retain homes. Historical handoff127
ABBA observations show the forward-edge/call benefit and 176 -> 161 code bytes;
reviewed output remains the same 161-byte program.

| Policy | Legality/fallback and scope | Composed work/growth budget |
|---|---|---|
| Parameter promotion, copies and folded addresses/loads | Same representation and complete interval; no volatile/escaped slot; memory folding only adjacent safe consumers | Fixed linear walks; no fixed point, body duplication or semantic recovery; alias/carrier uses propagate before placement |
| Register/XMM reuse | Dead original value, complete new interval, ABI/fixed-effect survival; otherwise another register or home | Nine GPR/fourteen XMM choices; one function's compact state; no search beyond the fixed pool |
| Reload carrying | Private one-store 64-bit home, complete same-block window, no forbidden explicit/implicit effects or overlapping carrier | Three probes of at most 64 instructions, bounded rewrite; unknown effects keep memory; no code growth |
| ABI/phi transfers | Shared argument classification, padded object homes, captured large-copy dependencies; parallel edge semantics | At most fourteen register moves plus linear stack arguments; phi moves sorted O(E log E), one transfer block per distinct edge |
| Bulk/immediate/branch encoding | Exact byte accesses, reserved scratch, integer-width wrapping, direct compare flags; conservative fallback | Constant-size local choices, fixed copy thresholds, bounded zero-form byte comparison, linear fixup patching |
| Wide arithmetic/FP/atomic primitives | Preserve both words, signedness, guard/sticky rounding, control word and atomic expected-value updates | Constant emitted code per instruction; 128 target division iterations, four conversion digits; atomic retry loops are target semantics, not compiler fixed points |

These limits compose over consumed/generated IR and CFG edges. There is no O1–O3
optimizer at this stage; those flags retain O0 policy. Existing telemetry counts
work already performed, with per-function phase timers and process peak RSS;
reporting does not trigger extra semantic analyses. MIR output is separable and
its byte-identity control passes.

[Performance130](../student.tests/pa24/performance130.md) records all four dimensions
using frozen correct A/B binaries, fixed inputs/flags, A/A calibration and six
ABBA blocks. Compiler paired B/A medians are **0.952 / 0.964 / 0.982** for
scalar/floating/wide, with unchanged or lower RSS. All six runtime images are
byte-identical: **161, 149, 1522, 557, 308, 2211** code bytes. No new speedup is
claimed; old floating and forward-edge improvements remain measurable historical
evidence, with all nine inherited observation-manifest hashes reverified.

The inherited 15% compiler latency/RSS and zero optional text-growth targets are
diagnostic, not additional exit criteria. They are met here; old noisy misses
cannot permanently fail corrected code. All mandated behavior, comparison rules,
MIR bounds and finite work budgets remain binding. Required wide semantic costs
and later driver/self-hosting work do not create extra checkpoint gates.

## Validation, remaining work and ledger

[Validation130](../student.tests/pa24/validation130.json) binds reviewed source hashes,
commands, terminal statuses and evidence hashes. Required checks were finally run
**sequentially**: the initial overlapping root reports shared `.test_counts`, so
their mixed totals are retained but excluded as acceptance evidence.

- `make test-pa24`: **282/296**, exit 2; the exact same **14** fixtures fail as at
  entry, with no new failure compensated by another pass. **17/17** focused
  properties pass. All **287** successfully compiled positive programs match
  runtime output/status, including six canonical-MIR failures.
- Required prior-through command: exit 0, **3856/3856** fixtures, **23/23** stages.
- Required file audit: exit 0, four inherited substantial-header warnings, zero
  fatal findings. No new implementation translation unit needed registration.
- Personal suites: **1820 scalar**, **1259 floating**, **1495 wide**, **160 object**;
  ABI/debug/ELF integration and parameter flow pass. Final audit controls are
  **21/21** and roundtrips **21/21**; the final payload control was added after
  the aggregate personal run and has its own successful final run.
- The reported 435-file inventory consists of 296 course fixtures, 125 excluded
  solution-regression fixtures and 14 control input files. Controls report 17
  properties separately. No coverage or comparator was reduced.

Remaining implementation is grouped in [the compact plan](plan.md): **runtime and
layout completion** (three TLS failures, five EH failures, required runtime and
dynamic/overaligned stack paths) and **scalar placement/final frame protocol**
(six mandatory canonical-MIR mismatches). These remain actual PA24 requirements,
not waivers, diagnostic targets or reasons to advance to PA25.

The three behavior handoffs were coherent, but repeated scalar/float/wide fixes
to scratch effects and parameter ownership caused avoidable fragmentation. Future
handoffs should close the shared owner and mixed-width/pressure/CFG consumers
together before recording a validated boundary.

| Audit | Reviewed accumulated range | Evidence and disposition | Remaining groups |
|---|---|---|---|
| 130 | `bde9eb3e..5bbf5325`, plus fix `9f3f9b5c` | Full code/interaction review; 11 reduced failures fixed; prior/file/progress gates pass; performance preserved; Last reviewed commit set to code tip | Runtime/layout; scalar placement/frame protocol |

Evidence directory: `/home/vishvananda/work/private/v4codex/artifacts/pa24-audit130/`.
The successor commit contains only plan/audit/validation/performance records;
there are no code edits after the reviewed tip.
