# PA31 implementation plan

Stage base commit: `c0566ded7ed123e1eb0696f0ae93b28ceb492fdf`.
Last reviewed commit: `c0566ded7ed123e1eb0696f0ae93b28ceb492fdf`.
Target: **PA31 full-stage**. Phase: **implementation205**.

## Design and remaining groups

Entry evidence: 78/84 required tests pass; six failures. Preserve all fixtures,
comparison rules and inherited PA30 audit/evidence. The preceding goal turn was
interrupted; the clean worktree and terminal log establish the current baseline.

| Group / owner to investigate | Data flow and complexity requirement | Validation |
|---|---|---|
| Imported globals and PC-relative data / native object selection and ELF relocations | Typed symbol ownership → addressing choice → relocation; linear per reference | Two failing relocation smokes, object inspection and reduced controls |
| Ostream constructor entries and virtual-base references / ABI entry and subobject lowering | Recorded layout and ABI identity → required entry closure → LowIR/native; bounded per demanded entity | Two failing ostream smokes, multi-TU symbols and reduced controls |
| Recursive map ownership / template fact demand | Canonical specialization and member facts → demand closure; no global retry | Self-element map/unique_ptr failure and focused demand controls |
| Braced vector temporary destruction / lifetime lowering | Semantic initialization and cleanup facts → LowIR cleanup → execution; linear per lifetime action | Failing temporary-dtor smoke and lifetime controls |

Keep the production typed pipeline and source-derived ABI ownership. Extend
related fixes while their owner is understood; no hosted names or fixture cases.
Record established ownership, changes, complexity and validation as work lands.

## Performance evidence

Freeze entry/final binaries and inputs. Collect compiler wall latency/peak RSS
and checked executable runtime/text size with A/A and ABBA observations. Apply
spec §9 stage-scoped acceptance: necessary correctness costs are documented;
optional transforms require useful measured benefit and explicit bounds.
Preserve mandated timeouts and inherited measurements; no new optimization
level or self-hosting acceptance is introduced at PA31.

## Handoff ledger

Unfinished implementation: all six entry failures, grouped above, pending
reproduction and ownership diagnosis. Independent review: whole-stage spec and
architecture audit remains with Ralph; neither review marker is advanced by
implementation. Required exit: PA31 failure reduction without coverage loss,
through30 pass, file audit pass, recorded boundary and clean committed changes.
