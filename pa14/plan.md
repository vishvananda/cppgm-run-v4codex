# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and course comparisons
are unchanged. PA15 has not started. Current implementation: `af01c062`.

## Design/spec alignment

Active continuation from `97006205`; previous turn: **verified progress**.
Current group: publish checked ordinary/special function signatures and raw
parameter facts together. Source prototype queries retain parameter ordinals,
raw types and access contexts; substitute them per frame, then let concrete
declaration/default/exception/transfer/virtual owners publish their own facts.
Measure source/signature/parameter publication scaling with N/K/Q and preserve
raw cv/array/function types, trailing returns, nested declarators, overloads and
unused-body/default/exception demand. The preceding acceptance below is inherited.
Implementation now passes through 1935, native 32, six new rejection controls and
three signature reducers. Prototype names no longer republish source identities.
Late/nested source defaults bind on the enclosing class completion event under
separate queued/active/complete/failed states. Next freeze proofs, sanitizer parity
and source/signature/parameter/default scaling before performance acceptance.
The completed group extends source declarations through concrete local types,
query substitution, object/lifetime consumers and sparse stable fact storage.
A source declaration owns its pattern EntityId; a substitution frame publishes
its concrete EntityId before use. Consumers no longer recover that decision
from projected syntax. Local classes/enums retain symbolic Named type identity.
The direct-initializer correction keeps bounded declaration lookahead and
preserves real function declarations. No source region is reparsed.

| Owner / data flow | Complexity and validation |
| --- | --- |
| Declaration publication (completed) | Source declaration → canonical pattern → per-frame concrete binding → type/query/object/lifetime use. For N specializations and K local groups: source work 8K+1, publications N(8K+1), type substitutions N(4K+1), query work K(3N+4), independent of repeated uses Q. Local/nested/shadowed identities, aliases, enum bounds, copies and cleanup execute correctly. |
| Sparse facts (completed) | Four-byte optional index per syntax/occurrence; only explicit publication allocates a twenty-byte Fact. TU-owned 1024-record slabs preserve references across growth and release in bulk. Absent reads allocate nothing; one local writable view per grouped publication. Standalone release/sanitizer storage control and full semantic parity verify consumers. |
| Definition, expression, region/default and value owners (inherited) | Checked source signatures → selected definitions → concrete member/body/lifetime facts. Immutable regions, parent-linked frames, canonical queries and conversion sharing retain their prior equations, proofs and measurements. |
| Remaining declaration/parameter graph | Ordinary member signature reconstruction still establishes prototype scopes and raw parameter facts; some embedded/class-scope types and queries lack source facts. Joint declaration/type-query/scope/object/lifetime ownership must precede eliminating these rechecks. Whole-region occurrence IDs and their optional indices remain. |
| Demand/failure dependencies (remaining) | Finish typed reasons, reverse edges and independent declaration/definition/layout/default/exception/body/vtable/emission states. Memoize structured expected failure per complete key; enqueue only affected consumers. |

**Concrete boundary:** the source-to-concrete local declaration binding and sparse
Fact consumers are complete. Further signature reuse crosses `declarator`'s
prototype scopes, parameter adjustment and trailing-return queries,
`instantiate_parameters`' raw body types, and `declare_object`'s default,
exception, transfer and virtual facts. A Function TypeId alone cannot replace
those publications. Remaining class-scope enum/embedded queries also need source
owners before reuse. This requires a joint producer/consumer change and new
parameter/context scaling controls, followed by typed demand-state work; merely
removing more Fact slots would lose semantic decisions. This continuation
extended through local identities, the exposed parser failure, all fact writes
and grouped publication views. There is no external blocker.

## Performance evidence and budgets

**12,838 observations verified**: 11,914 inherited, 84 from two isolated trials and
840 from the full 49-input/eleven-executable campaign. All four local N/K/Q cases
improve median compiler latency **4.45% / 4.41% / 7.45% / 9.19%**, each in both
ABBA blocks. Peak RSS changes **−3,192 / −64,882 / +16,308 / −107,906 KiB**;
the 4000-specialization increase remains a disclosed peak-memory cost, whose
exact native allocation cause is unisolated. All 49 LowIR and eleven native
hashes match exactly; generated growth is **zero**, with no runtime optimization
claim. [performance.md](performance.md) retains every median, spread and cost.

Compiler text grows **6,080 bytes (0.4636%)**. Entity/Expression/ObjectUse remain
112/36/36; Fact is 20, FactStore 56 and Analyzer grows 5920→6000. The live probe
covers 18 transitive headers; earlier probes preserve frozen snapshot integrity.
Inherited latency increases include calls-4 +1.04%, demand-wide +0.19% (mixed
pairs) and special-wide +2.05%; the latter saves 14,900 KiB RSS. Preliminary trials,
large timing outliers, native allocation variation and all historical RSS costs
and separate Massif evidence remain disclosed. No whole-corpus speedup is claimed.

Budgets remain source/key/concrete-use-proportional storage, at most four O0
conversion variants/source operation, one local view per publication and zero
generated growth. O0 has no mandated numerical latency/RSS/compiler-text ceiling.
Repeatable affected-case benefits justify the bounded facts, sparse indirection
and compiler growth. Unsupported historical live-header gates are snapshot
checks; no mandated limit, correctness or coverage was removed. Performance
acceptance leaves the remaining architecture group open.

## Handoff ledger

| Coherent increment | Commit / evidence |
| --- | --- |
| Earlier declaration/frame/default/value/expression and ordinary/special definition owners | `78bdbc3f` through `fda0a178`; 11,914 verified observations, proofs and heap diagnostics retained |
| Concrete declaration publication, local types and direct initialization | `58ead665`; distinct local/nested/shadowed types, aliases, enum bounds, copies and cleanup |
| Stable sparse Fact storage and telemetry | `15b8ac6d`; all 137 writes publish explicitly, absent reads allocate nothing |
| One writable view per grouped publication | `af01c062`; reviewed conditional scope, latency/text follow-up; both trial campaigns retained |
| Frozen proofs, validation, layouts and harnesses | `facf8d64`; two positive reducers, 18-header live probe and 345 sanitizer inputs |
| Full performance acceptance | `declaration-fact-performance.json` / `declaration-fact-handoff.json`; 924 new observations and cumulative verifier pass |

Validation: **314/314 stage**, **1621/1621 prior**, **1935/1935 through**, **31**
native programs, **345** release/ASan/UBSan parity sources, **157** rejections,
**six** ABI controls, **seven** inherited reducers and initializer/store/lifetime
controls (**92** recorded checks). File audit passes with three inherited header
advisories. Coverage retains all **1266** fixture/reference files. Initial failures,
patches, logs, binaries and outputs remain under
`$RALPH_ARTIFACT_DIR/pa14-declaration-facts/`; evidence/verifiers are in
`student.tests/pa14/`. Full-stage work remains incomplete at the joint
signature/parameter/query and typed demand/failure boundary above.
