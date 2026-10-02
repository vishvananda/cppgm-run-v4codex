# PA33 final whole-stage audit 220

Stage base commit: 676b6e328c7a05334b15f1c9ee1ec30c783e9aff
Last reviewed commit: d7b7c9cfedf916a38af813fd9e5144d2cdf058b0

Scope is **PA33 full-stage**, including the shared source/object backend and
every handoff since the PA32 final audit. This review began at clean `c1328ca0`.
There was no live prior build or test to resume. The previous implementation
turn made progress: committed code, tests and bound measurements were present.
The source, contracts and commits were independently read; checkpoint prose
was a navigation aid, not architecture evidence.

## Spec Alignment: reconstructed production flow

`lowering/driver.cpp` constructs a TU-owned preprocessor, `PostTokenCursor`,
syntax cursor, Ast and semantic analyzer. Immutable buffers and interned token
IDs feed bounded ring lookahead; the trace consumes 396 tokens with a maximum
48 pending. Parser and analyzer cooperate on the shared graph. There is no
second owning semantic tree or complete intermediate token stream.

Scope indexes in `semantic/lookup.cpp` use compact scope/name/kind identities.
Template declaration lookup uses canonical pattern/argument-pack identity in
`template_call.cpp`; enclosing substitutions belong to the selected pattern
and immutable parent-linked frame. Specialization declaration/body facts have
separate active/success/failure states. Completed nondependent type facts return
immediately. `query_dependencies.cpp` invalidates only reverse consumers of a
completed class/query, including the narrow prerequisite revision in cached
failure keys. No global generation counter retries unrelated specializations.

`syntax/occurrence.cpp` publishes source regions once. `Ast::instantiate`
projects occurrence IDs onto shared immutable source topology and defers member
bodies/default regions. It never invokes grammar. Semantic overlays establish
dependent facts without copying/reparsing the original body. The trace's two
`demand<T>` specializations make exactly two body transitions at every level;
`Packet<T>::unused`, containing an invalid dependent declaration, is never
demanded. Both int and long constructors, reads and destructors behave correctly.
The 645 parsed nodes remain shared beneath 1045 total source/occurrence IDs.

`lowering/symbols.cpp`, typed expression/function lowering and lifetime emission
consume recorded declaration, conversion, layout, linkage and ABI identities.
The frontend owners die at the end of each `build_program` iteration, leaving
typed LowIR pools and the minimal TU-linkage facts. Source object compilation
calls `lowir_model::optimize` at the requested level before native preparation.
External LowIR is parsed and validated; writing/replaying it is an explicit
inspection interface, never production transport. Audit-only validation is
available without repeated ordinary validation of unchanged in-memory IR.

`native::compile_image` expands required calls within their existing budgets,
creates one unit Workspace, then selects, optionally dumps and encodes each
function. The same Function supplies the MIR view, frame facts, instruction
encoding and host unwind preparation. The Selector and Function die after
that function; dense unit ID indexes and emitted code/fixups survive. Selection
does not retain all-function MIR or the frontend. Direct ELF writing consumes
typed symbols, relocations, sections and CFI/LSDA records. No assembler,
reference tool, host compiler or cached fixture implements production output.
Host C++ commands in the evidence build benchmark drivers or link emitted
objects, as allowed by the assignment boundary.

The review covered §§1–6 through these producers and consumers, §§7–9 through
the machine policies, data traces and frozen evidence below, and §10 through
the actual driver/encoding/object paths. Compact IDs, flat indexes, pooled
nodes and explicit owner lifetimes remain intact across stage composition.

## Finding and correction

Call setup previously constructed two local vectors on every call and used
`std::stable_partition`, which can allocate its own temporary buffer. That was
an inherited hot allocation at precisely the PA33 ABI-selection boundary.
`Selector` now owns and reuses register/stack move vectors across its calls.
Each call clears logical contents; default-initialized Assignment records reset
cycle-completion flags and captured-address facts. Capacity grows only to the
largest argument list and dies with the function. Register moves reserve the
fixed six-GPR/eight-XMM ABI bound once. Stable insertion by rotation replaces
the potentially allocating partition, preserving the prior instruction order.

Legality follows from unchanged move records and stable GPR-before-XMM order.
Parallel cycles, indirect targets, aggregate stack snapshots, variadic AL,
dynamic-copy early returns and stack restoration retain their original owners.
There are at most fourteen register moves; insertion and scheduling have fixed
work bounds, while stack transfers are linear in actual arguments. No new
source file, semantic key, cache, optimization pass or code growth was added.
The source trace and fixed measured objects verify byte-for-byte equivalence
to the accepted entry. This is an allocation-lifetime correction, not a claim
of improved executable runtime.

No further correctness, architecture, self-containment or timeout defect was
found in the reviewed PA33 ownership paths. Reference files and all course
comparison rules are unchanged; no reference exception or bundle revision was
needed. Four inherited substantial-header file-audit warnings remain warnings,
not unreviewed native implementation defects.

## Machine proofs, invalidation and budgets

| Owner | Legality/profitability and actual output | Complexity, budget and fallback |
| --- | --- | --- |
| Global placement | One definition, scalar integer, used, nonfolded value; fixed-effect census excludes clobbers. Whole-function reservation covers all backedges and parallel phi transfers. Rank by use count; constants/addresses rematerialize. Incoming carriers and over-aligned frame base stay reserved. | O(I + V log V), O(V) scratch; at most seven retained GPRs/five preserved registers. EH and multiple definitions retain conservative storage; no eviction, retry, clone or growth. |
| Ordinary placement and phi edges | Local intervals, call epochs, predecessor parameter availability and staged cyclic transfers retain value snapshots. Fixed-effect owners cover bulk, wide operations and calls. | Constant register pool; sorted edge identities; parameter flow has at most six newly contributed bits per edge. Spill work follows operands; no global dataflow restart. |
| Reload carry and control | Only private 64-bit homes with one store and complete same-block uses may use scratch; effect whitelist rejects unknown clobbers. Branch inversion only changes the final fallthrough pair. | Two linear use walks, at most three 64-instruction windows/home; one control compaction. No IR growth or fixed-point rescans. |
| Frame/ABI/debug | Actual homes, preserved mask and alignment determine saves/frame/CFI. Cleanup preserves labels and surviving locations; EH regions are prepared after final MIR control. | Linear work plus finite ABI operations; no invented cross-function locations. Global retention declines EH restoration paths. |
| Strlen prefix | Typed builtin identity plus exact direct ptr→i64 ABI, readonly/no-unwind/ordinary-return/query gates. Page offset ≤4080 admits a 16-byte SSE2 probe; zero mask or boundary uses the original target with RDI unchanged. | O(1)/site; 8/function, 128/unit; actual +53 bytes/site, reserved ≤64, hence ≤512/function and ≤8192/unit. All paths retain call clobbers. Exhaustion/incompatible signatures retain calls. |
| Dynamic memcpy | Canonical builtin, exact ptr/ptr/i64 signature, noalias pointers, no unwind and unused result; complete parallel ABI capture precedes REP. | O(arguments) setup, fixed `mov rcx,rdx; rep movsb`; no unrolled-size growth. Used results, memmove, unknown identities/effects retain calls. Applies at O0 too for its required native control. |
| Builtin persistence | Semantic builtin fact is independent of hosted ELF spelling. Explicit `builtin=` and legacy private markers feed the same typed metadata. Duplicate, conflicting and unknown identities reject. | Linear adapters, constant metadata/entity; no name-based production rediscovery. Signature/effect admission is checked independently. |
| Call scratch correction | Stable argument ordering and snapshots are unchanged; no stale `done`/address state survives call reset. | Function-owned capacity, ≤14 register records, O(max stack arguments) storage; no per-call vector/partition allocation. |

O0 keeps its original placement policy; O1–O3 share this bounded machine policy.
No distinct O3 machine pass is required. PA32 retains its fixed LowIR pass
composition, immutable inline admission, 32768 caller work bound and unit work
reservoir, two finite object splits and shared loop growth reservoir
`min(4096,2*(I+1))` with ≤256 clones/function. Machine selection consumes that
result without restarting inlining/loop passes. Its only PA33 expansion is the
separately bounded prefix reservoir. Analyses end at their function/pass owner;
changed MIR invalidates no persistent semantic cache. There is no multiplicative
unbounded pipeline retry or retained cross-function machine optimization state.

## Representative end-to-end evidence

[The source trace](../student.tests/pa33/audit-trace.cpp) follows `Packet<T>`
and `demand<T>` from grammar, typed specialization and lifetime facts through
PA32 optimization, native call/loop placement and ELF. At O0–O3, direct objects
equal replayed O0-LowIR objects and pre-refactor objects. Twelve executions
check three runtime argc inputs per level. Readelf/disassembly retain symbols,
sections and unwind details. The trace interleaves eight integer and two FP
arguments, exercises stack arguments and asserts destructor counts.

Builtin provenance is visible in source-produced LowIR, consumed MIR and the
encoded object. `length` retains its call location and `strlen_prefix=16` only
at optimized levels. `copy` emits `copy_bytes_dynamic` at every level. Existing
personal controls also cover six-parameter pressure, cyclic phi transfers,
reversed memcpy setup, zero/large copy sizes and strings ending at a protected
page. Admission stress reaches exactly 8/128 prefix limits and checks four
incompatible ABI/effect fallbacks at all levels.

Combined MIR+executable invocations equal separate invocations at every level.
Negative controls check missing inputs/options/arguments, unsupported targets,
unreadable/invalid LowIR, metadata conflicts, unwritable/failed writes and
unresolved external symbols. Both help forms succeed.

Inspection of the fixed `kernel0` loop finds four loads and two stores per
iteration in A, versus register-resident induction/accumulator state in B.
B saves RBX/R12/R13 once and restores them once; the corresponding call loop
keeps those values live across its actual call. The frame remains 32 bytes.
This is executed memory-traffic evidence, not an inference from IR node count.
Floating, memory, EH, template-heavy and compiler-component benchmarks cover
the other inherited surfaces. General DWARF line-section generation and the
full self-host ladder belong to their later contracts; PA33 requires and passes
meaningful LowIR/MIR debug locations and ABI/unwind preservation.

## Performance acceptance and validation

[Final measurements](../student.tests/pa33/evidence220/performance.md) retain
all four dimensions, A/A ranges, six ABBA pairs, extrema and every observation.
The source/binaries/flags/inputs are frozen; timed compilation and execution
are separate, with runtime inputs and checked results. The final measurement
process runs after builds/tests complete. The many-call scratch benchmark
alternates wide, small and zero-argument calls, checks results and compares
identical generated objects before interpreting compiler cost.

All **812** current samples recompute successfully, and all **14** fixed final
objects equal their accepted 219 outputs. Affected runtime B/A medians/ranges
are loop 0.335 [0.317–0.350], calls 0.635 [0.628–0.639], short string 0.595
[0.589–0.597], dynamic copy 0.029 [0.028–0.031], and long string 1.140
[1.103–1.202]. Their compiler ratios are 1.007–1.015 and maximum peak RSS
increase is 480 KiB. Prefix text grows 6784 bytes/unit within the 8192-byte
reservation; loop/call text shrinks. Common O0 images are identical; O2 text
shrinks 10–25 bytes. Common timing spreads overlap one, so no new general
runtime or compiler speedup is inferred. The compiler component's text stays
34466 bytes, with peak RSS 77592/77820 KiB. Scratch compilation is 0.831
[0.719–1.065], peak RSS 39848/39668 KiB and checked runtime 0.985
[0.847–1.082] on identical 1290240-byte text; this does not establish a
repeatable speed gain from the lifetime correction.

The complete 219 handoff is independently verified at its committed snapshot:
547 source bindings, 184 artifact bindings and 924 observations, including
rejected experiments. Its long-string slowdown and rejected suffix continuation
are preserved. Stage acceptance retains repeatable affected runtime benefits,
bounded compiler work/memory/text, mandated quality envelopes and correctness.
A longer-string prefix miss has a disclosed probe cost; the same bounded probe
provides the measured short-string benefit. It is not reported as a universal
strlen improvement. Timing ratios near one and identical generated code do not
establish a code-generation speedup.

Inherited ad hoc 2x compile, 1.75x RSS, zero-growth, 10% runtime and subsequent
1.5x/1.05x/1.25x targets remain historical diagnostics under spec.md's
stage-scoped rule. They do not override mandated MIR bounds, finite policy
budgets or semantics. No measurement, failed attempt, comparison or coverage
was removed. A compiler-component object supplies the applicable self-hosting
compile benchmark; PA34 inception is not a hidden PA33 exit gate.

Required final source checks pass: `make test-pa33` (57/57 plus five native
controls and 18 driver modes); `make -C pa33 test-debuginfo` (5+5+1);
`perl scripts/cppgm_file_audit.pl --stage pa33 --paths dev/src` (pass, four
inherited warnings); and `make test-report-through-pa33` (5454/5454, 33 stages).
The supplied primary log and both independent reports say 5454/5454, while the
prompt census said 5840/5840. The actual command output governs this record;
fixture trees through PA33 are unchanged. No new test timeout occurred.

## Handoff ledger

- `441d5ec9`: stage entry, baseline and ownership plan reviewed.
- `f9df1b5b`: level propagation, global placement, frame/control/bulk and
  builtin policy reviewed through selection and encoding.
- `cbf6f115`: builtin identity replay and incompatible declaration fallback
  reviewed through reader, writer, source producer and native consumer.
- `c1328ca0`: implementation handoff and every frozen/rejected measurement
  verified; its independent-audit obligation is closed here.
- `d7b7c9cfedf916a38af813fd9e5144d2cdf058b0`: function-owned call scratch correction, representative trace,
  independent controls and evidence verification.

The final plan and binding record close all handoffs. No PA33 work is deferred
to an unnamed checkpoint or waived because tests pass.
