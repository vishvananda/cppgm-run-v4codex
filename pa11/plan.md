# PA11 implementation

Stage base commit: a97e14d49c7edfc7acc115b974ab667cc90480db
Last reviewed commit: a97e14d49c7edfc7acc115b974ab667cc90480db

Target: PA11 full-stage. Phase: implement. **Incomplete: 234/302; 68 failures.**

## Design/spec alignment

Extend the integrated typed graph and PA10 lowering. Declarations/base edges own
access; indexed friendship/using/ADL edges preserve canonical entities and hidden
visibility. Selected functions, conversions and implicit objects are recorded
before lowering. Token identities distinguish operators; typed ABI terminals
encode them. Class/member/default/lifetime demand stays monotonic and local.
Temporary addresses and base projections feed existing call/cleanup machinery;
returns consume selected conversions. Seven personal programs execute, including
logical/comma side effects and an overloaded dereference returning a base reference.

Lookup visits relevant scopes/types/edges and candidates. Using-signature hiding
runs only for imported families. Selection uses two dominance passes over viable
candidates; lowering consumes selected records without searching names or
fabricating syntax. Eight array elements remains the expansion budget, with
counter loops beyond it. General PA12 value transfer and PA13 dispatch stay separate.

## Remaining groups and boundary

| Owner | Required data flow / complexity | Validation |
| --- | --- | --- |
| Layout and aggregate initialization | Bit-field storage/alignment, packing, union/volatile containment and a typed initializer cursor feed exact zero/copy spans; O(fields + initializer actions + output). | brace elision, string arrays, alignas, bit-fields, empty-base collisions, scalar narrowing |
| Construction and conversion records | Selected converting/inherited constructors need argument conversions, temporary lifetimes and explicit placement/ABI entry identities; O(candidates + selected actions). | implicit class-reference conversion, inherited/external constructors, placement new, empty-class argument contract |
| Declaration and parser context | Complete-class name facts must resolve late member/type ambiguity and injected names without replaying syntax; O(relevant declarations + lookup edges). | late member subscript, local-class constants, nested definitions, incomplete function/reference types, literal suffix identity |
| Storage/ABI and LowIR views | Storage duration and complete/base entry identity feed TLS/init roots; category/projection facts feed canonical accesses; O(objects + required helpers + IR). | TLS families, external D1/D2 roots, empty-class friend roots, boundary metadata, aggregate globals, discarded/address views |

The access/friend/ordinary-operator group is coherent and validated. Remaining
related cases cannot be closed by extending the candidate set: implicit class
conversions require a constructor/conversion/lifetime record; the two empty-class
ADL fixtures select functions but reach missing argument storage lowering; late
member syntax needs a complete-class ambiguity fact; an anonymous hidden-friend
fixture needs a distinct retained C2 root. Those are the concrete boundary for
this handoff. No fixture, reference or comparison rule has changed.

## Performance evidence

The [selection checkpoint](selection-checkpoint.md) records the frozen common
and new-behavior campaigns, including all pre-return-fix observations and noisy
follow-ups. The [protocol](../student.tests/pa11/performance-protocol.md) preserves
compiler wall/RSS and executable runtime/text requirements. No optional optimizer
or speedup is claimed. Historical numeric ratios remain diagnostics under the
spec's stage-scoped rule; correctness, coverage and proportional work remain
required. [Constructor](checkpoint.md) and [lifetime](lifetime-checkpoint.md)
evidence is retained unchanged.

## Handoff ledger

- a97e14d4: initial 43/302; 259 failures; stage markers remain unchanged.
- 4113c34d: member ABI/this, cv, references and projections: 84/302.
- 17897014 / cd054d80: constructors/defaults/DMI, namespace startup: 156/302.
- 2f886c18 / ae8255d4 / 3f590f18 / 55a21dac: scalar/array/temporary cleanup,
  lexical exits, noexcept and evidence: 173/302; clean continuation baseline.
- 165af40e: access/base/using provenance and namespace/hidden friends: 194/302.
- b74a80fa: canonical operator selection, derived conversion ranking, temporary
  storage and mutable qualification: 233/302; initial measurements preserved.
- c7206513: return lowering consumes selected conversions: **234/302**.
  This continuation fixes **61 existing failures**, with zero fixture regressions.
- `make test-pa11`: 234/302, all four controls pass; exit 2, stage incomplete.
  `make test-report-through-pa10`: **1025/1025**. File audit passes with the
  existing Analyzer-header advisory. Seven personal programs validate LowIR and
  execute with exit 0; three access/hidden-name rejection checks pass.
  No later assignment is advanced. Evidence is committed with a clean handoff.

- Continuation at 88e4ebe1: clean state and baseline 234/302 verified. Extend
  layout ownership first: retained alignment/packing facts, bit-field storage
  units and value widths, empty-base identity collisions, and aggregate cursors.
  Semantic declarations publish descriptors; access/constructor/initializer
  lowering consumes them. Work is proportional to fields, relevant layout
  edges, initialization actions and emitted IR. Validate layout constants,
  bit-field read/write/sign/range behavior, initializer boundaries and all PAs.
- Layout/bit-field increment: **248/302**, 14 baseline failures fixed and no
  regressions. Sparse field descriptors own widths, storage and preservation;
  layout handles nested EBO collisions. Lowering reads/masks/sign-extends and
  writes only the field bits, retaining neighboring ordinary fields and distinct
  repeated subobjects. Promotions use field ranges; const references snapshot
  bit-fields and nonconst references/addresses reject. Eight personal native
  programs and five rejection checks pass; prior PAs 1025/1025; file audit passes.
  Four narrowly edited references are proved against LowIR/C++11 in
  [reference corrections](reference-corrections.md), with reduced inputs and
  the pinned bundle revision. No coverage or comparison changes.
- Alignment/packing increment: **256/302**, eight additional existing failures
  fixed. Attribute operands remain typed syntax; sparse ownership indexes share
  the existing flat ID index. Preprocessor output snapshots pack state without
  enlarging hot token structs; class layout consumes alignment and packing once.
  Nine personal programs validate and execute, including nested named pack
  stacks, `_Pragma`, GNU packed records and actual unaligned member stores.
  Earlier PAs remain **1025/1025**. Next owner: one semantic aggregate cursor
  must retain brace-elision/string/zero actions for local and global lowering.
