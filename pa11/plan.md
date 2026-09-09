# PA11 implementation

Stage base commit: a97e14d49c7edfc7acc115b974ab667cc90480db
Last reviewed commit: a97e14d49c7edfc7acc115b974ab667cc90480db

Target: PA11 full-stage. **Complete: 302/302**, including all four controls.
Cumulative PA1–PA11: **1327/1327** (earlier stages 1025/1025). File audit passes
with the existing Analyzer-header advisory. Seventeen personal native programs
validate and execute; ten invalid programs reject. Coverage is unchanged.

## Design/spec alignment and ownership

Extend the integrated typed graph and PA10 lowering. Semantic selection records
canonical declarations, conversions, object paths, layout and lifetime actions;
lowering consumes them directly. Rare descriptors and actions live in TU-owned
flat ID indexes/arenas. Source regions are parsed once. Existing ABI and lifetime
identities remain available to the later virtual-object and value-transfer stages.

| Owner | Data flow / complexity | Validation |
| --- | --- | --- |
| Access/selection | Scope, friend and associated lookup select declarations and object adjustments. O(relevant edges + candidates + selected arguments). | access/ADL/operator fixtures and native calls |
| Layout/initializers | Field descriptors own bit widths, signed storage, EBO identities and alignment/packing. One cursor records brace elision, strings and omitted ranges; local/global/helper lowering consumes actions. O(fields + actions + required IR). | bit fields, aligned layouts, aggregate cursors, volatile stores and range bounds |
| Constructors | Inherited signatures respect local declarations and base access; converting calls own argument ranges and temporary lifetimes. Braced arguments check narrowing after selection. O(candidates + arguments + subobject actions). | inherited/explicit/deleted/access cases, conversion/default/lifetime execution and narrowing rejects |
| ABI/storage | Complete/base demand, rooted helpers, empty parameters and incomplete declarations remain distinct. TLS records guards/init/wrappers; emitted native names reserve collision-free labels separately from canonical ABI identity. O(required entries + objects + IR). | external/transitive entries, empty/incomplete boundaries, TLS collision and first use |
| Construction lowering | Placement calls consume selected allocation conversions and initializer actions. Cached effect-free scalar-forwarding summaries support required static constructor-array data; unsupported effects/conversion chains keep dynamic calls. O(parameters + actions + initializer data). | placement, static/dynamic array execution and conversion-preserving fallback |
| Parser/expressions | Injected names, complete-class categories, qualified decltype and UDL suffix IDs feed canonical types/ABI. Floating builtins and explicit discard emit typed operations. O(consumed syntax + selected operations). | stage fixtures, native float/UDL checks and earlier reports |

No PA11 behavior group remains failing. Seven narrow reference corrections have
reduced proofs, cited rules and the pinned bundle revision in
[reference corrections](reference-corrections.md). The final correction restores
mandatory static reference binding before dynamic initialization; fixture inputs,
comparison rules and coverage remain intact.

## Performance evidence

[Construction completion](construction-checkpoint.md) records frozen compiler
wall/RSS/text, checked native runtime/text, proportional work, all observations
and noisy follow-ups. Final compiler text grows 58,432 bytes (8.08%). Common
outputs and native binaries are byte-identical. Final common median latency
changes span -1.48% to +4.38%; the follow-up retains noisy reference observations.
The largest common-campaign/follow-up RSS increase is 19,948 KiB (7.08%); separate telemetry
preserves equal semantic/IR work and pool capacities, without claiming an
allocation cause. New 4x family inputs take 4.03–4.09x median compiler time.

These are required O0 semantics, with no optional optimizer or speedup claim.
Historical numeric targets remain diagnostics under the spec's stage-scoped
rule; all measurements and misses remain available. The mandatory eight-element
expansion budget remains unchanged: [layout evidence](layout-checkpoint.md)
records 10/27/31 instructions for omitted/string/volatile ranges at both 32 and
one million elements. [Earlier selection evidence](selection-checkpoint.md) and
all earlier campaigns remain preserved. Correctness, coverage and proportional
work remain mandatory.

## Handoff ledger

- a97e14d4: initial 43/302; stage base/review markers remain unchanged.
- 4113c34d: member ABI, cv/references/projections: 84/302.
- 17897014 / cd054d80: construction, defaults/DMI and startup: 156/302.
- 2f886c18 / ae8255d4 / 3f590f18 / 55a21dac: lifetime/cleanup/noexcept: 173/302.
- 165af40e / b74a80fa / c7206513 / 88e4ebe1: access/ADL/operators and returned
  conversions: 234/302, with frozen evidence.
- 09480f23 / ff86af31: bit fields/EBO and aligned/packed layout: 248 then 256/302.
- 9a394183 / 3b8fc5a3: initializer actions and bounded padding/scratch: 273/302;
  six proved reference corrections and both performance campaigns retained.
- 786ed29e: clean construction-continuation baseline, reproduced 273/302.
- f5918ce9: inherited forwarding and complete/base entry demand: 276/302.
- af72ea1d: construction, ABI/TLS, parser and expression boundaries: 302/302.
- 265440db: final braced-constructor narrowing review; 302/302 and cumulative
  1327/1327, file audit, 17 native programs and ten rejection cases pass.
  All 29 continuation-baseline failures fixed, zero regressions. Initial and
  corrected frozen evidence retained and verified. Handoff is stage completion;
  no PA11 work is deferred and no later assignment is advanced.
