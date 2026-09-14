# PA15 implementation plan

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`

Target: full-stage PA15, implementation phase, O0 typed LowIR. Entry evidence:
56/177 passing, 121 failures; prior stages and file audit pass. Preserve all
177 fixtures, references and comparison rules.

## Design and remaining groups

| Group / owner | Data flow and complexity | Validation |
|---|---|---|
| Integral constants / semantic constant and query facts | Retained source -> typed expression/query -> cached constant -> assertion or template argument; one evaluation per complete fact identity, linear expression work | Arithmetic, conversions, short circuit, dependent assertions and value arguments |
| Value template arguments / template declaration and specialization owners | Typed parameter and canonical argument identities -> immutable substitution frame -> demanded declaration/body; average constant-time interning and lookup, work proportional to arguments and dependent uses | Class/function defaults, dependent parameter types, redeclarations, lookup and ABI |
| Packs / template argument partitions and occurrence expansion | Explicit pack boundaries in specialization keys; expand retained semantic patterns proportional to produced declarations/expressions | Multiple partitions, empty packs, sizeof..., call/body expansion |
| Explicit specialization / template fact owners | Indexed primary/argument key -> selected declaration and narrow dependent refresh; no global retry | Visibility, stale primary, inheritance, vtable and static members |

Extend related groups while the same ownership work supports further fixes.
Keep source parsing once, fixed-node sharing, canonical keys, bounded TU-owned
storage, and typed LowIR. Native generation and optimization remain later-stage
boundaries; PA15 adds no optional optimizer or numerical performance gate.

## Evidence and handoff ledger

- Initial inspection: PA14 type-only argument model is the main missing value
  template path. Constant evaluator also has signed shift/modulo rejection gaps.
- Performance pending: freeze binaries/flags/inputs; retain A/A and ABBA compiler
  latency/RSS observations and equivalent-output checks. Measure runtime/text
  through the supplied backend where executable controls are available, with
  no claim that the PA15 compiler has its own native backend.
- Implementation unfinished: all groups above; baseline is not a handoff.
- Independent review pending: whole-stage architecture and acceptance audit,
  including complete argument keys and demand boundaries. Review markers above
  remain unchanged throughout implementation.
- Required final evidence: `make test-pa15`, `make test-report-through-pa14`,
  `perl scripts/cppgm_file_audit.pl --stage pa15 --paths dev/src`; decreasing
  original failures (or full pass), committed intended changes and clean status.

### Constant evaluator increment

- Fixed signed quotient/remainder and left-shift overflow (including the separate
  unsigned modulo path); completed integral functional casts and ordinary
  multicharacter literal decoding at the LowIR language entry. PA2's explicitly
  narrower token-view contract stays intact. Restrict inline metadata to functions.
- Validation: 64/177 PA15 (+8 original cases), 1935/1935 through PA14,
  file audit pass (three inherited header warnings), ten explicit personal
  constant/query controls via `python3 student.tests/pa15/constants.py`.
- Raw checks: `$RALPH_ARTIFACT_DIR/pa15-value/constants-*.log`. Historical PA14
  preflight LowIR outputs are losslessly gzip-compressed after SHA-256 verification
  to recover scratch capacity; inventory in `historical-compression.json` there.
- Next: value argument identity/substitution. Performance and full handoff remain
  pending; this increment does not close the implementation turn.
