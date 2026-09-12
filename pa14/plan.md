# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**.
Entry: **84/314**, **230 failures**; prior through PA13 **1621/1621**.

## Design and remaining groups

| Owner / group | Data flow, complexity, validation |
| --- | --- |
| Specialization demand and ABI | Canonical declaration + interned TypeId argument pack -> monotonic declaration/body/emission facts -> ordinary typed LowIR. Add body demand to the existing function specialization registry; retain one parsed pattern with contextual semantic occurrences, no token replay or syntax cloning. O(demanded nodes + candidates + emitted IR). Validate direct/recursive calls, references, local identities, ABI and repeated demand. |
| Template declarations and lookup | Declaration-owned parameter environments and defaults -> compatible redeclarations, explicit/deduced calls, recorded nondependent binding. Indexed lexical/associated edges; no global retry. Validate declaration order, shadowing, scopes and unused-body diagnostics. |
| Class specialization | Canonical arguments -> lazy member declarations/layout -> separately demanded bodies/static storage. Reuse PA11–13 class facts, conversions, lifetimes and virtuals. Validate dependent names/bases, nested/out-of-class definitions, completeness and unevaluated uses. |

## Performance acceptance

PA14 requires O0 LowIR, with no optional optimizer or student native backend.
Freeze the entry compiler before edits; use fixed common correct workloads for
A/A and ABBA compiler wall/RSS evidence, and B-only correctness costs for newly
implemented templates. Where supplied native execution is available, measure
checked runtime and text size separately. Preserve raw observations. Historical
PA13 diagnostic targets remain diagnostic, not additional stage gates; no
speedup claim from missing bodies or incorrect output. Later native/optimization
constraints retain their owning stages.

## Handoff ledger

- Entry inspection: clean HEAD above; no previous PA14 implementation turn exists
  in authoritative history. The preceding PA13 audit is verified progress.
- Function demand/identity increment: **140/314** (174 failures), prior
  **1621/1621**, file audit passes with the three inherited advisories.
  Personal function demand/declaration controls execute successfully via PA8.
  Parsed syntax is shared through compact source/context occurrences; one body
  is computed per canonical specialization. Parameter-head canonicalization and
  ABI consume typed identities. Local static objects remain outside the
  inherited PA10 boundary; the personal identity control uses local classes.
- Class registry/completion increment: **202/314** (112 failures), prior
  **1621/1621**. Class/default argument keys, immutable parameter overlays,
  lazy completion, member-body reuse, nested identity and template-aware ADL
  now feed existing layouts/lifetimes. Fixed bases are checked at definition;
  inherited class metadata is completed before constructor/virtual selection.
  The personal class-demand executable passes, including unused invalid bodies.
  Remaining architecture work: definition-time dependent/nondependent facts,
  dependent-only fact reuse, finer body occurrence allocation, and defaults/
  out-of-class-definition ownership. These are open requirements, not waived.
- Required checks pending after implementation: stage, through PA13/14, file
  audit, explicit personal controls and performance evidence.
- No handoff boundary reached; implementation continues across related groups.
