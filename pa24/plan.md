# PA24 implementation handoff133

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: 9f3f9b5caaec9677af0e8431b51cacebcf823dce

Entry was clean `18721ea79d6283dda2351101f2c50b54cac7cf10`. The preceding turn
made verified TLS progress; no process remained live. Implementation commits are
`1d73b01e` and `e5b5dada`. **All 296 course tests now pass.** This is the
implementation handoff; whole-stage independent review remains pending. Preserve
both review markers and do not advance to PA25 before that review.

[Audit130](audit.md), [validation133](../student.tests/pa24/validation133.json) and
[performance133](../student.tests/pa24/performance133.md) retain evidence.

## Design/spec alignment

Typed PA8 Program -> function-owned placement/flat MIR -> direct x86 encoding,
typed fixups and ELF. Explicit text adapters remain at tool boundaries; neither
MIR nor assembly transports production phases. Unit-owned dense identity indexes
survive across functions; function facts, call plans and MIR release after use.
No reference, host compiler or assembler implements output. Earlier scalar,
floating, wide, object, phi, atomic, variadic, EH/frame and TLS groups are retained.

133 closes the six canonical placement/frame failures as one ownership group:

| Owner | Data flow and completed behavior | Work/lifetime |
|---|---|---|
| Placement | Typed signature/value intervals -> scalar stack parameter homes, conservative mixed-conversion FP homes and GPR pool, safe adjacent ABI result carriers | Existing linear walks, fixed register pools, per-function facts |
| Call setup | Shared ABI fragments -> register/address read masks, GPR/XMM transfer order, safe indirect target retention, early dependent or late independent stack stores, stable stack-call result homes | O(arguments); at most fourteen register assignments and bounded cycle scheduling; call-local storage |
| Boolean selection | Compare's exact 0/1 fact -> representation-preserving scalar conversion aliases, full use intervals, canonical materialization and retained adjacent debug steps | O(values/uses); one-bit/wide conversions retain their explicit representation changes |
| Frame finalization/encoding | Actual homes/preserve/scratch facts plus returning-void parameter capacity -> final stack size consumed by prologue/epilogue and dump | One linear finalization walk; no duplicated body or output-only metadata |

Stable ABI homes are excluded from private scratch-carry windows. Incoming
register fallback is enabled only for the existing proven parameter-flow policy;
a newly marked ordinary spill must not regain an already reused incoming carrier.
The new controls cover that interaction across pressure, calls, CFG and widths.

Canonical fixtures require conservative mixed numeric placement and the lowered
void object-output frame. The first implementation reserved parameter capacity
more broadly; measurements exposed unnecessary leaf frames. Final code narrows
that policy to returning void boundaries. Scalar-return/nonreturning functions
use actual frame requirements. This preserves mandated shapes without imposing
the extra reservation on unrelated functions. Occupied frames retain ordinary
Boolean placement; an otherwise empty converted-return path uses its canonical
fixed carriers/scratch policy. All these facts reach real encoding.

Total work remains O(N + E log E), including inherited sorted phi edges and fixed
reload windows. There is no new fixed point, global invalidation, semantic
reconstruction, body cloning or optimization level. All fallback storage and
instruction growth are linear; per-call register scheduling has a fixed bound.

## Validation and performance

- Required checks ran sequentially: `make test-pa24` **296/296** plus **17/17**
  focused properties; prior-through **3856/3856**; root through-PA24 **4152/4152**.
  File audit passes with the same four inherited substantial-header warnings.
- Original required failures decrease **6 -> 0**, with no new failures. All 2403
  tracked fixture files, references and comparators are unchanged. The inventory
  convention moves **290/435 -> 296/435**: 125 solution regressions are excluded
  and 14 control inputs report their properties separately. No coverage is waived.
- Final personal runs: **205 placement**, **1820 scalar**, **1259 floating**,
  **1495 wide**, **160 object**, **75 EH/frame**, **236 TLS**, **21 audit** controls;
  parameter flow and scalar/TLS ABI/debug/ELF integration also pass. Early invalid
  personal inputs and rejected diagnostics remain in evidence; final inputs pass.
- `trace133.cpp` follows `Mixer<7>::value` and demanded `combine` through source
  facts, typed LowIR, MIR and ELF. One class completion, one demanded template
  region, two emitted functions, **214 text bytes**; the unused dependent member
  stays dormant. Both inputs and ELF/disassembly checks pass. Production source
  driver integration and self-hosting remain later-stage work.
- Final frozen A/A + six ABBA blocks: compiler paired B/A medians
  **1.003 / 1.009 / 1.011 / 1.010** (scalar/float/wide/ABI); RSS stable or lower.
  The indirect-call workload is **3.4% faster**, **213 -> 207** text bytes.
  Required mixed-ABI shape costs **8.1% runtime**, **248 -> 279** bytes. Eight
  inherited runtime images are byte-identical to entry, with both inputs rechecked.
- The inherited 15% compiler latency/RSS and zero optional text-growth targets
  remain diagnostic. Final medians meet the compiler targets; mandatory canonical
  costs are disclosed separately. All observations, including initial regressions,
  spreads/outliers and the corrected frame-policy measurements, are preserved.
  Correctness, required MIR comparisons/envelopes and finite work bounds remain
  mandatory. Implementation evidence does not substitute for independent review.

## Handoff boundary and ledger

**Unfinished implementation:** none identified for the PA24 course handoff. The
remaining six shape failures are closed, related placement consumers pass, and
required checks are green. No known defect in this group is relabeled as review.

**Independent review:** still required for 131 EH nesting/value ownership and
stack/alignment restoration; 132 TLS layout/accessor demand and address consumers;
133 placement/ABI-home dependencies, Boolean/debug facts, conservative canonical
policies and performance evidence. Ralph must audit the accumulated whole stage
against the handout/spec and resolve any findings before advancement. The review
markers above remain unchanged; passing tests do not waive that audit.

| Handoff | Code / evidence | Disposition |
|---|---|---|
| Audit130 | through `9f3f9b5c`; audit.md | Independently reviewed; 14 failures remained |
| 131 | `45774fa2`, `c7587f72`; validation131/performance131 | EH/frame complete; 9 failures; independent review pending |
| 132 | `db965a86`, `03eeff9e`; validation132/performance132 | TLS/address complete; 6 failures; independent review pending |
| 133 | `1d73b01e`, `e5b5dada`; validation133/performance133 | Placement/frame complete; zero course failures; whole-stage independent review pending |

Evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa24-133/`.
