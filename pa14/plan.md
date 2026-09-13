# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and course comparisons
are unchanged. PA15 has not started. Current implementation: `5b947a18`.

## Design/spec alignment

Active continuation from `fda0a178`; previous turn: **verified progress**.
Extend source declaration ownership through local class/enum identities, concrete
entity/scope publication, query substitution and object/lifetime consumers.
Source declarations publish canonical identity; per-frame overlays publish only
concrete bindings. Replace syntax-occurrence recovery at those consumers, then
reduce dense occurrence Fact storage where the same ownership permits it.
Validate distinct local types, self references, aliases, dependent enum/layout
queries, nested scopes, concrete objects and cleanup; freeze source/key/use
scaling and equivalent output before accepting compiler cost. Completed special
signature behavior and the preceding acceptance evidence remain inherited.

| Owner / data flow | Complexity and validation |
| --- | --- |
| Ordinary definition ownership (inherited, complete) | Source prototypes → signature index → selected definitions → concrete member/body facts. Source work K, applications/edges N, requests N(Q+1), hits NQ; required candidate work NKQ remains. Prior proofs/measurements retained. |
| Special signatures/injected heads/application (completed group) | Same prototype owner plus symbolic current types and declaring-head slices. For N specializations, K constructor overloads and Q uses: source work K+1, direct applications/edges 2N, requests 2N(Q+1), hits 2NQ; candidate work NKQ remains. Twenty newly rejected invalid definitions; two newly accepted native controls cover copy/move/assignment/conversion, nested U/V heads and late defaulted operations. |
| Expression, region/default and typed-value owners (inherited) | Immutable source topology → parent-linked frames → canonical queries → concrete uses/lifetimes and ABI facts. Source/instance equations, negative keys, conversion sharing and measurements remain verified. |
| Declaration/scope/object/lifetime graph (remaining) | Demanded regions still establish occurrence IDs and dense Fact slots. Establish joint local class/enum/type-query/declaration/scope/object identities, then eliminate fixed rechecks with cleanup and lowering consumers. |
| Demand/failure dependencies (remaining) | Finish typed reasons, reverse edges and separate declaration/definition/layout/default/exception/body/vtable/emission states. Memoize structured expected failures per complete key; enqueue only affected consumers. |

**Concrete boundary:** the completed class-scope signatures have source types and
known concrete members, including injected/current types and renamed nested
heads. The remaining local declaration/value-query cases lack that joint identity
contract: occurrence facts currently carry local scopes, objects, parameters and
cleanup decisions into lowering. Removing them alone would alias objects or lose
lifetime facts. Further related work requires changing these producers and
consumers together and extending typed demand states; another local signature
shortcut cannot establish those facts. The turn extended through all matched
special/defaulted definitions. There is no external blocker.

## Performance evidence and budgets

**11,914 observations** are verified: 11,158 inherited plus 756 new observations
on 44 compiler inputs/ten executables. Four constructor N/K/Q cases improve
30.30%, 57.12%, 31.11% and 7.06% in median latency, each in both ABBA blocks;
median peak RSS falls 24,668 / 404,456 / 102,970 / 11,772 KiB. Nested-head direct
application improves 6.31%/5.40% against the correct intermediate compiler, with
954/2,066 KiB less RSS; wider calibration ranges limit precision. Entry rejects
the valid nested alias case, so its rejection is not used as a timing baseline.

All 37 inherited outputs remain byte identical. Thirty-nine of 44 A/B LowIR
outputs match exactly; five constructor cases differ in local slot names but
match under the unchanged course canonicalizer and student-mode validator.
Initial byte-only/reference-order preflights were unsupported personal gates;
failed runs and harnesses remain. No comparison rule or fixture changed. All ten
native hashes match: generated growth is **zero**, with no runtime optimization
claim. [performance.md](performance.md) records every median, spread and cost.

Compiler text grows 960 bytes (0.0732%); measured layouts are unchanged, including
Entity/Expression/ObjectUse 112/36/36, properties/uses 24/20, Analyzer 5920,
TemplatePrototype/TemplateDefinition 24/32 and MemberFacts 124. Inherited latency
increases include large-body +2.78%, calls-4 +1.17% and input-128 +3.64%; the last
has mixed paired results, and large-body calibration exceeds its measured delta.
Body-large-8 RSS rises 3,622 KiB; exact native allocation cause remains unisolated.
Historical source-cache +6,714 KiB, calls-4 roughly +15 MiB and input-128 +15,206
KiB costs remain disclosed, with the preceding separate Massif evidence intact.

Explicit budgets remain source/key/concrete-use-proportional storage, at most
four O0 conversion variants/source operation, one local view/visit and zero
generated growth. O0 has no mandated numeric latency/RSS/text ceiling. Repeatable
affected-workload savings justify the bounded signature/parameter facts and
compiler growth. Current transitive probes check live headers; earlier probes
retain frozen snapshot integrity. No mandated limit, measurement or coverage
was removed. Performance acceptance leaves remaining architecture work open.

## Handoff ledger

| Coherent increment | Commit / evidence |
| --- | --- |
| Earlier declaration/frame/default/value and expression owners | `78bdbc3f` through `5ae726e0`; original proofs and measurements retained |
| Ordinary definition matching, head aliases and direct application | `c65dbfe6` through `60cf761c`; 11,158 verified observations and heap diagnostics |
| Typed special signatures and conversion targets | `bdbb04d4`; constructor/destructor/conversion checks replace nullary syntax matching |
| Injected types, declaring-head frames and direct special/defaulted application | `5b947a18`; two positive native controls and 21 rejection controls |
| Frozen proofs, validation and comparison harnesses | `f84029d0`; 17-header live probe, 344 sanitizer inputs, correct nested baseline |
| Full campaign and acceptance record | `special-signature-performance.json` / `special-signature-handoff.json`; 756 new observations, cumulative verifier passes |

Validation: stage **314/314**, prior **1621/1621**, through **1935/1935**, **30**
native programs, **344** release/ASan/UBSan parity sources, **157** rejection
controls, **six** ABI controls, **seven** native reducers and store/lifetime
controls. File audit passes with three inherited header advisories. Evidence and
verifiers live in `student.tests/pa14/`; frozen outputs, binaries, command/status
logs and failed initial probes remain under
`$RALPH_ARTIFACT_DIR/pa14-special-signatures/`. Full-stage work remains incomplete
at the concrete declaration/lifetime/demand boundary above.
