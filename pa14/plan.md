# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original **84/314**, current **314/314**: all 230 original failures resolved,
with unchanged fixtures, references and comparison rules. PA15 has not started.

Active continuation from `c1e17cdc`; previous turn: **verified progress**.
Current group: explicit key-definition/lifecycle → vtable → member-body/emission
edges. Publish key-definition availability through its owning class, enqueue each
consumer once, and retain independent active/success/failure vtable state. Stable
slot IDs provide outgoing dependencies without copying the slot vector. Lowering
will consume the published demanded-class list and class-owned symbol caches.
Work/storage budgets: one queued key notification and one emitted-class ID per
relevant class, one visit per demanded slot, no work proportional to unrelated
entities for vtable scheduling/emission. Validate late definitions, reverse source
order, recursive lifecycle demands, delayed template members and failure retries;
then required reports, native/sanitizer controls and frozen measurements.

## Design/spec alignment

| Owner / data flow | Complexity and validation |
| --- | --- |
| Inherited source facts, signatures, defaults/initializers and enum identities | Retained source identities → immutable substitution frames → concrete publications. Existing source/key/use bounds, native controls and all historical evidence remain intact through `1978d615`. |
| Specialization declaration/body and class layout | Canonical entity/fact key → active producer → success or terminal failure. Failed requests cannot resume partial publication or masquerade as recursion. Existing per-key state bytes; O(1) completed/failed lookup. |
| Selected member definitions | Specialization/source key owns the entire attempt, including environment/frame construction. Restore class/function context on failure and clear a nested class's complete flag after late validation failure. Source-head selection remains independently versioned. |
| Completion boundary | One terminal translation-unit state prevents an advanced queue cursor from skipping a failed producer on a second finish request. Typed cached diagnostics carry fact kind, entity and source. |

Nine public API controls each make 10,000 failed requests under release and
ASan/UBSan, assert stable graph size, and check unrelated valid layout. Entry
proofs preserve finish loops that rejected only once, spurious incomplete/recursive
errors and a stale nested-class complete flag. Native controls exercise recursion,
forward uses, renamed nested definitions and unused dependent bodies/defaults.

## Remaining groups and concrete boundary

The audited failure intervals are complete. Remaining required work spans typed
demand reasons, precise reverse dependencies and structured expected rejection
across defaults, destruction/exception specifications, bodies, vtables and emission.
Defaults use occurrence keys; definitions use source-head keys; destruction and
exception caches own separate values. `actions_ready` shares storage between
constructor/destructor paths, and destructor exception states also encode truth.
A blanket state table would conflate independent completeness, value and body
demands or freeze absence before a later definition arrives. The next increment
needs a producer/consumer and insertion/cycle audit before replacing these owners.
There is no external blocker. The in-scope retained type/query audit also remains;
unsupported language forms do not create architecture exit gates.

Scope correction: PA12 explicitly excludes member pointers; PA13–PA14 do not add
them. The exploratory patch, reducers and observations are preserved outside the
checkout in `pa14-member-pointer-types/`. They make no production change and impose
no stage gate.

## Performance acceptance

**14,112 observations**: 13,776 inherited, 266 accepted-state measurements and 70
from a rejected completed-class shortcut. Fourteen LowIR and five executable
hashes match exactly; observed work/storage counters are unchanged. The largest
consistent large-input median increase is **1.39%** (wide member definitions),
with peak-RSS differences across large inputs **−76 to +146 KiB**. Compiler text
is **1,320,774 → 1,323,142** bytes (+0.1793%); Analyzer **6152 → 6160**; existing
hot records retain their sizes. No runtime benefit is claimed.

The shortcut showed mixed compiler results and made the wide case 0.61% slower;
it was removed. Its patch, binaries, validation and observations remain intact.
The restored accepted compiler matches its frozen SHA exactly. Current layout
proofs own eighteen live transitive headers; older probes retain their snapshots.
Details, outliers and paired results remain in [performance.md](performance.md).

Budgets remain source/key/use-proportional storage, at most four O0 conversion
variants, one writable Fact publication view and zero generated growth. PA14/O0
mandates no numerical latency/RSS/compiler-text ceiling. Repeating all 54 historical
timing inputs is a diagnostic campaign choice, not a stage exit gate; this campaign
covers affected N/K/Q dimensions and representative compiler/native work. No
historical measurement, mandated limit or correctness coverage was removed.

## Handoff ledger

| Increment | Commit / evidence |
| --- | --- |
| Inherited source ownership and performance | Through `1978d615`; 13,776 preserved observations |
| Terminal specialization/layout/completion states | `3715bc22`; public repeated-demand proofs |
| Complete member-definition failure interval | `a4f88e1b`; environment and class-context restoration |
| Late nested-class publication failure | `b49acc80`; pre-fix ASan assertion and corrected control |
| Performance and final acceptance | `demand-failure-handoff.json`; accepted B, rejected shortcut, all evidence retained |

Validation: **314 stage / 1621 prior / 1935 through**, **34 native programs**,
**348 sanitizer parity inputs**, **166 required rejections**, one optional diagnostic,
six ABI controls, inherited reducers and initializer/store/lifetime checks, plus
nine repeated-demand controls per compiler. File audit passes with three inherited
header advisories. All **1266 fixture/reference files** are unchanged. Artifacts:
`$RALPH_ARTIFACT_DIR/pa14-demand-failures/`. Full-stage architecture remains open.
