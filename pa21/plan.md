# PA21 implementation plan

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Target: PA21 full-stage. Phase: implementation (loop 102).
Entry: clean worktree; 24/116 stage tests pass, 92 fail. Previous turn:
progress (PA20 final audit and committed validation); PA21 implementation starts here.

## Design and remaining groups

| Group / owner | Data flow and work bound | Validation |
|---|---|---|
| RTTI and casts / semantic queries, ABI, typed lowering | Canonical type and checked expression facts select static/dynamic queries; TU-owned RTTI identities emit once; single-inheritance edges bound cast traversal. No syntax replay or textual IR transport. | Type, expression, template, cv, demand, pointer cast and rejection fixtures; personal native controls. |
| Captures / closure facts and object construction | Existing indexed environments extend to value/class captures; selected copy/destructor facts feed closure lowering. | Captures, templates, local identity, lifetime fixtures. |
| Initializer lists / initialization and overload selection | Canonical element/conversion plans own backing storage and lifetime; lowering consumes checked plans. | List ranking, deduction, storage duration, range and class-element fixtures. |
| Exceptions and cleanup / lifetime and control flow | Typed active handler/cleanup states own unwind continuations and constructed subobjects; share only complete identical contexts. | Source throw/try/catch, constructor/destructor, argument transfer and condition cleanup fixtures. |

Implement RTTI/casts first and extend related paths while their ownership is
understood. Preserve stage fixtures and comparison rules. Later native backend,
optimizer, debug and ELF work remain in their owning stages.

## Performance evidence

PA21/O0 acceptance follows spec §9: measure compiler latency/RSS and checked
native runtime/text size using the supplied backend. Freeze binaries/inputs;
use A/A calibration and ABBA for equivalent correct paths. New semantic behavior
has no correct stage-entry A/B baseline. No optional optimization or unsupported
self-imposed performance gate is introduced. Measurements pending.

## Handoff ledger

Loop 102 in progress. All four groups above are unfinished implementation.
Independent whole-stage architecture/correctness review remains pending and is
separate from implementation; neither review marker is advanced here.
Required stage, prior-through, file audit, personal checks and clean committed
handoff evidence are pending.
