# PA34 full-stage audit 222

Stage base: `99ea7d9d` (completed PA33 audit). Review entry: `5a90033a`.
The preceding implementation turn made progress: committed fixes, canonical
comparisons and frozen observations exist. No prior build/test process is live.

Independent review plan: verify the recorded source/artifact hashes and raw
observations; inspect every stage change and its reducer/proof; trace source,
demanded templates and an optimization fact through the shared production
pipeline; check generation dependencies, deterministic emission and unchanged
resource/comparison policies; rerun the required exit commands and relevant
personal/debug checks. Recompute the frozen A/A and ABBA summaries before
accepting performance claims. Close this plan with current evidence below.

## Findings

Audit 222 found three gaps: the PA33 self-host rung selected the seed for its
18 driver checks, the mixed-auto reducer was under PA14 instead of PA20, and
the common performance harness hashed prepared images but not every timed
compilation. All are corrected below; production compiler code is unchanged.
The independent entry check verified 598 source records, 389 retained artifacts,
five binaries and 420 canonical objects against their recorded SHA-256 hashes.
The historical 532 A/A/ABBA observations also recompute independently.


PA34 retains the production pipeline reviewed in [PA33](../pa33/audit.md).
The corrections in [the ledger](fixes.md) repair existing language, ABI,
native or build contracts; they introduce no self-compilation language mode.

### Source to ELF

`lowering/driver.cpp::build_program` owns immutable preprocessor buffers,
interned identifiers, streaming post/syntax cursors, the shared Ast and Analyzer
for one translation unit. `Parser::translation_unit` publishes each declaration
to the analyzer once. Template instantiation uses retained source occurrences,
not grammar replay. `template_call.cpp` interns argument vectors by compact
type/argument IDs, and `template_instantiation.cpp` records separate active,
success and failure body states. `query_dependencies.cpp` invalidates only
reverse consumers of completed prerequisites.

The added local-static relocation queue stores entity/scope IDs. Its monotonic
cursor consumes each declaration after constructor-action demands finish; newly
discovered bodies join the existing queues. It neither restarts the translation
unit nor scans all pending declarations. Conditional cleanup uses recorded
conversion/elision identities. Complex initialization uses the existing typed
list plan and component conversions, including constant encoding and narrowing.
No spelling-based self-host exceptions or generated answers were introduced.

Typed `Procedural` lowering consumes these facts and writes the Program pools.
Frontend owners die at the end of the translation-unit iteration. Ordinary
object compilation invokes `optimize`, native legalization/selection, encoding
and the ELF writer directly. LowIR text and validation are explicit inspection
adapters. `native/driver.cpp` selects, optionally dumps and encodes one Function
before releasing its MIR and local scratch. It retains only unit indexes and
output code/relocations. There is no assembler or host code-generation fallback.
Host tools build the seed, discover the configured host environment at build
time and link compiler-generated objects at the assignment's ABI boundary.

The shared generated-header filename/owner now comes from
`frontend_source_sets.mk`; all generations consume the same host configuration.
Canonical PA34 object dependencies invalidate descendants of a changed compiler.
Single-object probe links are retained only as diagnostic evidence.

### Identity, allocation and demand review (spec §§1–6,8)

`SourceBuffer` keeps immutable bytes/file identity. `IdentifierTable` uses a
byte arena and flat open addressing; the syntax cursor discards consumed ring
entries. `NodePool` stores parsed topology once and instantiations as compact
source/context occurrences. `FactStore` allocates slabs; `ExpressionStore`
shares properties and keeps object/conversion uses in separate compact records.
All are TU-owned, with no per-node owning shared_ptr or recursive destruction.

`lookup.cpp` indexes ordinary/tag/qualifier scope+name identities separately and
keeps using/base/associated-scope edges explicit. Overload work uses candidate
sequences, records selection/conversions once and represents expected rejection
as facts; hard diagnostics belong to final invalid uses. No rendered semantic
key or whole-registry fallback was found on the reviewed production paths.

`template_call.cpp` keys a specialization by canonical pattern/argument-pack
identity; the selected pattern owns its enclosing frame. Parent-linked frames
share bindings, `substitute_type` reuses nondependent facts and caches dependent
work by type/frame. Separate declaration/body states observe in-progress work.
`query_dependencies.cpp` records reverse edges and narrow prerequisite revisions;
it invalidates only consumers affected by class completion. Source projections
in `occurrence.cpp` never invoke grammar and defer unused bodies/defaults.
These source owners substantiate the executed Packet<T>/demand<T> trace.

The frontend dies after typed lowering. Whole-unit LowIR supports explicitly
bounded interprocedural work; serialized IR and all-function MIR are absent.
FunctionBuilder, selection scratch and MIR have function/pass release points;
unit indexes and output code/relocations outlive them. Process launching under
`dev/src` is confined to the test runner's isolation/re-exec boundary. Builtin
spellings enter typed registries; standard ABI abbreviations are ABI encoding,
not library-body recognition. No production assembler/compiler delegation or
mutable process-global semantic cache was found.

### Optimization fact and budgets (spec §§7–9)

The trace's strlen identity enters through the semantic registry and
`lowering/function_declaration.cpp`'s readonly/no-unwind signature. Typed symbol
metadata survives to `native/calls.cpp`, which also requires exact ptr→i64 direct
ABI and ordinary return/query facts. Only then may `prefix_call.cpp` encode a
16-byte SSE2 probe at page offsets ≤4080. Boundary/no-zero paths retain the
original call and unchanged RDI; conservative call clobbers remain. MIR reports
the actual prefix encoded. The trace has zero prefixes at O0 and one at O1–O3.
The short-string benefit and long-string regression remain disclosed in PA33's
frozen evidence; no new PA34 optimization gain is inferred.

I/O/B/E/P/S/F denote instructions/operands/blocks/edges/parameters/slots/functions
at admission. Fixed linear censuses and output work are additional. The detailed
[PA32](../pa32/plan.md) and [PA33](../pa33/plan.md) limits remain operative:

| Owner | Work/growth, invalidation and fallback |
| --- | --- |
| Scalar/CFG | Dirty-use worklists; each propagation/alias walk ≤16*(I+uses+1), dominance ≤32*(I+O+E+1), call/EH closure ≤32*(I+O+B+1). Unproved facts stay conservative. |
| Storage/memory | Promotion ≤16*(I+O+B) with capped phi growth; two fixed splits, ≤64-byte/16-field homes, ≤8*(I+1) growth each. Memory ≤128*(I+O+E+1), ≤32 cells/state, ≤16 diamonds, ≤64 comparisons/proof, ≤16 pieces/128 bytes per copy. Effects invalidate affected states. |
| Optional calls | Immutable admission; ≤4096/site, ≤32768/caller, ≤32*(I+O+P+S+F+1)/unit; depth ≤64, growth ≤1536/caller (2048 single-use). Missing proof/budget retains calls. |
| Forced preparation | Separate ≤262144/caller and ≤4194304/unit; cycles/unsupported frame behavior retain calls. |
| Loops | One invocation; proof ≤16*(I+O+E+1); O3 ≤4 trips/64 clones per loop, ≤256/function and min(4096,2*(I+1))/unit. Widened endpoints, zero-trip guards, residues and parallel-phi snapshots preserve semantics. |
| Native | O(I+V log V) placement, ≤7 retained GPRs, constant pool; ABI register moves ≤14, stack setup linear, function-owned scratch. Whole-function reservations cover calls/backedges; EH declines. No allocator retries/cloning. |
| Prefix/encoding | ≤8/function, ≤128/unit; actual +53 bytes/site within 64 reserved, ≤512/function and ≤8192/unit. Exhaustion retains calls. Linear selection/encoding/ELF work. |

O0 bypasses optional LowIR passes. O1 uses bounded scalar/storage/control/call/
memory policies; O2 adds exact call-constant propagation, O3 permits proved small
loop unrolling. Optimized levels share the bounded machine policy. Fixed pass
composition never restarts admission or multiplies growth through an expanding
fixed point. Analyses end at their pass/function owner; transformations preserve
ABI, unwind and meaningful debug facts. PA33's retained inspection records
actual loop loads/stores removed, saved registers and frame size, rather than
inferring runtime profit from smaller IR.

## Changes

### Independent audit corrections

`pa34/Makefile` now passes CPPGM_DRIVER_APP pointing to cppgm++-self in its PA33
branch and makes PA32/PA33 depend on that compiler. Previously PA33 only set
LOWIR2NATIVE_APP, leaving its 18 driver controls at the seed default. The corrected
rung passes all 18 driver modes, 57 native behavior/MIR cases and five native
properties. This strengthens coverage without altering course comparisons.

The mixed-auto negative test/proof moved to PA20, which owns placeholder
variable deduction. It does not expand PA20's one-declarator positive contract.
All three generations reject it and the PA11 narrowing reducer. Historical
bindings keep the old path at review entry 5a90033a. The common measurement
harness now verifies every timed object hash after stopping the clock, and the
trace verifies telemetry-off objects. `verify_samples.py` independently checks
sample order, calibration, summaries, work and output equality.

The sole stage reference change remains one trap-probe token. The
[proof](../student.tests/pa29/trap.md) now explicitly combines PA29's registry
requirement, the GNU trap contract and C++11 [cpp.cond]/3,6: a nonzero probe
selects the first group. C++11 alone does not specify vendor builtins. The
unchanged fixture therefore emits `bad`. Bundle revision and SHA-256 in that
proof match manifest.tsv; coverage/comparators and all other references stay
unchanged. Host compiler agreement is not the proof.

### Stage commit review

Every production delta from 99ea7d9d to 5a90033a was inspected with its reducer:

| Commits | Owning correction |
| --- | --- |
| 9e5c5aaa, ab294a3c | PA5 elaborated class binding and functional-pointer prediction. |
| c23eace7, 783226b2, 6a9c114c | PA7 scoped-enum promotion, PA11 local-static action scheduling, PA12 conditional cv/cleanup. |
| 3c3ed533, 826933ea | Ill-formed compiler declarations; PA20 mixed-auto and PA11 narrowing rejection controls. |
| 0c511fe1, ed4ef511, 673bda97, 11d7c09e | PA29 trap, floating-helper order, complex libm signatures/component list initialization. |
| 17b90083 | Shared generated host-header filename and owning source. |
| 3db616dd, b280580f | PA24 indexed wide load/store scratch, including stack/spilled addresses. |
| e59b73dd, 5cd7d549 | PA29 deterministic complex emission and PA10 recursive dispatcher stack correction. |
| 5a90033a | Canonical comparisons and all frozen handoff measurements independently bound and checked. |

The [ledger](fixes.md) links owning reducers and standard/contract proofs. The
three principal native/reproducibility/resource diagnoses follow.


The self PA25 crash was traced to the i128 equality load in
`semantic/constant_integer.o`, then to `native/selection.cpp::move`.
Its first load destroyed the scaled index required for the second word. The
scratch choice now preserves source and destination address carriers across
both chunks. This is constant work, without extra MIR operations, allocation,
code-growth reservoir or optional optimization. The PA24 reduced LowIR fails
before the change and passes afterward. Replacing only the affected self object
reproduces the seed's exact LowIR for the original failing input.

The subsequent full inception comparison differed in `semantic/complex.o`.
Its real/imaginary additions and subtractions were emitted in different orders
while retaining identical results. `lowering/complex.cpp` passed two IR-emitting
calls as arguments to another call; C++11 allows either evaluation order.
Explicit full-expression sequencing now makes the artifact order independent
of that choice. The [PA29 proof and reducer](../student.tests/pa29/complex-order.md)
separate this reproducibility defect from miscompilation. Replacing only the
self-built lowerer object restores exact equality for the original compiler
source. No valid language rule or optional optimization was changed.

The next inception failure, semantic/output.cpp, exhausted the self compiler's
8 MiB process stack. Diagnostic 64 MiB runs and frozen A/A + ABBA measurements
produced identical objects/work and isolated a code-quality difference: the
expression dispatcher reserves 18,704 bytes in the self build versus 1,312 in
the seed. The [PA10 reducer and measurement proof](../student.tests/pa10/deep_calls.md)
retain the full evidence. A separate call dispatcher now uses 432 bytes while
keeping the original guards, debug/invocation lifetimes and lowering order.
The original source and 400 nested procedural calls pass at the normal stack
limit. No production stack/resource setting, valid input construct, cache,
lowering worklist or optimizer budget changed. A/A + six ABBA comparisons find
no repeatable compiler speedup; RSS falls 4,268 KiB and generated text is exact.

Existing optimization proofs and budgets remain owned by PA32/PA33: immutable
inline admission with 32768 caller work and a unit reservoir, loop growth
`min(4096,2*(I+1))` and at most 256 clones/function, and strlen prefix limits
8/function, 128/unit, 64 reserved bytes/site. Unsupported proofs or exhausted
budgets preserve the conservative path. No new optimization performance claim
or stage-independent ratio gate is introduced.

### Executed trace

`student.tests/pa34/trace.py` reuses the PA33 `Packet<T>`/`demand<T>` trace with
mixed integer/floating/stack arguments, destruction, undemanded invalid members,
builtin identity and source debug provenance. At O0–O3 the seed and canonical
self compiler produce identical LowIR, ELF objects and nontiming work counters.
Direct objects equal explicit O0-LowIR replay objects. All 24 generated-program
executions pass. Commands, disassembly, unwind records, counters and hashes are
retained under `$RALPH_ARTIFACT_DIR/pa34-221/trace-final` (earlier traces remain).

## Performance Evidence

[All measurements and their summary](../student.tests/pa34/evidence/performance.md)
retain A/A calibration and six ABBA blocks: 476 final observations and 56 earlier
stack-diagnosis observations. Compiler latency/RSS and checked runtime/text are
reported together. The handoff seed/self compiler-source paired latency ratio is
1.811 [1.546–2.011]; common O0/O2 ratios are 2.200–2.546. Prepared A/B objects
match, and common executable images are identical. Untimed compiler-source
counters, every common compile's nontiming counters and the executed trace
agree. This separates generated compiler code quality from repeated work or
miscompilation; it establishes no optimization speedup.

The default GCC seed and self compiler have different machine code and allocator
linkage. These comparisons characterize those canonical configurations, not an
isolated optimization. The necessary dispatcher correction stays within the
original stack and all mandated time/RSS limits. No optional transform was
added. Inherited self-selected 2x latency, 1.75x RSS and zero-growth targets remain
diagnostic under spec §9, as already classified in PA33; they do not override
current-stage acceptance or waive a correctness/resource failure. Existing
level-specific work/growth bounds and coverage are unchanged.

The [binding](../student.tests/pa34/evidence/binding.json) ties canonical checks,
every compared compiler object, binaries, sources, reducers and complete retained
artifacts to SHA-256 identities. Measurements and trace records are also checked
in; generated objects, executables and logs remain outside version control.

### Independent measurements and acceptance

[Evidence 222](../student.tests/pa34/evidence222/performance.md) adds 476 fresh
frozen observations with four A calibration runs and six ABBA blocks on the
same compiler-source and common O0/O2 inputs. Compilation and checked execution
are separate, pinned to CPU 2, without concurrent builds/tests. Every one of
224 timed common compilations now has an output hash. The earlier 532 records
remain unchanged: their common data establish prepared-image and work equality,
not per-sample object hashes. Recomputed summaries preserve all spreads/noise.

Fresh compiler-source paired latency is 1.981 [1.582–2.205], with peak RSS
77,852/87,896 KiB and identical 34,466-byte component text. Common compilation
paired medians are 2.365–2.604; all generated executables remain byte-identical.
Their runtime paired medians span 0.952–1.001 with spread consistent with noise;
actual executable text is 151,474–151,781 bytes at O0 and 125,086–125,268 at O2.
The full report retains each workload's latency/RSS/runtime/text and calibration.

Seed/self use different code generators and allocator linkage; measurements
characterize those canonical generations. Self and inception match exactly,
so their executable compiler code is the same generation. Nontiming work and
emitted objects agree despite the disclosed latency/text gap. No optimization
speedup is claimed for PA34. The inherited 10% runtime and later 1.5x/1.05x/1.25x
targets also remain diagnostics under spec §9, alongside the ratios above.
Necessary semantic costs do not excuse avoidable regressions; mandated resource,
correctness, debug, coverage and finite work/growth bounds remain unchanged.

## Validation

The required file audit passes, with the four inherited header warnings. The
host report and final self ladder each pass 5454/5454 across 33 stages; canonical
self rungs through PA5 and PA8 also pass. Pptoken inception and the required root command
`make inception CXX=g++ CPPGM_HOST_CXX=g++` both match byte for byte. The final
self/inception comparison covers all 420 compiler object pairs. The final
self/inception compiler SHA-256 is
`8f8a194eac0f8ea2ac5fcee010d1b540d7671228789d88e1e4e3cb9110e3f646`.
No timeout, RSS, stack or comparison policy was changed for these runs.

Native debug checks pass 11/11 and the self compiler's native driver checks pass
18/18. The deep-call reducer explicitly enforces the original 8 MiB stack and
passes at O0/O3. The canonical self compiler also reproduces the original
semantic/output.cpp object exactly under the default stack limit. Personal
language/backend reducers remain under their earliest owning assignments.


Audit 222 reruns the five exact exit commands and the full self PA1–PA33 ladder.
The corrected PA33 rung also passes independently with the self compiler wired
into the driver controls. Seed/self native debug each pass 11/11; owning personal
reducers pass explicitly. The 400-call control also passes in the inception
compiler at the normal 8 MiB stack. The renewed source trace retains 396 tokens,
48 maximum pending, 645 parsed nodes, 1045 source/occurrence IDs and two template
body transitions. Eight telemetry-off objects match the reporting-enabled ones.
Telemetry reports existing facts/pool sizes; it demands no extra semantic work.

The four inherited file-audit warnings concern typed interfaces, accessors and
pool/model construction in procedural.h, lowir/model.h, semantic/analyzer.h and
semantic/model.h. New dispatcher/behavior implementations have .cpp owners;
there is no newly introduced ownership failure. The exact file-audit command
passes. Canonical object/binary equality and shared source/configuration ownership
were verified again; restored/probe targets are absent from the canonical path.

[The audit binding](../student.tests/pa34/evidence222/binding.json) records current
commands/statuses, source/binary/object hashes and retained artifact hashes.
Full logs and generated artifacts live under $RALPH_ARTIFACT_DIR/pa34-222.
No generated object, executable, log or .my output is committed. The independent
plan is closed by this evidence; no unnamed follow-up or relaxed gate remains.
