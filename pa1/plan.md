# PA1 implementation plan

Stage base commit: 1b05951a54c7803f3ac1a1db87b7213ec2b426f1
Last reviewed commit: 1b05951a54c7803f3ac1a1db87b7213ec2b426f1

## Design and ownership

- Source/translation cursor: immutable UTF-8 source, physical offsets, bounded
  lookahead; Unicode, UCNs, trigraphs, splices, final newline and raw reversion.
  Linear byte traversal; no full translated-source or token vectors.
- Token cursor: maximal munch, Annex E identifiers, pp-numbers, punctuation,
  comments, literals/suffixes and directive-local header context. Typed borrowed
  spellings and compact identifier IDs; flat TU-owned interning. Linear source
  work and expected linear interning; bounded delimiter/operator lookahead.
- PA1 adapter: stream typed events to the supplied debug output contract.
  Later preprocessing/parsing consume the same cursor, never serialized tokens.

## Remaining groups and validation

1. Translation/source, identity and all token/literal/header groups implemented;
   all 54 course fixtures pass. Extend personal checks for byte validity,
   ordering, raw reversion, EOF and identity/location lifetimes. Personal suite
   now passes 59 boundary/property cases plus typed cursor and telemetry checks.
2. Finish sanitizer validation, then root through-PA1 and file audit.
3. Measure fixed compiler workloads (latency/RSS, work counters); record flags,
   hashes, observations and limits. PA1 produces no executable, so generated
   runtime/text size and optimization claims are inapplicable. No speedup claim
   against the incorrect stub; any optimization comparison must use the spec's
   frozen A/B, A/A and ABBA protocol with equivalent output.

## Handoff ledger

- Entry: clean baseline, 0/54 passing (54 failures), no earlier stages.
  Previous state provides failure evidence; no running process to await.
- Core implementation: 54/54 course tests pass; file audit passes (24 files).
  Source is immutable, characters use an 18-slot ring, spellings borrow ranges
  unless translated, and identifiers/suffixes enter a TU-owned flat table.
  Remaining work: explicit boundary tests, measured compiler evidence and final
  through report. No incomplete handoff boundary claimed.
- Boundary closure: preserved the final logical newline after a trailing splice,
  treated BOM-only input as empty, and protected escaped backslashes from UCN
  recognition. Course remains 54/54; personal tests and file audit pass.
  Performance validation envelopes (not optimization claims): repeated-name
  workloads must keep identifier storage constant; scanner decoding work must
  stay <= 2 * source bytes + 64; 4x source should use <= 6x latency after noise,
  and peak RSS <= 4 * source bytes + 32 MiB. No emitted-program budgets apply.
