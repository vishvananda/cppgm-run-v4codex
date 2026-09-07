# PA2 final architecture audit

Scope: **PA2 full stage**, through tokenization in translation phase 7.
Independently reviewed the working tree at `93c5baabf`, all stage commits since
`be4bff26f`, the handout, unchanged fixtures, `spec.md`, and the local N3485
literal rules. The last implementation change is `ceaa2dd1b`. This final audit
changes records only; the measured compiler remains byte-identical.

The incoming checkpoint represents progress: its implementation, commits and
measurements exist and were verified. It was not accepted as proof of this
audit. Fresh required checks passed, all 168 historical observations were
independently recomputed, and a fresh 84-observation campaign completed. No
live process from the previous checkpoint required continuation. Stage entry
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

## Final Spec Alignment

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

## Representative data flow

For a declaration/template spelling, `template<class T> struct Box { T value; };`
is read once into immutable source; PA1 interns `T`, `Box`, and `value`; PA2
classifies punctuation/keywords and passes name identities directly. For
`"a" u"𝄞"_tag`, one pending PP token ends the sequence, `_tag` survives table
growth by ID, and the typed result is an array of four `char16_t` units. The
PA2 writer consumes these bytes; production callers can omit the joined source
entirely. Neither trace has a second token graph or textual phase transport.
Appending `Box<int> box;` exercises only the same lexical paths: there is no
grammatical parse or specialization demand at this stage. A spliced `bo` plus
backslash/LF plus `x` keeps a physical range across the splice and still has
`box` identity; transformed spellings use the PA1 scratch buffer.

`0xffffffffffffffffu` traverses PP-number recognition, base-16 grammar scan,
suffix validation and checked accumulation to UINT64_MAX. The candidate table
selects `FT_UNSIGNED_LONG_INT`; the eight inline bytes are FF. The writer adds
the type name and hexdump. A decimal value exceeding signed 64-bit range is
invalid unless its suffix admits unsigned values; an enormous `999..._tag`
instead retains its full prefix and interns `_tag`. The independent Python
candidate oracle verifies 7,062 types/values at these boundaries.

The API trace `"\x3c0" /* comment */ u"𝄞"_π` followed by spliced `alpha`
exercises deferred encoding and transformed lookahead together. The reader
tags 0x3C0 as numeric and U+1D11E as a scalar; the late `u` selects UTF-16.
Conversion emits `C00334D81EDD0000`: four code units including the terminator.
The suffix is a stable ID. The spliced `alpha` ends the maximal sequence and
remains borrowed as the only pending PP token until the next pull. The API
check verifies those bytes and the pending spelling with joined-source
construction disabled; the CLI separately constructs the required display.

Raw strings enter inherited raw mode just after the opening quote: only bounded
speculative lookahead is discarded. Physical trigraphs, splices and apparent
UCNs remain contents while UTF-8 validation continues. `LiteralReader` skips
the delimiters and encodes scalars. Ordinary numeric escapes instead produce
one code unit, with sticky overflow. Conflicting encodings/suffixes invalidate
the whole maximal sequence and recover at its next non-string token. Long
raw near-matches, late encoding, escape overflow and subsequent-token recovery
are covered by personal checks and the fixed compiler workloads.

Two course boundaries are intentional. Empty characters are recoverable only
when the PA2 entry enables `recover_empty_character`; PA1's default still
rejects them lexically. Fixture `750-reserved-literal-operator-suffix` requires
`operator""sv` to emit an empty string and an identifier. N3485 §13.5.8 example 8
instead identifies adjacent `""E` as one token. The fixture owns PA2's behavior
here: splitting applies generally after `operator` to an empty ordinary string
plus any suffix, never to a recognized library spelling. Physical suffix
coordinates come from PA1, including after UCN transformation. This is a course
contract adaptation, not a claim about that draft's grammar.

## Ownership, validity and pipeline budgets

- Source bytes never move after publication. PP/post-token spelling views are
  conservatively valid until the next cursor pull. Name IDs survive table
  growth; spelling views are reacquired and never kept over insertion. Each
  string part is decoded before fetching another PP token. Pending transformed
  tokens are returned before calling the PP cursor again.
- Scalars live inline in the returned token. String elements, code-unit bytes
  and optional display spelling belong to the post cursor. Logical scratch is
  cleared at the next string sequence; high-water capacity is reused until
  cursor destruction. The largest sequence, not accumulated token count,
  determines this storage. TU destruction and exception unwinding release
  cursors, table and source in reverse ownership order.
- Decoding precedes final encoding because a later prefix can make an early
  numeric escape representable. Each element is decoded once and encoded at
  most once. Invalid sequences consume tokens without allocating more unusable
  elements. These bounded passes avoid repeated concatenation rescans; there
  are no fixed-point transforms or optimization levels at PA2.
- Legality rests on full grammar, checked arithmetic, scalar/code-unit identity
  and maximal-sequence agreement. Omitting joined spelling is legal only for
  consumers that do not request that view. Scratch reuse invalidates only
  borrowed views. No semantic analysis cache or executable transform exists;
  ABI-call, debug, spill and loop obligations are later-stage work.
- The warmed integer/character/string path makes zero allocation calls in the
  API check. That assertion does not cover host floating-stream internals.
  Floating extraction uses stack-owned borrowed input and the required starter
  algorithm; x87's six padding bytes are zero. Optional range rejection is
  omitted as permitted. Capacity/growth counters are not all host mallocs.
- For N source bytes the fixed gates allow `2N+64` decoded source units, N
  numeric/literal bytes, N decoded elements, and `8N+4` encoded string bytes.
  Post scratch is bounded by `28N+64`: geometric 8-byte elements, optional
  joined spelling and 1/2/4-byte code units. These workloads permit at most 256
  growth events. These passes add bounded frontend work and no generated text.
- Source capacity is bounded by `2N+65536`, spelling by `2N+15`, and flat names
  by `64U+2N+64` for U identities. The `8U+64` rehash-probe gate applies to fixed
  workloads, not adversarial worst-case hashing. Peak RSS must meet both
  `40N+32 MiB` and twice total retained capacities plus 32 MiB, accounting for
  growth overlap and runtime overhead. Fourfold repeated source must take at
  most 6x wall time in both ordinary and telemetry variants.

These are resource ceilings, not optimization-profit claims. The long raw case
uses up to 78,176 KiB (76.3 MiB) peak RSS for 4.46 MB of input because it retains decoded
elements and output with geometric capacity. That cost is reported and meets
the ownership budget; the smaller repeated-token footprint does not describe
all workloads. No extra optimization work or larger budget is justified by the
measurements. No executable runtime benefit is inferred from token/node counts.

## Review disposition

The independent API, numeric oracle, resource and sanitizer checks supplement
the unchanged course tests. They found a telemetry undercount when the character
decoder read the second element of a rejected multicharacter literal. That read
is now counted, with a focused test. Runtime token output was unaffected.
The independent final audit found no further compiler correctness, ownership,
self-containment, timeout or resource defect. It corrects the stale review
marker and clarifies the course/draft distinction above. No behavioral refactor
is warranted. The entry point and shared sources invoke no external process;
the course runner's ordinary CLI calls `test_runner_real_main` directly, and
standalone sanitizer tools omit that runner entirely. All eight shared sources
are registered for `posttoken`. Course fixtures and references are unchanged.

## Handoff ledger

| Commit since the previous review marker | Independent disposition |
| --- | --- |
| `85d89e23e`: plan and 0/26 baseline | Read against the actual inherited implementation and stage requirements. Superseded its stale review boundary in the final compact plan. |
| `e08732384`: streaming PA2 and PA1 adapter changes | Read all shared source paths and registration; traced classification, numbers, characters, strings, lookahead and output. Fresh full-stage and inherited tests pass. |
| `ceaa2dd1b`: personal checks, initial audit and counter fix | Verified second-element counting on rejected characters; reviewed and reran numeric, encoding, resource, identity/location and sanitizer checks. |
| `93c5baabf`: completion/performance consolidation | Independently checked binary/build-source/harness/input identities, ordered samples, paired calculations and all budgets; fresh campaign uses the same compiler and seven frozen inputs. |

No PA2 handoff through `93c5baabf` remains unaudited. This final record and the
performance/plan consolidation introduce no further compiler change. Future
semantic/template/native consumers remain later-stage work, not unverified
capabilities claimed by this audit. This audit does not advance PA3.

## Performance and final validation

The [performance report](../student.tests/pa2/performance.md) retains **252 timed
observations** across three campaigns. The fresh CPU-0 campaign freezes the
same binary as A (ordinary) and B (`--stats`) and all seven original inputs.
It records two A/A pairs and two ABBA blocks per workload, with complete output
hash equality before timing. Wall time includes startup/read/conversion/output
formatting; `/usr/bin/time` measures fresh compiler-process peak RSS. Every
invocation has a 60-second timeout. The fresh benchmark ran without a concurrent
compiler or test build.

Fresh ordinary medians are 0.758290/3.025535 s and 8024/20312 KiB peak RSS for
4/16 MiB repeated source, scaling 3.989943x (ordinary) and 3.948158x (telemetry).
Name storage stays 618 bytes and post scratch 30 bytes at both sizes. Floating,
self-source, 200k suffixes, raw near-matches and late-encoding concatenation all
pass their work/memory gates. A/A changes reach 7.717%; fresh paired telemetry
changes range from -0.649% to +2.288%. All positive costs and spread are visible;
no speedup is claimed. Host-tool text remains **83,290 bytes**. Generated runtime
and text size are **N/A**, because PA2 emits tokens.

| Final check on the audited implementation | Result |
| --- | --- |
| `perl scripts/cppgm_file_audit.pl --stage pa2 --paths dev/src` | PASS, 35 files |
| Additional `--paths dev/src,dev/posttoken.cpp` file audit | PASS, 36 files |
| `make test-pa2` | PASS, 26/26 |
| `make test-report-through-pa2` | PASS, 80/80, both stages |
| PA1/PA2 standalone ASan/UBSan tools, all course fixtures | PASS, 54/54 and 26/26 |
| PA2 normal and sanitizer personal checks | PASS, 316 cases each including 7,062 integer types/values, plus cursor API checks |
| PA1 sanitizer personal checks | PASS, 64 cases plus cursor API checks |
| Historical/fresh benchmark identities, observations and resource gates | PASS, 252 observations, none discarded |
| Fixture/reference diff from stage base; `git diff --check` | Unchanged; PASS |

The [compact plan](plan.md) names the independently reviewed checkpoint. Only
audit records and performance evidence are committed by this final audit;
generated binaries, objects, logs and `.my*` outputs remain ignored.
