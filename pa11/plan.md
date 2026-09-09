# PA11 implementation

Stage base commit: a97e14d49c7edfc7acc115b974ab667cc90480db
Last reviewed commit: a97e14d49c7edfc7acc115b974ab667cc90480db

Target: PA11 full-stage. Phase: implement. **Incomplete: 273/302; 29 failures.**

## Design/spec alignment

Extend the integrated typed graph and PA10 lowering. Access/friend/ADL and
operator selection record chosen declarations, conversions and object paths.
Sparse field descriptors own bit widths, signed storage, preservation and
alignment; class layout handles EBO identities, explicit alignment and token
snapshots of pragma packing. One semantic initializer cursor records brace
elision, strings, scalar conversions and compact omitted ranges. Local/global
lowering and generated array aggregate helpers consume those actions. Eleven
personal programs validate LowIR and execute; six invalid programs reject.

Work follows declarations, relevant lookup edges, layout fields, initializer
actions and emitted IR. Ordinary nodes/tokens retain their compact sizes; rare
attributes/descriptors use flat ID indexes. Eight elements is the array expansion
budget; larger omitted ranges use bulk zeroing or loops preserving volatile
stores. Existing ABI/lifetime records remain the basis for PA12/PA13 extensions.

## Remaining groups and boundary

| Owner | Required data flow / complexity | Validation |
| --- | --- | --- |
| Constructor selection and constant initialization | Record converting/inherited constructors, argument substitutions, temporary lifetimes and legal early static initialization; O(candidates + selected actions). | implicit class-reference conversion, inherited/external constructors, placement new, namespace arrays with constructors |
| ABI entries and storage duration | Keep complete/base roots, empty-class arguments, incomplete declaration boundaries and TLS initialization families distinct; O(objects + required helpers + IR). | anonymous/local constructor roots, external destructors, incomplete return/reference types, TLS |
| Parser/declaration context | Complete-class name facts and literal suffix identities must feed canonical types without syntax replay; O(relevant declarations + lookup edges). | late member subscript, injected names, nested definitions, UDLs, invalid static initializers |
| Expression and boundary views | Recorded category/conversion facts select discarded accesses, function/reference views and builtin boundaries; O(expressions + output). | discarded parameters/objects, reference-indexed member access, floating intrinsics, boundary metadata |

The layout/aggregate group is implemented and validated, including its nested
reference-binding and volatile-helper variants. Remaining constructor-array
static output needs argument substitution and body-effect legality facts;
inherited/placement/converting calls need new selection/ABI/lifetime records.
Those cannot be obtained by extending the initializer cursor or changing class
layout. Remaining declaration ambiguities need parser ownership, and remaining
root/discard cases need separate ABI/value-category decisions. No later PA is
advanced. Six narrowly corrected references have reduced proofs and the bundle
revision in [reference corrections](reference-corrections.md); comparison rules,
source tests and coverage are unchanged.

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

- Initializer increment: **273/302**, 17 additional failures fixed since the
  alignment commit; **39** baseline failures fixed in this continuation, no
  regressions. Explicit initializer actions feed static data and procedural
  lowering; narrow list conversions, zero-before-constructor facts, readonly
  string pooling and scalar aggregate-array helpers share this owner. Large
  omitted ranges are represented once and lower within the eight-element
  expansion budget. `make test-pa11`: 273/302, all four controls pass; earlier
  PAs: **1025/1025**; file audit passes with the existing header advisory.
  Eleven personal programs validate/execute; six rejection checks pass.
