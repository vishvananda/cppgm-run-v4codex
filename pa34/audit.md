# PA34 ownership and reproducibility audit

PA34 retains the production pipeline reviewed in [PA33](../pa33/audit.md).
The corrections in [the ledger](fixes.md) repair existing language, ABI,
native or build contracts; they introduce no self-compilation language mode.

## Source to ELF

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

## Native proof and budgets

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

## Executed trace

`student.tests/pa34/trace.py` reuses the PA33 `Packet<T>`/`demand<T>` trace with
mixed integer/floating/stack arguments, destruction, undemanded invalid members,
builtin identity and source debug provenance. At O0–O3 the seed and canonical
self compiler produce identical LowIR, ELF objects and nontiming work counters.
Direct objects equal explicit O0-LowIR replay objects. All 24 generated-program
executions pass. Commands, disassembly, unwind records, counters and hashes are
retained under `$RALPH_ARTIFACT_DIR/pa34-221/trace-final` (earlier traces remain).

## Canonical validation

The required file audit passes, with the four inherited header warnings. The
host report passes 5454/5454; canonical self rungs through PA5, PA8 and PA33
pass. Pptoken inception and the required root command
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

## Frozen performance and stage acceptance

[All measurements and their summary](../student.tests/pa34/evidence/performance.md)
retain A/A calibration and six ABBA blocks: 476 final observations and 56 earlier
stack-diagnosis observations. Compiler latency/RSS and checked runtime/text are
reported together. The final seed/self compiler-source paired latency ratio is
1.811 [1.546–2.011]; common O0/O2 ratios are 2.200–2.546. Every generated object
matches, and common executable images are identical. Untimed compiler-source
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
