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

The [layout/initializer checkpoint](layout-checkpoint.md) records frozen A/B
compiler wall/RSS, native runtime/text, all observations and noisy follow-ups.
Final compiler `.text` grows 50,624 bytes; common median latency differences
span -2.37% to +1.02%, with at most +220 KiB RSS. Common native binaries are
identical. New family work scales proportionally; 32/1000000 omitted, string and
volatile ranges keep 10/27/31 instructions. No optimizer or speedup is claimed.
Historical numeric targets remain diagnostics under the spec; required bounds,
correctness and coverage are unchanged. Initial measurements (including the
14,136 KiB RSS increase) and [earlier evidence](selection-checkpoint.md) remain.

## Handoff ledger

- a97e14d4: initial 43/302; stage markers remain unchanged.
- 4113c34d: member ABI/this, cv, references/projections: 84/302.
- 17897014 / cd054d80: constructors/defaults/DMI, startup: 156/302.
- 2f886c18 / ae8255d4 / 3f590f18 / 55a21dac: lifetime/cleanup/noexcept: 173/302.
- 165af40e / b74a80fa / c7206513 / 88e4ebe1: access/friends/ADL/operators,
  returned conversions and evidence: 234/302; clean continuation baseline.
- 09480f23: bit-field storage/read/write/promotions, reference rules and nested
  EBO collisions: 248/302. Four reference conversion defects have reduced proofs.
- ff86af31: parsed alignment, pragma state and packed layout: 256/302.
- 9a394183: initializer actions, brace elision, strings, narrowing, value-init,
  aggregate helpers and volatile paths: **273/302**. Two further reference
  corrections use the same bit-field rules and explicit volatile-init contract.
- 3b8fc5a3: bounded string padding and directive scratch lifetime; unchanged
  **273/302**. Both initial and corrected frozen performance campaigns retained.
- Final validation: **39 baseline failures fixed, zero regressions**;
  `make test-pa11` 273/302 with all four controls passing (exit 2, incomplete).
  Earlier PAs **1025/1025**; file audit passes with the existing header advisory.
  Eleven personal native programs, six rejection checks, six corrected-reference
  validations, frozen artifacts and range IR bounds pass. Stage base/review
  markers, coverage and comparison rules are preserved. Clean committed handoff.
