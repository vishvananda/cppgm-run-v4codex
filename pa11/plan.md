# PA11 implementation

Stage base commit: a97e14d49c7edfc7acc115b974ab667cc90480db
Last reviewed commit: a97e14d49c7edfc7acc115b974ab667cc90480db

Target: PA11 full-stage. Phase: implement. **302/302; final validation and performance review.**

## Design/spec alignment

Extend the integrated typed graph and PA10 lowering. Access/friend/ADL and
operator selection record chosen declarations, conversions and object paths.
Sparse field descriptors own bit widths, signed storage, preservation and
alignment; class layout handles EBO identities, explicit alignment and token
snapshots of pragma packing. One semantic initializer cursor records brace
elision, strings, scalar conversions and compact omitted ranges. Local/global
lowering and generated array aggregate helpers consume those actions. Seventeen
personal programs validate LowIR and execute; nine invalid programs reject.

Work follows declarations, relevant lookup edges, layout fields, initializer
actions and emitted IR. Ordinary nodes/tokens retain their compact sizes; rare
attributes/descriptors use flat ID indexes. Eight elements is the array expansion
budget; larger omitted ranges use bulk zeroing or loops preserving volatile
stores. Existing ABI/lifetime records remain the basis for PA12/PA13 extensions.

## Completed ownership groups

| Owner | Data flow / complexity | Validation |
| --- | --- | --- |
| Constructor selection | Inherited signatures hide behind local declarations; selected converting calls own argument ranges and temporary lifetimes. O(candidates + selected arguments + actions). | inherited/access/external paths; converting defaults, rejection of explicit/chained conversions; native lifetime checks |
| ABI/storage | Complete/base demand, rooted helpers, empty object parameters and incomplete declaration storage remain distinct. TLS owns guard/init/wrapper identities and native-name reservations. O(required entries + objects + IR). | object roots, incomplete references/returns, empty arguments, native TLS collision and first-use checks |
| Initialization | Placement allocation consumes selected conversions and existing initializer actions. Cached effect-free scalar-forwarding summaries permit early static constructor-array data; unsupported effects/conversion chains retain dynamic calls. O(parameters + actions + initializer data). | placement execution, static/dynamic array behavior, conversion-preserving fallback |
| Parser/expressions | Injected names, complete-class value categories, qualified decltype and UDL suffixes flow to canonical types/ABI. Floating builtins and explicit discard boundaries emit typed operations. | exact stage fixtures, native float/UDL checks, earlier PA reports |

No current behavior group remains failing. Final frozen performance review and
cumulative validation are in progress; no later PA is advanced. The seventh
narrow reference correction proves mandatory constant reference initialization
with a reduced ordering program and cited C++11 rules. All corrections and the
bundle revision remain in [reference corrections](reference-corrections.md).
Source fixtures, comparison rules and coverage are unchanged.

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

- 786ed29e continuation baseline: reproduced **273/302**, 29 existing failures.
  Frozen compiler `/tmp/pa11-construction-base-cppgm`. Current group records
  inherited constructor forwarding and ABI entry demand; selection owns base
  access, semantic actions own arguments, lowering consumes identities. Work is
  proportional to inherited signatures, selected arguments and subobject actions.
  Validate inherited/access/external-transitive fixtures and native personal
  construction cases, followed by full PA11 and earlier reports.
- Inherited-constructor increment: **276/302**, all five inheritance fixtures;
  twelve native personal programs and seven rejection checks pass. Earlier
  PAs remain **1025/1025** and file audit passes. Complete/base forwarding
  entries consume shared actions; external base calls use the C2 ABI entry.
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

- Construction/boundary completion: **302/302**, all four controls. Seventeen
  native personal programs and nine rejection cases pass, including a native TLS
  name collision missed by the course fixtures. Earlier cumulative validation
  reached **1327/1327** after fixing a PA10 multiplicative conversion regression;
  final freeze will repeat it. File audit passes with its existing advisory.
  Performance protocol and B-only construction corpus are ready; evidence pending.
