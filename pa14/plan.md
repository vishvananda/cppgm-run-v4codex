# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and comparisons are
unchanged. PA15 has not started.

## Design/spec alignment

Active continuation from `5b1afe54` (previous turn: verified progress): retained
raw parameter/declaration types now flow through immutable specialization/head
frames. Renamed heads retain enclosing class bindings; complete frame/type/query
keys share signature/body/local work. Access contexts, overload unions and
parameter type identities substitute without runtime-object guesses. Source
work tracks declaration syntax; concrete work tracks distinct dependent type
nodes and actual uses. Twenty native controls and eight new rejections pass;
final performance/sanitizer evidence is in progress.

Canonical source bindings and shared scalar/call/receiver facts now retain fixed
fields owned by templates. Method contexts carry the pattern owner, cv and
availability of `this`. Concrete field identity and base adjustment are cached
by source field plus concrete object type, including cv. Class declaration
contexts survive separate out-of-line bodies and later forward declarations.
Preserved operator-result identities use the same field cache. Mutable/reference
members, fixed-base access and known bit-field promotions use ordinary rules.

Out-of-line declarations establish static status from candidate evidence;
unknown mixed dependent signatures remain deferred. Nested pointer-return
declarators select the method's own parameter list. Prototype scopes bind
parameters sequentially for later type queries. Two bits per parsed source node
and reusable scratch avoid repeated syntax scans and allocation per declaration.
Prototype parameter ordinals describe type queries, separate from runtime
objects. Detailed ownership, C++11 rules and reducers: [implementation.md](implementation.md).

| Remaining current-stage owner | Data flow, complexity and validation |
| --- | --- |
| Dependent typed body graph | Symbolic current-instantiation receiver/field types, dependent declarators/signatures, static value/storage dependencies, constructor/operator nodes and declaration/return conversions must feed substituted facts. Replace whole-region projection with dependent-only work. Validate access, nested environments, cv/categories, queries, lifetimes and source/instance scaling. |
| Demand/failure graph | Separate declaration/definition/layout/default/exception/body/vtable/emission states, typed reasons/reverse edges and structured expected failures. Compute once per complete key; validate recursion, negative keys and unrelated-declaration scaling. |

**Concrete boundary:** the completed group maps template-owned objects whose
field value facts are fixed. Receivers/fields with substituted types require
symbolic type propagation across declarations, queries, overloads and member
uses. They cannot enter the source-only fixed-expression index without mixing
environments. That is the next graph owner, beyond extending this field cache.
Whole regions still project **92N/130N** nodes in the member instance corpora,
**32N+176** in the repeated-field corpus; inherited receiver corpora remain
77N/46N. These are current-stage defects, not waived by green tests or performance
acceptance. No external blocker exists.

## Performance evidence and budgets

All **7,266 observations** verify: 6,104 inherited plus 1,162 from eight frozen
member, allocation, packed-storage, prototype and method campaigns. See
[performance.md](performance.md). Correctness costs are separated from
common-correct reuse comparisons; every outlier is retained.
The inherited source-hash/layout check now uses a current model snapshot while
preserving previous probes. Hot Entity/Expression/ObjectUse records remain
112/36/36 bytes; new method-context/member-use records are 8/12 bytes.

At 4,000 class instances, expression work falls 96,004→52,009 and object uses
40,000→24,000. Repeated-field work falls 36,024→8,020; object uses 16,012→12.
Final repeated-1,000 latency is .061380→.058063 s (ABBA .9398/.9431).
Final compiler text is 1,265,670 bytes, +12,608 (+1.006%) from entry.
Unused source validation costs .082713→.088259 s at 1,000, with fourteen newly
rejected invalid bodies. Ordinary calls-4 RSS remains about +15 MiB; scratch
reuse and packed flags did not explain it. This investigation remains open;
the delta is not attributed to necessary semantics or waived as harmless.

Work/storage follow source type syntax, actual overload candidates, concrete
field/object keys and emitted uses. Scope flags occupy two bits per parsed node;
scratch retains at most the largest parameter-type traversal. Common-correct
fixed reuse has a **zero generated-code growth budget**, checked by exact LowIR
and executable hashes. Necessary prototype-query behavior has B-only compiler
and native baselines because the entry rejects it. No optional optimizer/native
backend was added. O0 has no mandated numeric compiler latency/RSS threshold;
unsupported inherited diagnostic gates do not override stage-scoped acceptance.
Correctness, mandated limits and coverage remain required.

## Handoff ledger

Entry `33b791da` was **verified progress** from receiver/default-use work.

| Coherent increment | Commit / evidence |
| --- | --- |
| Template-owned fixed fields, bit-field properties and concrete class mapping | `78bdbc3f`; initial through 1935/1935 and eighteen native controls |
| Out-of-line contexts and preserved field-result identities | `d2ae9665`; thirteen entry-accepted invalid unused bodies now rejected |
| Frozen field workloads/layouts and removal of free-function qualifier work | `480e7a36`, `a6c99a0b`; first complete AA/ABBA campaign preserved |
| Prototype parameter scope and type-only identities | `b8ad7f6b`; inherited positive reducer repaired; through 1935/1935 |
| Reusable prototype traversal scratch | `e9893b21`; repeated required/native checks pass |
| Packed prototype classification | `918f3971`; source-indexed two-bit facts; full output preflight |
| Actual method parameters through nested declarators | `24ad2c45`; fourteen invalid-body proofs; four positive reducers, enum coverage `ff744067` |
| Frozen follow-up harnesses | `e41b0025`, `e10cac38`, `625a0182`, `d1f229e0`; eight campaigns and final sixteen-input/four-executable preflight |

Validation at implementation `24ad2c45`: **314/314** PA14, **1621/1621** default
prior report, **1935/1935** serial through report, eighteen native programs,
**332** release/ASan/UBSan parity inputs, **82** rejection controls, two ABI
controls, four reducer parity/native checks and file audit (three inherited
header advisories). The default through attempt's twelve PA1/PA2 I/O timeouts
are retained; the retry changes concurrency only, with identical coverage and
timeouts. Expanded reducers at `ff744067` also pass release/sanitizer/native.

Logs, command/status manifests, proofs and frozen binaries are under
`$RALPH_ARTIFACT_DIR/pa14-dependent-objects/`; raw campaigns and verification
are committed in `student.tests/pa14/`. This handoff finishes the fixed-field,
prototype-scope and method-declarator group. The two graph owners and measured
RSS investigation above remain; the full-stage goal is active.
