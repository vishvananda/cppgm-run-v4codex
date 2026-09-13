# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original **84/314**, current **314/314**: all 230 original failures resolved,
with unchanged fixtures, references and comparison rules. PA15 has not started.

## Design/spec alignment

| Owner / data flow | Complexity and validation |
| --- | --- |
| Inherited source facts, signatures, defaults/initializers and enum identities | Retained source IDs → immutable frames → concrete facts. Source/key/use bounds and historical evidence remain intact. |
| Specialization definition, layout and member bodies | Independent active/success/failure states; terminal failure at the narrow owner, including the entire selected-definition interval. Nine public API controls repeat failed requests without graph growth. |
| Key-definition availability → owning class | The selected key has an O(1) reverse edge through its member owner. Publish one notification, including definitions discovered during finish; source availability does not imply a checked body. |
| Lifecycle use → vtable → slot bodies/deallocation | Typed key/constructor/destructor reasons; independent four-state vtable demand. Visit each immutable slot ID once without copying the slot vector. A later body failure leaves completed vtable dependency registration valid. |
| Completed vtable demand → ABI emission | Publish one class ID; sort only emitted IDs. Lowering caches use class/member identities and deleting-entry IDs, including final lifecycle ordering. No virtual scheduling/emission scan of unrelated entities. |

The virtual group includes reverse definition order, lifecycle recursion, local
classes introduced by template bodies, late key publication, unused dependent
virtual bodies, terminal vtable failure, and independent later body failure.
Six public controls make 10,000 completion requests each under release and
sanitizers. Native controls cover dispatch, destructor effects and heap deletion.
The late-key case preserves the existing PA13 publication policy; it does not
claim that C++ requires every template key body to emit a vtable.

## Remaining groups and concrete boundary

The audited specialization failure intervals and virtual scheduling/ABI consumer
group are complete. Remaining required work includes typed demand and structured
expected rejection across defaults, destruction/exception specifications, member
bodies and general emission, plus the in-scope retained type/query audit.

The next boundary is semantic fact production in `destructor_exception.cpp`,
constructor/destructor action preparation and default-argument occurrences.
`actions_ready` shares constructor/destructor storage; exception-state bytes also
encode truth. Definitions may appear after a type-only query, so replacing these
records requires separate insertion/completeness/cycle keys and an audit of
both overload queries and actual lifecycle execution. The virtual dependency
edges do not supply those keys. Extending the completed ABI list changes into
these owners without that independent audit would conflate declaration, value
and body demand. There is no external blocker.

Scope correction remains: PA12 excludes member pointers; PA13–PA14 do not add
them. The archived exploration imposes no stage gate. The new pure-virtual
personal source remains an IR control because the supplied native backend cannot
resolve its support symbol; course pure-virtual coverage and comparisons remain
unchanged, and its concrete-base companion executes natively.

## Performance acceptance

**14,504 observations**: 14,112 preserved plus 308 main and 84 repeat samples.
Fifteen A/B LowIR hashes and seven executable hashes match exactly. Work controls
vary unrelated aliases N, classes K, slots S and evaluated uses Q. Notifications
and emissions equal K; slot demand equals K×S. N=16,000→64,000 and Q=1→4,000 leave
one-class queue/cache storage at **8/56 bytes**. Large-alias median peak RSS drops
**1064 KiB**, repeated at **1074 KiB**. Compiler text grows **0.276%**; Analyzer
**6160→6216**, Procedural **1456→1472**, VirtualClass stays **64**; hot records stay
unchanged. Twenty-four live transitive headers are frozen and checked.

The initial retained-body latency increase **2.34%** repeated at **0.36%**, with
mixed paired blocks. Other affected compiler medians are small and mixed; no
broad latency or runtime benefit is claimed. Every observation and unisolated
outlier remains in [performance.md](performance.md). No timing ran alongside
builds, tests, proofs, layout probes or verifiers.

Budgets: at most one key notification and emission ID per class, one visit per
slot, geometric class/member/use storage, final ordering by emitted IDs, four
O0 conversion variants, one writable Fact view and zero generated growth on
comparable correct outputs. PA14/O0 mandates no numerical latency/RSS/text ceiling.
Repeating all 54 historical timing inputs remains a diagnostic choice, not an
exit gate. All historical evidence and correctness coverage are preserved.

## Handoff ledger

| Increment | Commit / evidence |
| --- | --- |
| Inherited source ownership and terminal failure intervals | Through `c1e17cdc`; 14,112 preserved observations |
| Explicit virtual dependency publication and terminal state | `8d596c53`; key/lifecycle/slot edges and public controls |
| Class/member ABI caches and complete deleting-entry consumers | `9ec76f55`; native/lifecycle ordering, body-failure separation |
| Validation, layouts and performance acceptance | `virtual-demand-*.json`; 392 new observations and frozen A/B binaries |

Validation: **314 stage / 1621 prior / 1935 through**, **35 native programs**,
**349 sanitizer and entry/current parity inputs**, inherited rejections/ABI
controls, twelve new public virtual-state runs and eighteen inherited failure
runs. Owning PA13 native, semantic, lifecycle, linkage and audit controls pass
under both compilers. File audit passes with three inherited header advisories;
all **1266 fixture/reference hashes** are unchanged. Artifacts:
`$RALPH_ARTIFACT_DIR/pa14-virtual-demands/`. Deduplication preserved every archived
path/hash and recovered 5.16 GB of duplicate LowIR storage. Full-stage architecture
remains open; this handoff completes the connected virtual-demand behavior group.
