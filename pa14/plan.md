# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original **84/314**, current **314/314**: all 230 original failures resolved,
with unchanged fixtures, references and comparison rules. PA15 has not started.
Active continuation from `460f495a`; the preceding turn was verified progress.
Audit scope: declaration/default binding → checked default expression/conversion
→ per-call materialization. Group failures by declaration environment, concrete
parameter/default key, terminal failure, and consumer-owned lifetime. Budgets:
one binding and conversion check per complete default key, no unused dependent
default/body work, and O(required call arguments) materialization. Validate
invalid conversions, access context, repeated failures, recursive defaults and
independent object lifetimes; preserve earlier source-identity controls.

Conversion increment: immutable declaration slots now address concrete default
facts, including the function-specialization key. Copy-initialization and list
recipe validation publish terminal success/failure before calls consume them.
Seven public probes repeat 10,000 requests; 314 stage, 1621 prior and 35 native
controls pass. Related demand defect remains active: defaults first seen in a
type query can lose required function/member/storage definitions. Ordinary unused
defaults must also defer those dependencies under N3485 [temp.inst]/10.

## Design/spec alignment

| Owner / data flow | Complexity and validation |
| --- | --- |
| Source facts, signatures, defaults/initializers and enum identities | Retained source IDs → immutable frames → concrete facts. Historical source/key/use bounds remain intact. |
| Specialization definitions, layout and demanded template bodies | Independent active/success/failure states; terminal failure across the selected-definition interval. Nine inherited repeated-request controls preserve graph sizes. |
| Virtual dependency publication → ABI emission | Typed key/lifecycle reasons, one reverse key edge and emitted class ID, immutable slot IDs, class/member caches and final ordering of emitted IDs. Six inherited public controls and native ABI checks. |
| Destructor declarations → exception/triviality properties | Complete subobject declarations before querying them; independently infer absent exception specifications on redeclaration. Typed Boolean outcomes distinguish unavailable, active, true/false and terminal failure; no unused body demand. Twelve source and four public controls. |
| Member preparation → lifecycle effects and transfer representation | One action state per mutually exclusive constructor/destructor owner; separate omission and transfer states. Publish success after the whole preparation interval. Deleted transfer is a completed negative fact. Six controls repeat 10,000 queries without graph growth. |
| Local lifecycle definition → member work → subobject bodies/vptr | Typed use/definition/vtable/transfer reasons. External destructor use leaves local actions pending; a checked local definition roots them even without a local use. Wake only its prior external request. Five controls cover four cross-TU executables and an external-only declaration. |

The lifecycle group extends beyond its initial declaration-property scope to
premature action queries, repeated transfer failures, external completeness
requirements and missing cleanup/vptr setup in unused out-of-line definitions.
The shared action-state field is valid member ownership; its old Boolean
publication and omission timing were the defects. Typed state storage shrinks
MemberFacts rather than adding a second constructor/destructor action cache.

## Remaining groups and concrete boundary

The connected lifecycle declaration/action/definition group is complete.
Full-stage architecture still requires typed demand and structured expected
rejection in default arguments, general member/function-body publication and
emission, plus the retained type/query audit. These are not PA15 work.

The next owner is `default_arguments.cpp` and instantiated default occurrences:
source binding uses its own queued class-completion consumers; concrete defaults
use occurrence-root states and replace a declaration's slot with a converted
expression. Its keys must preserve declaration environment, parameter target,
substitution context and per-use lifetime actions. The lifecycle cache's complete
class/member key cannot substitute for this occurrence/use audit. General body
publication also needs its own full failure interval, including direct source
bodies outside the template worklist. Extending the lifecycle records into these
owners without tracing their distinct producers and consumers would conflate
source checking, converted defaults and runtime materialization. This is the
concrete boundary of this increment; there is no external blocker.

PA12 excludes member pointers; PA13–PA14 do not add them. The archived exploration
imposes no gate. The inherited pure-virtual personal source remains an IR control
because the supplied backend cannot resolve its support symbol; its concrete-base
companion executes natively and course coverage remains unchanged.

## Performance acceptance

**14,896 observations**: 14,504 preserved plus 308 main and 84 repeat samples.
Fifteen A/B LowIR and seven executable hashes match exactly. Work controls vary
unrelated declarations N, owners K, fields S and uses Q: **K+1** member requests
and **K×S** destruction actions are independent of N and Q. Budgets are one
property/preparation per completed owner key, one visit per required edge, no
unused body demand, geometric storage and zero generated growth on comparable
correct outputs. There is no optional runtime transform or mandated numerical
PA14/O0 latency/RSS/compiler-text ceiling.

Compiler `.text` grows **0.222%**; MemberFacts **124→120**, ClassFacts **120**,
Analyzer **6216**, Procedural **1472**; other measured hot records are unchanged.
Twenty-four live transitive headers are frozen and checked. Declaration latency
is **+1.80%**, repeated **+0.69%**; repeated-demand latency **+0.99%**, repeated
**−0.16%**. The 512-owner/four-field case is **+0.66%**, repeated **+1.78%** with
a noisy paired block. All positive costs, RSS spread and unisolated outliers
remain in [performance.md](performance.md); no broad speed/RSS benefit is claimed.
No timing overlaps builds, tests, proofs, layout probes or verification. Historical
misses and optional all-input retiming remain diagnostics, not new exit gates.

## Handoff ledger

| Increment | Commit / evidence |
| --- | --- |
| Inherited source ownership, terminal failures, virtual scheduling/ABI | Through `d06d62b2`; 14,504 preserved observations |
| Complete destructor declaration properties and compatible redeclarations | `4b55c175`; late subobjects, unused bodies, forward queries |
| Publish action, omission and transfer states with terminal failure | `d269a1fc`; six repeated public controls |
| Root local lifecycle definitions and defer external bodies | `ebe35d0b`; four entry failures repaired, five source/cross-TU controls |
| Validation, layouts and performance evidence | `lifecycle-*.json`; 79 checks, 392 observations, frozen A/B/sanitizer binaries |

Validation: **314 stage / 1621 prior / 1935 through**, **35 existing native
programs + four cross-TU programs**, **349 sanitizer and entry/current parity
inputs**, 20 new public query runs, inherited rejection/virtual/failure controls
and owning PA13 ABI checks under both compilers. File audit passes with three
inherited header advisories; all **1266 fixture/reference hashes** are unchanged.
Artifacts: `$RALPH_ARTIFACT_DIR/pa14-lifecycle-facts/`. Full-stage architecture
remains open; this increment completes the connected lifecycle behavior group.
