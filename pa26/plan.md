# PA26 implementation

Stage base commit: 369c57f19fa13c0394d4fb0345cf2bb79708fc67
Last reviewed commit: 369c57f19fa13c0394d4fb0345cf2bb79708fc67

Target: PA26 full-stage. Phase: implementation142. Entry: 0/30 passing;
prior PA1–25 report 4253/4253. No independent PA26 review yet.

## Design and remaining groups

| Owner / group | Data flow and bounded work | Validation |
|---|---|---|
| Object writer / host runtime | Existing typed symbol roles and native fixups -> direct ELF sections, symbols and relocations. Linear bytes/fixups plus symbol partition; no assembler or source reconstruction. | Host link, runtime imports, PIE relocation inspection; previous driver tests. |
| Native EH | LowIR regions/clauses -> per-function MIR handler facts -> final machine call ranges, CFI and LSDA. Function-local state; bounded indexed work, sparse ranges and shared resume terminal. | Same/cross-TU catch, cleanup, callee-save unwind, selectors and sparse/coalescing facts. |
| Frontend lifetime boundaries | Recorded lifetime actions -> balanced protected regions at control joins, shared terminate action and local generated-function binding. | Branch/loop/conditional/cleanup tests and local binding inspection. |

Production keeps typed LowIR and per-function MIR; external text views remain
adapters. Preserve PA25 private compilation/link compatibility while introducing
host ELF output; settle the format-selection boundary before final acceptance.
No optional optimizer is planned. Necessary host ABI/metadata costs are subject
to stage-scoped acceptance, not inherited blanket zero-growth diagnostics.

## Performance and handoff ledger

Freeze entry/final compiler binaries and workloads; measure compile latency/RSS,
execution and text size with A/A and ABBA observations. Compare equivalent
working paths, and identify new semantics where entry host output cannot run.
Record raw evidence and exact commands under student.tests/pa26.

Implementation unfinished: all three groups above. Independent review questions:
whole-stage typed data flow, format compatibility, CFI and LSDA legality, and
performance evidence remain unaudited. These are not waived by an implementation
handoff. No handoff boundary has been accepted yet.
