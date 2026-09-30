# PA25 final whole-stage audit141

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: 67ea755a273223b7fcc973215e0ca1ec8131689d

Target: **PA25 full-stage**. Phase: **audit**. Disposition: **complete**.
Entry was `e6cdc480`; the review covers the entire stage from the base above,
including the eight implementation139/140 commits since the last checkpoint's
code marker `4530fe93`. The earlier [audit138](audit138.md) is preserved verbatim.
Its conclusions and the implementation handoffs were inputs, not substitutes for
reading the combined source, assignment, [spec](../spec.md), testing policy and
plan. [Validation141](../student.tests/pa25/validation141.json) pins the complete
commit/source inventory, commands, binaries, traces and measurements.

## Findings and changes

**Exception binding was lost at the semantic/runtime boundary.** Catching `T*&`
bound a local copy, so assigning another pointer and rethrowing did not update the
exception object. Mutable pointer references also accepted conversions that are
permitted only for value or const-reference handlers. Nested pointer/member-pointer
qualification chains were incompletely matched, and a handler could name a pointer
to an incomplete class. The reduced programs are in
[audit141.py](../student.tests/pa25/audit141.py), including rejection controls,
exact const-reference identity, mutation through rethrow, secondary-base adjustment,
nullptr, member/function pointers and direct/separate/mixed three-TU execution.

The proof is C++11 [except.handle]/1,3,16–17 and [conv.qual]/4–7 in
[N3485](../doc/n3485.txt) ([published draft](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3485.pdf)).
Only value and const-reference pointer handlers receive the listed pointer
conversions; an exact reference binds the exception object. Changes through a
non-const reference remain visible on rethrow. Mixed qualification conversion
requires matching member owners and intervening const qualifiers. C++11 does
**not** add a root member-pointer qualification conversion to the handler list;
the negative test preserves that distinction. Compiler agreement is not the proof.

`semantic/source_exception.cpp` now records compact binding mode by handler ID.
`lowering/exception_handlers.cpp` consumes it and keys function-local selectors
by `(type, binding)`. Typed LowIR clauses, validation, text adapters, MIR clauses,
MIR dumps and native matcher arguments all carry that same fact. Exact references
receive the payload address; mutable-reference mismatches fail. Converted const
references receive handler-owned temporary storage, including separate member-
pointer temporary identities across nested catches. The 80-byte exception header
keeps only transient match state, cleared both before matching and at catch entry;
a failed conversion or catch-all cannot consume stale temporary state. Handler
storage survives inner catches and ends with its function frame. A two-word
member representation is copied only when required; ordinary pointer temporaries
use one word. The private begin-catch signature changes with object format **4**;
version-3 objects must be rebuilt rather than silently linked with the new runtime.
The final adapter review also found that native clause indexing could read beyond
a clause when an external input omitted its selector. Explicit source selectors
are unchanged; unsupported selector-less native clauses now receive a diagnostic
before operand access. Generic LowIR views retain their accepted syntax.

**Completed static layout was reconstructed incorrectly.** A packed `char,int`
class had semantic offsets 0 and 1, but native data emission reinserted padding
before the integer. Packed arrays and pointer relocations exposed the same hole.
Overaligned class objects could lose alignment between objects/TUs. Declaration
alignment was retained only for fields; local statement parsing discarded leading
alignment attributes entirely.

The parser now retains local alignment metadata through the existing occurrence
model. Semantic declaration facts record and check explicit object alignment by
entity ID, including template locals. Source slots consume this fact; the existing
native frame-alignment path supplies aligned automatic storage. Defined structured
and explicitly aligned globals reuse the compact LowIR `Object(bytes,alignment)`
type as their exact storage layout. Items contain already-decided padding; native
emission writes them consecutively and checks the final extent. Compiler objects
carry the maximum required alignment through the linker. This adds no field to
`Global` and no second layout graph. Legacy untyped structured LowIR retains PA24's
item-derived alignment contract. The course presentation adapter retains its
original spelling; production inspection exposes the new exact layout.

C++11 [basic.align]/1 and [dcl.align]/1–6 require the accepted alignments and prohibit
weakened, invalid and inconsistent specifications. `#pragma pack` follows this
compiler's supported extension and the offsets its own semantic layout establishes.
The existing private-object/data-image alignment limit is 4096; unsupported global
alignments are rejected, not silently truncated or weakened. Controls cover packed
constant objects/arrays/relocations, aligned zero and initialized storage, local and
template declarations, static locals and cross-TU direct/object/mixed linking.

The final expanded control set exposes **47 failures in 178 checks** in the frozen
entry binary and passes **178/178** on the final compiler. The nested converted
member-reference identity reproducer distinguishes two simultaneously live handler
temporaries. Another **15/15** adapter/version checks pass; validation141 records
the commands and initial failures. One intermediate full report exposed a
PA21 initializer-list display change; the backing-storage presentation owner was
corrected, with the original fixture and comparison rule preserved.

## Independently reconstructed architecture

| Spec surface | Actual owner, identity, work and release boundary |
|---|---|
| 1: source/parser | `preprocess` owns immutable buffers and interned names; `PostTokenCursor` feeds the geometric ring in `syntax/cursor`. Integrated parser/Analyzer construct one source graph. `syntax/occurrence.cpp` retains deferred regions and compact `(source,context)` views, including alignment operands; substitution does not replay grammar or clone complete syntax trees. |
| 2–3: canonical facts/lookup | TU-owned entity/type/scope/constant/ABI IDs and flat indices carry equality. `semantic/lookup.cpp` indexes `(scope,name)` and explicit parent/using/base/ADL edges. Overload candidates retain selected declaration/conversion IDs for lowering. Wide integers keep constant-pool identity and explicit ABI numeric sign; diagnostic/mangled strings are views or object-linkage names, not semantic equality keys. |
| 4: templates/demand | `template_binding*`, `template_instantiation`, definition environments/owners and specialization owners separate definition-time fixed facts from contextual dependent work. Parent-linked frames avoid copying visible environments. NotStarted/Active/Success/Failure states distinguish recursive demand from new work; class completion does not demand unrelated member bodies. |
| 5: scheduling/invalidation | `query_dependencies.cpp` stores deduplicated reverse edges from an incomplete class/query to affected consumers. Completion resets those facts and per-query revisions, including failed dependents; no global generation flush. Deferred function use, definition and vtable queues are owner-indexed. Lifetime/selector mappings are function-owned and reset at the function boundary. |
| 6: typed lowering | `lowering/driver.cpp` constructs `Program` directly while the Analyzer is alive, then releases source/semantic owners before native construction. Constants, destination identities, ABI entries, construction/destruction actions, F80 evaluation, catch binding and exact storage layout cross as typed facts. Each TU gets its own frontend; source, compiler-object and mixed driver paths converge on `compile_object`/Linker. Text readers/writers serve explicit tools only. |
| 7: native | `native/driver.cpp` creates and destroys one selected function's MIR/frame/encoding state per iteration. The unit workspace holds compact ID maps. EH clauses are indexed once by block; branch labels retain ownership. Six-bit parameter-clobber propagation queues only changed blocks, at most six additions per edge. Direct machine encoding and ELF byte writing invoke no assembler. |
| 8: allocation | Semantic nodes/facts/expressions use TU slabs and sparse compact indices, not a second owning semantic tree. Local substitution/candidate/selection temporaries die with their owner. Native objects retain only bytes, symbols and producer-owned fixups; runtime support IR dies after object construction; linker buffers die with the invocation. No accumulating process-global cache was found on this path. |
| 9–10: bounded/self-contained | Foreign ELF is an explicit bounded input adapter with one extent sort. Definition/alias/GOT ownership feeds a flat deduplicated relocation worklist; retained definitions/edges are visited once. Class/RTTI/EH services are a finite set of compiler-built typed IR bodies plus native syscall primitives. No reference or host compiler implements source output; empty-PATH controls exercise that boundary. Host tools only build the compiler, API probes and allowed foreign test helpers. |

The stage-wide source review also rechecked the earlier enum range/representation,
ABI literal, floating narrowing, statement-expression lifetime, alias/GOT and builtin
identity repairs. No name/address recovery fallback was reintroduced. Static class
initialization retains destination-relative self pointers, vptrs and base/field
layout; RTTI traversal checks public access and ambiguity, including virtual-base
identity. EH ownership spans constructor failure, function-try handlers, catch-copy
failure, rethrow, cleanup and final destruction/freeing. Hierarchy matching follows
required paths with depth-proportional stack; it is not falsely described as linear
in a shared inheritance graph.

## End-to-end trace and optimization review

[audit141-trace.cc](../student.tests/pa25/audit141-trace.cc) combines a demanded
`Item<9>` specialization, constant polymorphic globals with self pointers, a
function-template function-try block, destructor cleanup, pointer-reference
mutation, rethrow, secondary-base conversion, virtual dispatch and dynamic_cast.
The same type/entity/ABI identities are followed through validated production
LowIR, canonical text roundtrip, actual MIR and independently executed native ELF.
The trace checks the replacement object, adjusted address, virtual result and one
destruction, not merely IR counts.

Observed counters: **312 tokens**, pending ring maximum **41**, **489** parsed nodes
and **144** occurrence views; **3** specializations, **1** demanded template body
transition and **1** template class completion; **2** substitution frames;
**2** fixed expressions reused twice; **233** semantic fact records; **256** LowIR
instructions and **336** operands. Constant polymorphic globals need **zero** dynamic
initializer units. Additional traces inspect packed relocations, aligned local and
template storage, converted member-reference binding, runtime services and F80
arithmetic. Canonical typed facts survive all roundtrips. Complex ABI-entry traces
can reorder functions because the explicit writer uses `function_order` while
native emission uses the function pool; whole-ELF byte equality is therefore a
diagnostic there. Both images execute the same checked result. Simple layout and
floating traces remain byte-identical; no course comparison was weakened.

The independent final statement trace also exercises two demanded specializations:
**273** parsed nodes plus **200** occurrence views, **2** body transitions and
**8** fixed expressions with **16** uses. This checks shared definition facts
across `int`/`long` instantiation and normal/early destruction. The combined class/EH
trace emits **12** source functions, demands **13** runtime functions and visits
**63** definitions / **243** relocation edges; telemetry is outside timing runs.

The useful optional fact reviewed is exact power-of-two scaling. Source F32/F64
arithmetic carries F80 evaluation; `native/floating.cpp` tests at most two constant
operand bit patterns. A finite nonzero normal/subnormal power of two times a
binary32/64 value is exact in binary80, including the full possible product
exponents, leaving only the declared-precision rounding. The selected MIR records
the proof, and the encoder uses shorter SSE work without x87 operand spill/reload
traffic. Unknown operands keep F80. There is no persistent analysis to invalidate,
no search/allocation/new IR, no additional frame bytes and zero text-growth budget.
The 120,120 float/double comparisons and production/MIR trace check legality;
frozen correct/correct measurements establish profitability separately.

Pipeline-wide budgets remain proportional to real source/fact/edge work, per-
function IR and emitted bytes, with required foreign extent sorting O(n log n).
The new binding/layout propagation adds constant work per clause/item/declaration,
one sparse semantic entry per applicable declaration/handler, and one bounded
handler temporary when conversion is possible. Matching state remains in the
existing header; EH registrations remain 96 stack bytes. No speculative cloning,
unbounded fixed point or optional growth pass was added. Inherited PA24 branch,
register, ABI, bulk-copy and declared MIR bounds remain enforced by the root report.
Later O1–O3 policies, host object interoperability/metadata and self-hosting remain
owned by their assignments rather than invented PA25 gates.

## Performance acceptance and validation

[Performance141](../student.tests/pa25/performance141.md) reports final frozen
compiler latency/peak RSS, checked executable runtime and text size together.
It retains A/A calibration, six ABBA blocks, every paired ratio/spread and all
intermediate observations. Template, loop/call/memory, floating, class, cast,
allocation and exception workloads compare equivalent correct entry/final behavior;
new binding semantics have a final/final baseline. Timing runs had no concurrent
build or correctness runner. All inherited 138–140 manifests were independently
rehashed, including frozen inputs/binaries and executable images.

No final compiler speedup is claimed from noisy samples. Required binding checks
increase demanded EH support text; costs and affected runtime are disclosed.
The unchanged exact-scale selection retains its prior isolated **17.3%** runtime
benefit (paired ratio **0.827 [0.824,0.838]**, text **377 -> 362**), with identical
final floating benchmark code. Fewer IR nodes are not used as evidence of benefit.
Inherited **15% latency/RSS** targets and blanket zero-growth targets for necessary
semantic work remain diagnostics under spec section 9. Historical misses and all
measurements are preserved. This does not waive correctness, coverage, mandatory
native bounds or the optional scale transform's explicit zero-growth budget.

| Required check | Final result |
|---|---|
| `perl scripts/cppgm_file_audit.pl --stage pa25 --paths dev/src` | Pass, four inherited header warnings |
| `make test-pa25` | 101/101 |
| `make test-report-through-pa25` | 4253/4253, all 25 stages |
| Personal controls | 178 binding/layout and 15 adapter/version checks; 74 exception, 67 class, 61 driver, 19 scalar + 96 full-width pairs, 122 audit138, 49 statement; runtime/API/MIR/roundtrip and source reducers pass |
| Preservation | `git diff --check` passes; fixtures, harnesses, references and comparison rules unchanged |

The authoritative root count is **4253**, including prior-through24 **4152** plus
PA25 **101**. The supplied state said 4416; the initial primary log and independent
reports agree on 4253. No cases were removed. The four inherited file-audit warnings
are substantial header bodies in `lowering/procedural.h`, `lowir/model.h`,
`semantic/analyzer.h` and `semantic/model.h`; fileAudit itself passes.

The inherited unused-dependent-local item was independently rechecked: N3485
[temp.dep.type]/5–6 makes its missing current-instantiation member ill-formed even
without instantiating that function. The valid `T::missing` control executes without
demanding the unused body. [The proof](../student.tests/pa25/deferred-member-proof140.md)
closes that item; the earlier floating reference remains valid under [expr]/12.
No reference correction was made. Bundle
`cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`, is unchanged.

## Consolidated ledger

| Boundary | Review and disposition |
|---|---|
| 135–137 implementation / audit138 | Entire original stage range independently revisited, including driver, wide scalars, ABI facts and statement lifetimes. Original audit and measurements preserved; its 74/101 incompleteness is historical. |
| implementation139: `7dfdc6fc`, `4ecb9b41` | Previously unaudited class runtime/static destination/vptr/RTTI work reviewed through all owners; behavior and fixed class/cast/allocation measurements checked. Accepted with this audit's exact global-layout repair. |
| implementation140: `c75e9946`, `17f7f6b4`, `5bf12aef`, `9655db73`, `e7f66a25`, `e6cdc480` | Previously unaudited source EH, function-try ownership, nullptr/member representations, evaluation precision, scale proof and records reviewed together. Accepted with binding/temporary/qualification repairs. |
| audit141: `67ea755a273223b7fcc973215e0ca1ec8131689d` | Whole-stage ownership repairs, expanded reducers, actual source-to-ELF/MIR traces, frozen performance, required stage/root/file checks complete. No unaudited implementation handoff or identified PA25 blocker remains. |

Raw evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa25-141/`.
The final records commit follows the reviewed code commit; it changes no compiler
behavior. [Plan](plan.md) is the compact final disposition, not a second open queue.
