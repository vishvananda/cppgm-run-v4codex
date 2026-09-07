# PA2 completion audit

The course handout, unchanged checked-in fixtures and current `spec.md` own
requirements. `pa2/plan.md` preserves both initial review markers. Stage entry
was 0/26; completion requires the entire 26-test suite and inherited 54 tests.

## Behavior and data flow

| Requirement | Implementation owner and evidence |
| --- | --- |
| PA1 phases 1–3, phase 4 no-op | Existing `SourceBuffer`/`CharacterCursor`/`PPTokenCursor`; PA1 54/54 and 64 personal cases, also under ASan/UBSan |
| Keyword/operator/identifier/invalid classification | `posttoken/token_types` immutable vocabulary; `PostTokenCursor::next`; 100-simple and personal invalid/header tests |
| Integer grammar, base, legal suffix and first fitting ABI type | `posttoken/number`; monotonic grammar scan and overflow-checked unsigned accumulation; course limits plus 7,062 Python candidate-table results |
| Decimal floating grammar, suffix and bytes | `number` validates the entire prefix/suffix before PA2Decode stream extraction; borrowed input stream buffer follows the starter extraction method; float/double/x87 personal bytes and course cases |
| User-defined numbers | Grammar yields an unconverted prefix and TU-interned suffix, without range rejection; Unicode, exponent-like suffix, huge magnitude and malformed-shape controls |
| Characters | `LiteralReader` plus `decode_character`: one Unicode scalar, ordinary char/int choice, UTF-16 single-unit fit, UTF-32/wchar values; numeric overflow and multichar rejection |
| Strings, raw restoration and phase-6 maximality | PA1 owns raw restoration; `LiteralReader` decodes once, tagging numeric escapes; cursor collects only one sequence, selects encoding/suffix by identity, emits UTF-8/16/32 or numeric code units and one terminator |
| Invalid sequence recovery and required output | All sequence parts are consumed, even after invalidation; optional joined source view is separate from typed data. PA2 output adapter formats bytes and ABI type names, with eof |
| Empty characters and reserved literal-operator suffix | PA2 enables scoped empty-character recovery without changing PA1's default. Immediate `operator` context splits empty ordinary string plus suffix, using physical suffix coordinates; course boundary fixtures and transformed-suffix API checks |

Numeric hex accumulation stays bounded even with arbitrary leading zeros or
overflowing tails. Adjacent strings are encoded only after the final prefix is
known: an early ordinary `"\x3c0"` can become valid when a late `u""` selects
16-bit code units. Unicode characters instead encode to one or more units.
Suffix conflict uses interned IDs. Rejection returns a typed invalid token;
only lexical/I/O errors use exceptions. Optional floating range rejection is
not implemented, as permitted by the handout. No output is delegated to another
compiler, reference binary, cached answer or fixture-specific branch.

## Current-stage spec alignment

- Sections 1–2: immutable source and compact locations flow into trivially
  copyable post-tokens. Identifiers and suffixes have stable TU-owned IDs;
  fundamentals and simple tokens have enum identity. There is no owning token
  sequence, token serialization between phases or semantic key made from output.
- Sections 5 and 8: existing flat identifier storage owns names until TU release;
  no new hot maps, per-token nodes, shared ownership or global mutable cache.
  Post-token scratch owns only the largest active string's decoded elements and
  bytes, clearing logical contents at the next sequence and releasing capacity
  with the cursor. Only the explicit PA2 view requests joined spellings.
- Section 9: PP scanning, numeric validation and string conversion do bounded
  passes proportional to consumed/produced bytes. Raw delimiter lookahead is
  bounded by PA1. Vocabulary search uses bounded read-only metadata; suffix
  comparisons use O(1) IDs. Geometric storage and one PP lookahead avoid repeated
  concatenation rescans. Actual work/capacity counters and a warmed-path
  allocation test support the ownership claim. [Performance evidence](../student.tests/pa2/performance.md)
  records compiler latency/RSS, noise, scaling budgets and all observations.
- Sections 3–4 and 6–7: parsing, lookup, specialization demand, typed IR, native
  optimization and direct ELF have not been introduced at PA2. No claim is made
  about runtime profit, executable size or later semantic work. Future consumers
  use typed post-tokens and copy only demanded literal values to their arenas;
  they need not request the debug spelling or parse its output.
- Section 10: implementation sources are registered for `posttoken`, reuse PA1
  code, retain attribution, and invoke no host/reference compiler at runtime.

For a declaration/template spelling, `template<class T> struct Box { T value; };`
is read once into immutable source; PA1 interns `T`, `Box`, and `value`; PA2
classifies punctuation/keywords and passes name identities directly. For
`"a" u"𝄞"_tag`, one pending PP token ends the sequence, `_tag` survives table
growth by ID, and the typed result is an array of four `char16_t` units. The
PA2 writer consumes these bytes; production callers can omit the joined source
entirely. Neither trace has a second token graph or textual phase transport.

## Review disposition

The independent API, numeric oracle, resource and sanitizer checks supplement
the unchanged course tests. They found a telemetry undercount when the character
decoder read the second element of a rejected multicharacter literal. That read
is now counted, with a focused test. Runtime token output was unaffected.
No known PA2 behavior group remains incomplete; final command evidence and
commits are recorded in the compact plan. The initial stage/review markers are
intentionally unchanged, as requested by the implementation workflow.
