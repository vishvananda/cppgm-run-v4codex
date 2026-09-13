# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and comparisons are
unchanged. PA15 has not started.

## Design/spec alignment

Active continuation from `c78e8d3b`; previous turn: **verified progress**.
The body value owner will distinguish fixed expression types/conversions from
value-dependent layout queries. Source sizeof/alignment queries retain canonical
typed operands; complete frame/query keys substitute only dependencies. Enclosing
fixed scalar operations should retain their semantic decisions even when operand
values depend on a template. Extend this graph into value-dependent array bounds
where its type/constant owners apply. Work follows unique source queries, complete
substitution keys and demanded values; preserve short-circuit evaluation, source
checks, local identities, default-head ownership and lifetime consumers. Validate
course/native parity, new definition-time rejection proofs and frozen compiler/RSS
plus executable measurements before closing this group.

Continuation from `ca42e706` is **verified progress**. Member bodies, constructor
initializers and default expressions now have separate demand roots. Concrete
defaults retain semantic active/success/failure state and the declaring template
head, including renamed definitions and calls preceding those definitions.
Fixed default names are checked at definition time; dependent values wait for
an omitted argument. Later declarations cannot add template defaults contrary
to N3485 [dcl.fct.default]/4–6. Six invalid-default controls are newly rejected.

| Owner / data flow | Complexity and validation |
| --- | --- |
| Declaration types (inherited, implemented) | Source types → immutable substitution frames → concrete declaration facts. Complete frame/type/query keys; raw body parameters preserve cv, reference collapse and declarator adjustment. Previous evidence remains in `ca42e706` and performance.md. |
| Source regions and defaults (this group, implemented) | Parsed source root → immutable node/root/attribute ID slices → demanded source/context occurrences → body/initializer/default semantic owner. Index each demanded source region once; work/storage follow indexed source edges and concrete demand. Defaults compute once per specialization/root; declaring-head overlays contain only head parameters. Validate nested/local classes, explicit/omitted arguments, side effects, renamed heads, alignment/packing and 1,000/4,000 scaling. |
| Dependent typed body graph (remaining) | Expressions, local class/enum identities, value-dependent bounds, receivers, storage dependencies, constructors/operators and declaration/return conversions need explicit typed dependencies. Reuse fixed nodes within used regions; validate contexts, identities, lifetimes and source/instance scaling. |
| Demand/failure graph (remaining) | Extend separate declaration/definition/layout/default/exception/body/vtable/emission states with typed reasons, reverse dependencies and structured expected failures. Compute once per complete key; validate recursive demand, negative keys and unrelated-declaration scaling. |

**Concrete boundary:** this group separates regions and indexes their immutable
source topology. A used region still projects occurrences for all its contents.
Replacing that projection requires expression, declaration-identity, conversion
and lifetime dependencies shared by semantic analysis and lowering. Extending a
source-region index cannot establish those facts. These are current-stage defects,
not later-stage exemptions; the new region/default owner is coherent and tested.
No external blocker exists. The two remaining graph owners require a separate
representation change across their producers and consumers.

## Performance evidence and budgets

All **8,932 observations** verify: 7,854 inherited, three frozen 336-row region
campaigns and a 70-row isolated cache comparison. [performance.md](performance.md)
retains every sample, A/A calibration, ABBA pairing, spread and compiler/native
hash. Large unused-body N=4,000 latency is 4.114022→.672118 s (ratios .1635/.1640),
peak RSS 777,150→111,708 KiB. The intermediate fully-used-body slowdown is retained
and corrected: final 2.538970→2.393640 s (.9459/.9704). Isolating the source cache
against the already-correct demand implementation improves both body workloads
in both paired blocks; its used-body RSS rises **6,714 KiB**, still unexplained.
The inherited roughly +15 MiB calls-4 RSS delta against `33b791da` also remains
open; measurements moving both A and B together do not establish its cause.

Body occurrences change from `(88+11W)N` to `69N` for the small used body and
`(65+11W)N` for the large used body. The small-body source index stays at two
regions, 65 nodes and five roots, independent of N/W. Unused defaults fall from
34N to 21N occurrences with zero semantic default work. Repeated defaults retain
28 occurrences and one default computation, independent of call count. Index
storage follows source nodes/roots/attributes; specialization storage follows
demanded occurrences and complete semantic keys. Used-region projection remains.

Compiler text is 1,282,758 bytes, **+8,704 (+0.683%)** from entry; the cache itself
adds 2,240 bytes. Entity/Expression/ObjectUse remain 112/36/36 bytes, frames 20
and occurrences eight. Per-TU Ast grows 304→352→504 bytes. Frozen historical
layout snapshots and a current live-header probe replace an unsupported demand
for unchanged historical header hashes. Native hashes/payload sizes are identical
for common-correct inputs; the generated-code growth budget remains **zero**.
No optional optimizer or native backend was added. O0 has no mandated numeric
compiler latency/RSS/text ceiling. Source/key work bounds and repeatable body
savings justify the cache; all measured costs, correctness, mandated limits and
coverage remain. Unsupported diagnostic targets are not additional exit gates.

## Handoff ledger

| Coherent increment | Commit / evidence |
| --- | --- |
| Inherited field/receiver/prototype ownership, declaration types, substitution frames and query contexts | `78bdbc3f` through `ca42e706`; prior proofs and 7,854 observations preserved |
| Independent member body, initializer and default demand | `feac8cd6`, `6edb0f18`; unused dependent defaults and declaring-head identity/native proofs |
| Frozen region compiler/native corpus | `8a672653`; three full campaigns, exact common-correct output preservation |
| Initial declaration ownership of template defaults | `03e76021`; six new rejections, CWG 15/217 proof; incorrectly permissive personal source preserved, no course/reference change |
| Independent proofs and layout probe | `0e2761bb`; two new native successes and frozen historical/current layouts |
| Immutable source-region index | `e1afac7c`; stage/through/native pass, source work equations and alignment/packing parity |
| Isolated cache comparison | `10807fc9`; both body workloads improve in both paired blocks; RSS/outlier costs disclosed |

Final validation of `e1afac7c`: PA14 **314/314**, prior **1621/1621**, default
through **1935/1935**, 21 native programs, **335** release/ASan/UBSan parity inputs,
**102** rejection controls on both compilers, two ABI controls, six explicit
reducer parity/native checks, all performance evidence and file audit (three
inherited header advisories). Command/status manifests, earlier failed probes,
frozen binaries and proofs are under `$RALPH_ARTIFACT_DIR/pa14-regions/`; raw
evidence and verification are committed in `student.tests/pa14/`.

This handoff completes region/default ownership and the related source-index
extension. Full-stage dependent-body and demand/failure graph work, plus the
unexplained RSS observations above, remain open at the concrete boundary stated.
