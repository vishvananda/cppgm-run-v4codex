# PA25 accumulated checkpoint audit138

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: 4530fe939e95a45ff6a46d5e53c76807b5bdd352

Target: **PA25 full-stage**, phase **checkpointAudit**. This first audit reviews
`fee6ad9076ff35c5272526e1c4c4df235fbf3bfe..4530fe939e95a45ff6a46d5e53c76807b5bdd352`,
including all changes through entry `34fe77a31e8bc73d46e425a48266b7537f21d773`
and the audit repair. Both entry markers named the stage base; the range was not
narrowed to the latest handoff. All 12 original commits and their combined source
changes were reviewed, including driver/scalar/statement interactions. The final
combined inventory has 84 changed `dev/` paths, recorded with commit inventories
in [validation138](../student.tests/pa25/validation138.json).

Disposition: **checkpoint audit complete; PA25 incomplete**. Entry and exit are
74/101 with exactly the same 27 failing cases. The three accepted implementation
checkpoints are preserved. The code repair is committed separately before these
records, making the marker above the next review's unambiguous baseline.

## Complete commit review

| Commit | Reviewed contribution and disposition |
|---|---|
| `b6eae734` | Initial stage boundary/ownership plan; marker and unfinished requirements retained |
| `ce08119c` | Direct typed source/native driver, objects, foreign ELF and linking; repaired producer definition ownership and alias/GOT interactions |
| `ff3e3848` | Foreign alignment and driver measurements; retained layout, strengthened full-width alignment/extent validation, preserved all measurements |
| `cf26b48b` | Driver validation and 64/101 handoff; checked coverage and remaining owner groups |
| `da31a876` | Wide scalar plan; finite required work retained, inherited diagnostic targets do not add gates |
| `1cc95b47` | Canonical wide source types, constants and lowering; repaired enum range/payload and floating narrowing interactions |
| `bcdd0b72` | Scalar builtins and ABI adapters; repaired enum sign/type identity, native builtin metadata and floating semantic views |
| `ba608c4f` | Constant-owner telemetry and precision discussion; counters observe existing work, reference proof does not justify an oracle change |
| `11aee2da` | Scalar validation and 71/101 handoff; reviewed full-width controls and performance manifests |
| `832d2d85` | Statement ownership plan; retained review boundary and remaining stage scope |
| `2705e57b` | Statement values, template/query/control/lifetime facts; traced combined wide/template statements and normal/early cleanup to ELF |
| `34fe77a3` | Statement validation and 74/101 handoff; verified unchanged failure inventory and historical measurements |
| `4530fe93` | Cohesive audit repair across semantic/ABI/native owners, plus explicit personal controls; validated before recording this tip |

## Findings and repairs

**Scalar representation and ABI identity.** Non-fixed enums could shrink their
underlying type according to a later small enumerator, keep narrow payloads under
a wide type tag, or lose the required pre-closing-brace enumerator type. Implicit
increments and fixed-range values needed checked bounds. The enum owner now
tracks the full signed/unsigned range, selects a type representing every value,
checks increments, and converts all constants to the selected representation
before assigning enum type. This prevents narrow integers being read as wide
constant-pool IDs. C++11 [dcl.enum]/5–6 in [N3485](../doc/n3485.txt) specifies
initializer/preceding-enumerator types and representation of all enumerators.

Integer-to-floating list initialization compared values already rounded to long
double, accepting some non-exact conversions. It now checks exact integer
roundtrip, as required by [dcl.init.list]/7. Non-finite floating-to-integer
conversion is rejected before a host cast: [conv.fpint]/1 makes out-of-range
conversion undefined, and [expr.const]/2 excludes undefined operations from core
constant expressions. Floating builtin constant views now use the floating pool,
not an integer pool interpretation. Reduced programs cover NaN, float/double/
long-double boundaries, enum widening/sign/range/type/increment and typed views.

Wide enum template arguments exposed a cross-handoff hole: source constants
reached an ABI graph that accepted wide values only for builtin integer types.
Unsigned 64-bit enum values could also acquire a negative spelling. Canonical
wide ABI facts now preserve declared enum type and separate numeric sign; the
reader, writer and encoder agree. Small named values keep their existing key.
[Itanium ABI section 5.1.6.1](../doc/itanium-mangling.txt) requires an enum literal's
declared type plus numeric value of its base integral type. Positive/negative
128-bit and unsigned-64 enum controls check exact manglings, fact/API roundtrips,
direct/separate compilation and linkage with independently built helper objects.
Host agreement supplements that ABI rule; it is not the proof by itself.

**Native definition ownership.** Link-time nearest-address reconstruction could
lose relocations required by a retained alias when a weak sibling was replaced.
Conversely, a synthetic GOT slot for a discarded weak body could demand an
otherwise unused undefined target. Native code/data producers now attach a
compact definition ID to each fixup. Aliases share that identity. Compiler object
version 2 persists and validates definition/lazy/owner facts; old version-1
objects must be rebuilt. This is an internal-format revision, not the later
host-compatible object-output milestone.

The explicit foreign ELF adapter decodes and sorts bounded function/object
extents once, coalesces same-address aliases, and supplies the same typed facts.
It validates alignment before narrowing and checks symbol extents. Synthetic GOT
slots are lazy definitions. Linker symbol selection roots the selected definitions;
a flat adjacency and deduplicated worklist visit each retained definition and
edge at most once. A retained alias keeps its dependencies, while unused slots
of discarded bodies remain undemanded. Section-owned relocations are retained.
Dead object bytes are not removed; this is necessary relocation selection, not
an optional code-size optimization. Controls cover ordinary PLT and `-fno-plt`,
both object orders, retained aliases, shared slots and required unresolved targets.

**Builtin facts.** The native driver recovered `strlen` identity from an object
name. The source path now carries the semantic builtin through a typed LowIR
metadata field. Only the external LowIR reader decodes the legacy textual form.
The field uses existing padding: symbol metadata remains 36 bytes; ABI nodes
remain 32 bytes. Direct-source and explicit-adapter execution both pass.

The committed [audit controls](../student.tests/pa25/audit138.py) contain the
reducers. Initial controls exposed 26 failing checks among 70 attempted checks
against the frozen entry binary (some follow-ups require successful compilation).
The expanded final suite passes 122/122. All intermediate failures and generated
reducers remain under the artifact directory; no course tests were altered.

## Architecture and optimization trace

Read against [spec](../spec.md), [assignment](README.md),
[testing policy](../TESTING_AND_REFERENCES.md) and [layout](../PROJECT_LAYOUT.md).
The nontrivial lifetime declaration in `student.tests/pa25/statement-trace.cc`
and the demanded template in the audit's `template-statement-wide.cc` were
traced from source to final ELF, including explicit AST, validated LowIR and MIR
inspection and execution with an empty PATH. All 16 final-binary trace checks pass.

| Boundary | Owner, identity, work and release evidence |
|---|---|
| Source/parser | Streaming cursor and immutable parsed source; each deferred source region parsed once and cached; occurrence views keyed by `(source, context)`, no cloned grammar or replay |
| Template/semantic | Canonical entity/type/constant IDs, parent-linked substitution frames and complete `(owner, id)` query keys; fixed definition facts reused, only dependent facts instantiated; demanded body transitions once per specialization |
| Result/lifetime | Selected statement result conversion and constructor/destructor facts feed typed lowering; function-owned `(overlay, state)` mappings share unchanged prefixes; skipped-initialization guards follow recorded control/lifetime edges |
| LowIR/native | Semantic scopes/source graphs end at the TU boundary; direct typed symbols, ABI values and runtime roles cross it. One function's MIR/selection/frame/encoding temporaries are released before the next; explicit text views do not feed source production |
| Object/ELF | Compact bytes, symbols, aliases and producer-owned fixups outlive frontend/MIR; O(symbols + relocations) linker worklist, one O(symbols log symbols) foreign-extent sort; buffers released with the driver invocation |

The lifetime trace has 165 tokens, 273 parsed nodes, 473 occurrence-view nodes,
two specializations/body transitions and two parsed template source regions.
Eight fixed expressions have 16 uses. Four lowered statement regions need two
lifetime mappings with 22 hits; five native functions produce 158 instructions
and 1581 text bytes. Linker visits: nine definitions, 53 relocation edges.
The wide/template/statement composition has 126 parsed nodes, two body
transitions, six fixed expressions with 12 uses, two statement regions, three
functions/109 native instructions, 559 text bytes and four definition/two edge
visits. Normal/early destruction order is checked, not merely inspected.

Useful optimization fact: semantic result category, same class type and selected
constructor facts allow the existing C++11 destination copy-elision path to use
the final destination. Legality comes from the recorded construction/conversion
and lifetime facts; profitability is avoiding a temporary/copy. Ineligible
results retain ordinary conversions/copies and cleanup. Function-owned lifetime
state is reset between functions, so mappings cannot survive invalidation of
their owner. This extension adds no iterative optimization or speculative growth;
work follows actual regions, lifetime edges and emitted instructions. Final MIR
and execution preserve return/control behavior, ABI calls and debug associations.
Existing PA24 native bounds and contract checks still pass. No allocator-specific
or later optimization-policy requirement is invented at PA25.

The reviewed production path has no unexplained text transport, global retry,
repeated specialization construction or new per-node hot allocation. The name
and address reconstruction found by this audit was repaired in its producers
and explicit adapters, rather than hidden by a fallback in the consumer.

## Performance and validation

[Performance138](../student.tests/pa25/performance138.md) reports compiler latency,
peak memory, checked executable runtime and text together. It pins exact A/B
binaries, flags and inputs, A/A calibration and six ABBA blocks per mode/workload.
All 1624 observations and historical outliers remain; all 11 manifests rehash
correctly. The exact final template compiler median is 0.32798 s, paired B/A
1.061 [0.794, 1.304], peak 14900 KiB. The cumulative packed predecessor versus
the first correct driver has median 1.028 [0.864, 1.153]. These costs and spread
are disclosed. Equivalent executables are byte-identical: 384567/521/340 text
bytes for template/memory/floating workloads. No speedup is claimed.

Stage acceptance follows spec section 9, including inherited plans: unsupported
15% latency/RSS and zero optional text-growth targets are diagnostics, not extra
exit gates. Necessary semantic/ABI work is bounded; avoidable common-node growth
was removed. There is no new optional transform whose cost lacks a demonstrated
benefit. Wide/statement final/final baselines remain available; PA34 owns
self-hosting. This does not relax mandated correctness, coverage or native bounds.

| Check | Final result |
|---|---|
| `make test-pa25` | Exit 2; 74/101, exactly the original 27 failures |
| Required `n=25` conditional through report | Exit 0; PA1–PA24 4152/4152, all 24 stages pass |
| `perl scripts/cppgm_file_audit.pl --stage pa25 --paths dev/src` | Pass; four inherited header warnings |
| Stage progress / coverage | Pass; zero new failures, zero removed cases, all 101 anchors preserved |
| Explicit controls | 122 audit, 61 driver, 19 scalar + 96 randomized operand pairs, 49 statement, 4 ABI/API, 16 trace checks pass |
| Source/fixture checks | `git diff --check` passes; no fixture, harness, reference or comparison changes |

Exact commands, exit codes, source/fixture inventories, binary and evidence hashes
are in validation138. Driver/scalar/statement and ABI controls precede only the
last semantic-view correction; required suites, audit controls and traces use the
exact final binary. The four file-audit warnings are inherited substantial header
bodies in lowering/procedural.h, lowir/model.h, semantic/analyzer.h and
semantic/model.h. No file-audit requirement is waived.

## Remaining scope and reference preservation

Group the next implementation work broadly: native class/runtime support plus
source EH integration (12 + 14 required failures), then source-semantic completion
(the floating oracle and inherited unused-dependent-local reducer). Runtime
hierarchy, payload, cleanup and handler facts must connect through shared owners;
a handoff per support symbol or failing case would fragment their interactions.
The three prior checkpoints each contain substantial driver, scalar or statement
work, but separate planning, telemetry and evidence mini-commits caused avoidable
review fragmentation. Keep related fixes, interaction controls and validation
within one broad ownership handoff.

The five floating differences among one million lines remain required failures.
[The reduced precision experiment](../student.tests/pa25/rounding136.md) and
C++11 [expr]/12 permit both direct double and excess-precision evaluation, so
there is no proof that the reference is wrong. No reference output is corrected.
Bundle: `cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The exact comparison and all coverage remain. Full-stage success still requires
the stage and root through-PA25 reports to pass; this audit grants no advancement.

## Audit ledger

| Audit | Reviewed range / accepted checkpoints | Findings and evidence | Exit / remaining work |
|---|---|---|---|
| 138 | `fee6ad90..4530fe93`; entry `34fe77a3`; 3 checkpoints | Fixed scalar/enum/ABI representation, definition/alias/GOT ownership, typed builtin and constant views; source-to-ELF trace; validation138/performance138 | Audit complete; prior 4152/4152, stage 74/101 with same 27 failures; native runtime/EH and source-semantic completion remain |

Raw evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa25-138/`.
The following records-only commit contains this audit, compact plan and evidence
summaries; its code parent is the `Last reviewed commit` above.
