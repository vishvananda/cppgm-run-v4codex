# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and comparisons are
unchanged. PA15 has not started.

## Design/spec alignment

Active continuation from `ca42e706` (previous turn: verified progress). The
projection/demand owner will separate member bodies, constructor initializers
and default arguments from declaration completion. Shared source regions retain
roots; demanded regions establish context-owned occurrences and facts. Work
must follow declaration syntax plus demanded regions, with no repeated traversal
of undemanded bodies. Validate unused dependent defaults, explicit arguments,
recursive/nested demands, context identity, course/native parity and compiler/RSS
scaling before extending the same region ownership further.

Continuation from `5b1afe54` is **verified progress**. Raw function parameter
types now feed bodies directly, preserving cv, array/function adjustment and
reference collapse. Canonical declaration facts extend that path to supported
local aliases, fields and renamed out-of-line members. Immutable frames include
specialization, declaration head and enclosing bindings; complete frame/type,
query and binding keys avoid cross-environment reuse. Queries retain concrete
access context, overload identity and implicit-object cv. Body and prototype
parameter identities remain distinct for definition-time checking.

| Owner / data flow | Complexity and validation |
| --- | --- |
| Retained declaration types (implemented) | Source specifiers/declarators → canonical types → complete substitution frames → concrete parameter/local/field declarations. Work follows source type syntax, distinct frame/type keys and actual uses; validate renamed heads, references, queries, native execution and 1,000/4,000 scaling. |
| Dependent typed body graph (remaining) | Local class/enum identity, value-dependent bounds, symbolic receivers, storage dependencies, constructors/operators and declaration/return conversions need typed dependencies. Replace whole-region projection; validate contexts, identities, lifetimes and source/instance scaling. |
| Demand/failure graph (remaining) | Separate declaration/definition/layout/default/exception/body/vtable/emission states, typed reasons/reverse edges and structured expected failures. Compute once per complete key; validate recursion, negative keys and unrelated-declaration scaling. |

**Concrete boundary:** this group retains types whose source representation is
complete and substitutes them through declaration-owned environments. Local
class identities, value-dependent array bounds and unsupported query forms
explicitly defer to their existing semantic owners. Completing those forms and
dependent-only projection requires expression, declaration-identity, conversion
and lifetime dependencies; extending a type cache alone cannot represent them.
Whole regions still project 457N/174N occurrences in the new declaration/member
corpora, and inherited field corpora retain 92N/130N/32N+176. These remain
current-stage defects. No external blocker exists; PA15 has not started.

## Performance evidence and budgets

All **7,854 observations** verify: 7,266 inherited plus two frozen 294-row
declaration campaigns. [performance.md](performance.md) retains every sample,
A/A calibration, paired blocks, spread and all compiler/native hashes. Final
4,000-instance declaration latency is 2.945107→2.856347 s (ABBA .9570/.9874);
renamed members 1.552243→1.531868 s (.9855/.9892). Unused source checking costs
1.753455→1.865442 s (1.0649/1.0647), with five newly rejected invalid function-
pointer bodies. Compiler text is 1,274,054 bytes, +8,384 (+0.662%) from entry.

Entity/Expression/ObjectUse remain 112/36/36 bytes; immutable frames are 20 bytes.
Repeated declarations retain 30 source type facts, N frames, 6N substituted
records and 57N uses/hits; renamed members retain ten source facts, 3N frames,
10N records and 14N uses. Unused definitions allocate no frames or occurrences.
Work/storage are bounded by source facts, complete dependent keys and actual
uses. Five native loops preserve exact executable hashes/payload sizes; the
common-correct generated-code growth budget remains **zero**. No optional
optimizer or native backend was added. O0 has no mandated numeric compiler
latency/RSS/text ceiling; unsupported inherited diagnostic gates do not override
stage-scoped acceptance. Correctness, mandated limits and coverage remain.

Calls-4 RSS is 305,864→305,796 KiB against this entry. The inherited approximately
+15 MiB delta against `33b791da` remains unexplained; it is not attributed to
necessary semantic work or waived as harmless. See retained earlier campaigns.

## Handoff ledger

| Coherent increment | Commit / evidence |
| --- | --- |
| Inherited fixed fields, receivers, prototype scopes and nested method declarators | `78bdbc3f` through `24ad2c45`, reducers `ff744067`, reviewed evidence `5b1afe54`; 7,266 observations and all former proofs preserved |
| Raw source parameter types feed concrete function bodies | `4f0d95e1`; default through 1935/1935, nineteen native controls |
| Canonical declaration types, immutable frames and query context/binding substitution | `d19e36b8`; source deferral regressions reduced 312→314, twenty native controls |
| Frozen declaration/compiler/native corpus | `bbb9d729`; all outputs identical across A/B; preliminary observations retained |
| Preserve fixed checks alongside type-only query parameters and source access contexts | `5ce59182`; eight new rejection controls pass, including two reduced ordinal regressions; fixed unknown-call binding rejection restored |

Final validation at `5ce59182`: PA14 **314/314**, prior **1621/1621**, default
through **1935/1935**, twenty native programs, **334** release/ASan/UBSan parity
inputs, **90** rejection controls on both compilers, two ABI controls, four
reducer parity/native checks, all performance evidence and file audit (three
inherited header advisories). Earlier failed probes/logs remain preserved.
Artifacts, command/status manifests, proofs and frozen binaries are under
`$RALPH_ARTIFACT_DIR/pa14-signature-facts/`; raw evidence and verification are
committed in `student.tests/pa14/`.

This handoff completes the declaration-type, substitution-frame and query-context
group. Extending it into the two remaining graph owners requires explicit
expression, identity, conversion and lifetime dependencies rather than another
type-cache extension; the concrete boundary above remains. Full-stage work and
the inherited RSS investigation are open, with no external blocker.
