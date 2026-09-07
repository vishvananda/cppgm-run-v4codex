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

1. Implement translation/source and identity owners, with explicit personal
   tests for byte validity, ordering, raw reversion and lifetime boundaries.
2. Implement token/literal/header groups together; run all 54 course fixtures
   and personal boundary cases, then root through-PA1 and file audit.
3. Measure fixed compiler workloads (latency/RSS, work counters); record flags,
   hashes, observations and limits. PA1 produces no executable, so generated
   runtime/text size and optimization claims are inapplicable. No speedup claim
   against the incorrect stub; any optimization comparison must use the spec's
   frozen A/B, A/A and ABBA protocol with equivalent output.

## Handoff ledger

- Entry: clean baseline, 0/54 passing (54 failures), no earlier stages.
  Previous state provides failure evidence; no running process to await.
- Current: implementation in progress. No incomplete handoff boundary claimed.
