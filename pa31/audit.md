# PA31 final whole-stage audit206

Stage base: `c0566ded7ed123e1eb0696f0ae93b28ceb492fdf`.
Last reviewed commit: `6039c065709a32a26d74f4758e235150d621e0ae`.
Reviewed implementation: `ffea90db`, `d5bd5aee`.
Target: **PA31 full-stage**. Phase: **final audit complete**.

The previous goal turn made progress: it committed the implementation and its
validation evidence. No inherited process was live at this audit's entry. The
four stage commits, current spec/handout, all five changed implementation files
and the cumulative production owners below were read independently. The prior
checkpoint located evidence; its conclusions did not replace source review.

## Findings and changes

No additional compiler correctness or architecture defect was found in the
reviewed PA31 surface. The five implementation changes follow their shared
owners, without a hosted-only code-generation path:

- Hosted new/new[]/delete/delete[] declarations retain separate externally
  owned ABI identities. Standalone allocation-role adapters remain standalone.
  Both source-generated clients use the host-provided replacement allocator.
- The completed virtual-base count prevents incorrect complete/base entry
  aliases. Separate C1/C2 and D1/D2 bodies use their recorded VTT and subobject
  facts; the multi-TU control exercises a different derived-object layout.
- A list constructor's same-brace input is evaluated through its selected
  backing-array conversion, avoiding the incoming-conversion cycle. Element
  expressions, nested conversions, defaults and temporary destruction still
  contribute to the canonical exception fact.
- Inherited zero-argument candidates remain available when a local constructor
  suppresses the implicit default. `using T::T` is constructor syntax; ordinary
  parameter shadowing still rejects. Inherited forwarding uses retained source
  declarations and typed argument recipes.
- Constructor validity includes inherited constructors' other bases and members,
  checking deletion/access/destruction without demanding their bodies. Complete
  per-constructor facts are memoized; unavailable class facts remain retryable.

The audit adds three explicitly run controls: a combined hosted/template runtime,
nested initializer lists with partial-construction cleanup, and inherited
default arguments with another base. The source, direct objects, public LowIR,
rebuilt objects and actual hosted MIR are checked together by
[audit206.py](../student.tests/pa31/audit206.py). Existing tests, references,
comparison rules, timeout settings and implementation sources are unchanged.

An exploratory host cross-check disagreed with the inherited default-argument
`is_nothrow_constructible` assertion. The control retains the required result:
`next()` can throw while providing the omitted argument. C++11
[expr.unary.noexcept]/3 and [meta.unary.prop], Table 49
([local draft](../doc/n3485.txt)) require that potentially throwing evaluation
to count; [P0136R1, class.inhctor.init](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0136r1.html)
treats inherited initialization and argument evaluation as one call. Its
[C++11 DR lineage](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4429.html)
is the hosted inheritance interpretation already used by implementation205.
The student control catches the default argument's exception and checks that an
explicit argument skips it. Host compiler agreement is not its oracle; no
reference output was changed for this observation.

The supplied status count was stale: its primary log and the fresh root report
both say **5,178/5,178**, including **84/84 PA31 fixtures and 31/31 stages**.
There is no removed test. The compact plan now records the audited design,
stage-scoped acceptance and complete handoff ledger.

## Independent Spec Alignment

| Spec surface | Reconstructed ownership and evidence |
|---|---|
| §1 immutable sources / streaming / one parse | `preprocess/preprocessor.h` owns immutable source buffers, interned identifiers and slab-backed expansion tasks; nested arguments borrow captured slices. `PostTokenCursor` and the ring in `syntax/cursor.h` discard consumed tokens. `Parser::translation_unit(&sem)` publishes into the shared graph. Retained template source regions and `(source, context)` occurrences in `syntax/occurrence.cpp` do not re-enter grammar. Source ambiguity resolution is forbidden after publication. |
| §2 canonical facts | Entity, scope, type, query, argument-pack, specialization and ABI graph IDs are primary keys. `IdIndex` is open-addressed; `FactStore` allocates 1,024-record slabs, not per-node ownership. `abi_types`/`abi_scopes` cache canonical source identities. Rendered names are final ABI output, never semantic lookup keys. |
| §3 relevant lookup / complete candidates | Scope/name and signature indexes, explicit using/base/friend/ADL edges and candidate slices restrict work to required relationships. Arity/template/member filters precede expensive conversion work. Selected declarations and conversions persist. `candidate_substitution.cpp` uses typed active keys and compact rejection, including holes before defaults; it does not publish a negative cache across later defaults or class completion. |
| §4 templates / monotonic demand | `specialize_class` interns `(pattern, argument pack)`; the canonical pattern owns its lexical environment, while nested/member patterns carry enclosing specialization identity. Source heads and parent-linked substitution frames retain context. Declaration, body, layout, exception, forwarding and emission facts have distinct states. `instantiate_function` demands retained regions; fixed semantic expressions are shared and dependent facts are substituted. Invalid dormant member bodies remain undemanded in the combined control. |
| §5 scheduling / invalidation | `Analyzer::finish` drains separate cursored queues for selected function uses, vtables, storage, specializations, friends, members and exceptions. `query_dependencies.cpp` records class/query reverse edges; completion invalidates only affected consumers, including their value/exception facts and local revisions. Constructor validity and forwarding use active/success/failure states, with unavailable prerequisites returning to not-started. No global cache epoch or whole-registry retry was introduced. |
| §6 typed lowering / one emission identity | `toolchain/driver.cpp` → `lowering::build_program` → `Procedural::run` consumes semantic call, construction, cleanup, layout and ABI facts directly. Entity-indexed symbol/base/deleting tables identify distinct entries. Hosted ownership, inline flags, aliases, object spellings and runtime facts are ordinary LowIR metadata. The writer/reader are explicit inspection adapters; ordinary `-c` neither serializes nor reparses. |
| §7 native work / encoding | `compile_object` invokes extended-float legalization and `compile_image`; forced-call preparation runs once per Program. `object_demand` visits typed roots and symbol edges once, then the selector builds one function's MIR, placement, frame and unwind facts. `Encoder` writes bytes directly; `HostElf` moves native buffers into ELF sections and writes named relocations/COMDAT/unwind records. No assembler is invoked. |
| §8 lifetimes | Source, identifiers, source graph, fact slabs and semantic indexes die at each `build_program` TU boundary. Program pools retain the minimal typed bodies/facts needed by bounded forced-call preparation and object closure. Expansion replaces original pools, and per-function selection/placement/MIR is released before the next function. Object/ELF byte buffers move between owners. Mutable caches belong to a TU, Program, encoder or function; there is no process-global accumulated semantic state. |
| §9 work / performance | The reconstructed passes are linear or linear plus sorting in consumed/generated records, apart from explicit language-required candidate comparisons and bounded expansion. Existing work/growth limits and conservative fallbacks remain intact. All four performance dimensions, fixed workload outputs, frozen images, A/A, ABBA and spreads are reviewed in [performance206](performance206.md). |
| §10 self-containment | Compiler output follows the compiler-owned path above. Host macro/include configuration supplies the allowed environment, and the host driver only links emitted objects in runtime controls. No reference compiler, fixture identity, cached answer or host-codegen delegation exists on that path. `std::initializer_list` recognition is the language-mandated declaration identity, not a library-private shortcut. |

The ABI audit also followed `lowering/symbols.cpp` into
`abi/itanium/{graph,encoder,function_encoder}.cpp`: standard substitutions have
typed graph nodes and do not enter the numbered table. One Encoder preserves
that table across the function template arguments and bare function type.
Inline namespaces, ABI tags, owner scopes and local contexts remain semantic
inputs. External allocation names and complete/base aliases use this same path.

## Representative data and optimization trace

[interactions.cpp](../student.tests/pa31/source206/interactions.cpp) demands
`Pipeline<int>::run` and `Inherited<Base>`, while retaining an invalid unused
member template. Runtime input feeds list element construction, vector storage,
array allocation, a `std::function` call, stream output and destructor cleanup.
The second vector throws while constructing its backing array; the live-object
counter must return to zero. Two runtime inputs check the direct and rebuilt
objects. Existing multi-TU controls separately check complete/base ABI entries
and externally replaced array allocation.

The combined trace consumes **197,275 tokens**, with **7,274 maximum pending**,
**5,039 semantic specializations**, **650 retained template source regions**,
**1,300 reused expression facts**, and **569/569 member demands processed**.
It has **9,049 original / 9,789 prepared LowIR instructions**, **6,935 native
instructions**, and **37,961 text bytes**. The trace verifies telemetry on/off
identity, matching code and named code relocations, matching function-associated
CFI, and absence of the dormant member symbol.

`Pipeline<int>::twice` carries semantic `force_inline` and nonthrowing facts into
LowIR. The expander checks a direct target, fixed ABI, absence of unsupported
dynamic stack/varargs, recursion and budget before mutation. Admission reserves
operand/instruction/slot/return-cleanup work. It permits at most **262,144 units
per function, 4,194,304 per Program and depth 64**; a declined request remains a
valid call. This is required attribute expansion, not a new optional optimizer
or a claimed speedup. The whole trace uses **130 expansions, 2,405 actual /
4,847 reserved work units**, with **400 maximum per-function reserved units**.

The original immutable bodies own eligibility/cost facts. Expanded bodies get
fresh selection/placement facts, so old liveness is never reused. The emitted
`step` MIR contains multiply/add and no call to `twice`; its frame is **16 bytes**.
The noinline `step` remains addressable through `std::function`. `run` has a
**672-byte frame**, preserving rbx/r12 and exception slots. Loop loads, stores,
indirect calls and cleanup remain real executable work. Selection folds only
legal adjacent nonvolatile loads/addresses, extends carrier intervals before
placement, and honors call epochs/clobbers. Parameter availability is a dirty
worklist with at most six clobber-bit additions per edge. Integer/float/volatile,
ABI and unwind constraints are not traded for instruction count.

## References, validation and ledger

The existing [global relocation correction](reference-correction205.md) was
independently checked against both reduced programs, C++11 global scope/linkage,
the handout's hosted ABI requirement and
[Itanium §5.1.2](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-structure).
Global namespace `g` has the bare ABI spelling. Only four expectation sidecars
differ from stage base, replacing `_Z1g` with `g`; required PC32/GOTPCREL classes,
counts, forbidden relocations, source inputs and comparison rules are unchanged.
The pinned bundle revision/hash in the proof match the current manifest.

Final command results, hashes, evidence verification and review boundaries are
recorded in [evidence206](../student.tests/pa31/evidence206/validation.json).
The file audit's four substantial-header organization warnings are inherited;
their implementations were inspected and no audit errors occur.

- `perl scripts/cppgm_file_audit.pl --stage pa31 --paths dev/src`: **pass**.
- `make test-pa31`: **84/84**, exit 0.
- `make test-report-through-pa31`: **5,178/5,178, 31/31 stages**, exit 0.
- **46** inherited control commands and **104** hosted trace commands pass.
- Final evidence verification checks current sources, preserved references,
  binary/object identities, all **824** workload observations and **48** launcher
  observations, and independently recomputes the performance summaries.

| Handoff | Disposition |
|---|---|
| PA30 final / `c0566ded` | Inherited frontend, templates, value/lifetime, ABI, vector and native owners reviewed on the actual PA31 source path; earlier stages rerun. |
| PA31 entry / `a93350a8` | Six entry failures and 84-fixture inventory reconciled with unchanged course coverage. |
| Implementation205 / `ffea90db` | Allocation ownership, virtual-base aliases, list exception recursion, inherited zero-arity/using syntax and relocation proof reviewed end to end. |
| Semantic probe follow-up / `d5bd5aee` | Previously unaudited inherited-subobject validity reviewed with default/parameterized deletion, access, reference and exception controls. |
| Implementation handoff / `6039c065` | All saved source/binary/image bindings and performance observations checked; entire production architecture independently reconstructed. |
| Final audit206 | Three new controls and hosted LowIR/MIR/ELF trace added, performance reconfirmed, status counts reconciled, compact plan and final records consolidated. No compiler repair or new reference correction needed. |

No unaudited handoff or known required PA31 work remains. PA32/33 optimizer-level
acceptance and PA34 inception are later-stage requirements; they do not create
extra PA31 gates. This audit does not advance to PA32.
