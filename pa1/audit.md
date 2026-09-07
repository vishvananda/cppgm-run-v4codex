# PA1 final architecture audit

Scope: PA1 full stage, translation phases 1–3 and the explicit `pptoken` view.
Audited baseline: `dccd027f1` (implementation `78a0872d6`), independently read
from the working tree and the changes since `1b05951a5`. The incoming clean
status and 54/54 checkpoint were revalidated; no running job needed continuation.
The final implementation is the identifier-table fix in `29e95571f`; no compiler
changes follow it in the accompanying record/benchmark consolidation.

## Final Spec Alignment

| Spec surface | Actual implementation and conclusion |
| --- | --- |
| Production pipeline; §1 source and cursor | `pptoken.cpp` moves its input string into an immutable `SourceBuffer`. `CharacterCursor` feeds `PPTokenCursor::next()`. The writer consumes each typed token immediately. No translated-file array, owning token vector, or text roundtrip occurs. |
| §2 canonical identity | TU-owned `IdentifierTable` hashes bytes only at the interning boundary, then publishes 32-bit IDs. Hash, length and byte equality resolve collisions. Reallocation changes storage addresses, never IDs. Names and literal suffixes use the same table. |
| §§3–5 lookup, demand and caches | PA1 has lexical include context and identifier interning only. The four-state directive machine observes logical newline/token kinds. Table hits now perform no growth. No semantic lookup, specialization, worklist or mutable global cache exists yet. |
| §§6–7 typed lowering and native optimization | No parser, semantic graph, LowIR, MIR, allocator, encoding or ELF emission is present in this PA. Declaration/template and optimization traces below end at the actual token boundary. No generated-code or runtime benefit is inferred. |
| §8 allocation and lifetime | One source owner, one flat TU name arena, one reusable transformed-token buffer, and an inline 18-character ring. Tokens are trivially copyable views. No hot per-token allocation, shared ownership, recursive destruction, or duplicated stage graph. |
| §9 complexity and evidence | Bounded lexical lookahead; geometric source/name/spelling growth; expected linear interning. Optional phase times, RSS and work/capacity counters observe existing work. Frozen A/B, A/A and ABBA evidence is recorded in the performance report. |
| §10 self-containment | The PA1 executable links the four registered preprocess sources and the supplied test wrapper. The wrapper's ordinary CLI calls the implementation directly. No implementation code invokes a process, reads references, or recognizes fixtures. Standalone sanitizer builds omit that wrapper. |

The untouched legacy `IPPTokenStream`/`DebugPPTokenStream` headers are supplied
output interfaces, not a second token graph or production transport. Future
tools must call the shared cursor and add its sources to their tool lists.
The unused later-stage skeletons do not establish any later-stage capability.

## Representative end-to-end traces

1. `template<class T> struct Box { T value; };` followed by
   `Box<::Tag> bo` + backslash/LF + `x;` traverses source bytes, UTF-8 decoding,
   phase-one mapping, splicing, token recognition, interning and the PA1 writer.
   The declaration's repeated `T` and `Box` have canonical IDs. `<::Tag` emits
   `<`, `::`, `Tag` by the language exception; the instance variable emits
   `box` with a physical source range spanning the splice. Unchanged tokens
   borrow source bytes, while `box` uses scratch storage. The personal oracle
   checks the complete token sequence. There is no demanded specialization at
   this stage: template parsing, semantic demand and ELF remain later PAs.
2. `%:/**/inc??/` + LF + `lude /*x*/ <a//b> "c"` translates the trigraph and
   deletes its splice before recognizing `include`. Comments contribute one
   whitespace token and preserve directive state. `%:` enters `after_hash`,
   `include` enters `after_include`, and the first angle form becomes a header
   token whose `//` stays inside its spelling. The following quoted token is a
   string because the header already consumed the directive context.
3. A raw string with a 16-character delimiter, near-matching closing sequences,
   trigraphs, apparent invalid UCNs and physical line splices enters raw mode
   immediately after the initial quote. Mode entry discards only speculative
   characters and restores the consumed physical position. UTF-8 validation
   continues, while UCN/trigraph/splice transformations are disabled. Matching
   the physical closing quote restores translation before a UCN suffix is
   interned. Escaped backslashes in ordinary strings similarly suspend UCN
   recognition for exactly the escaped character. The cursor checks identities,
   locations, suffixes and subsequent tokens across these transitions.

`SourceCharacter` carries physical byte begin/end, line and byte column through
all mappings. The BOM is omitted at input start. Synthetic final newlines have
zero-width physical ranges; empty/BOM-only input has none. Trailing physical
splices retain the required final newline. Comments are skipped without a
spelling buffer; internal block-comment newlines are part of that comment's
whitespace, while physical positions still advance.

## Ownership, validity and work budgets

- `main` owns the source and identifier table through the last token. Cursor
  destruction releases scratch state first, then the table and source release
  their flat storage, including on a lexical exception.
- `PPToken::spelling` is conservatively valid until the next cursor call.
  Source-backed bytes remain immutable, but transformed views must not be
  retained across calls. Name IDs survive TU table growth and separate source
  lifetimes; table spelling views must be reacquired after insertion. Deferred
  token consumers must provide explicit storage when they arrive in later PAs.
- Raw lookahead is at most 18 code points; delimiter comparisons inspect at most
  16 characters. Punctuation inspects at most four characters. Mode changes
  restore only unconsumed lookahead, not an earlier source region. Splice runs
  consume physical bytes even when they produce no code point.
- Spelling borrowing is legal exactly when every consumed character is unchanged
  and contiguous. At the first transformation/gap, `take()` copies the preceding
  prefix once and appends translated UTF-8. This conservative fallback preserves
  the complete spelling. Scratch grows geometrically and retains only its high
  water capacity; source positions and IDs are unaffected.
- The table stays at most half full. Insertions alone can rehash, at geometric
  thresholds; ordinary name hits cannot invalidate any storage. Entry equality
  is checked before the profitability decision to avoid a redundant allocation.
  Rehashing rebuilds only slot indices and preserves entry IDs/offsets. There is
  no semantic-cache invalidation or output/code growth.
- With `B` source bytes and `U` unique names, retained identifier capacity is
  bounded by `64*U + 2*B + 64` bytes for this host: entry capacity below `2*U`
  times 24 bytes, slots below `4*U` times 4 bytes, and geometric spelling bytes.
  Source capacity is at most `2*B + 65536`; scratch is at most `2*B + 15`.
  Peak RSS permits twice the total retained capacities plus 32 MiB for growth
  overlap, allocator and process overhead. This counts actual ownership instead
  of assuming unique names cost the same as repeated names.
- Fixed-workload gates additionally bound decoded work by `2*B+64`, measured
  rehash probes by `8*U+64`, and each fourfold workload scaling step by 6x latency.
  The probe bound is an empirical gate on frozen workloads, not a worst-case
  guarantee for adversarial collisions. Hash lookup has expected O(1) probes;
  byte hashing/comparison tracks spelling lengths. No stronger bound is claimed.
- The original five workload envelopes remain historical evidence. A universal
  `4*B + 32 MiB` RSS claim would be incorrect for millions of unique names;
  the final audit adds dense-name workloads and the ownership-based envelope.
  Repeated, translated, raw and long-token workloads still use the byte envelope.

`storage_growths` counts identifier storage growth events, not every host malloc.
`intern_probes` counts occupied lookup slots; `rehash_probes` counts inspected
rehash slots, including the final empty slot. Capacity counters cover the other
large allocations. `read_ms` and `scan_emit_ms` separate source reading from
scanning plus required formatting. `getrusage` is a process high-water mark and
can include inherited pre-exec RSS; benchmark comparisons use `/usr/bin/time`'s
fresh child measurements, not the output-hashing warmups' RSS or phase times.

There are no optimization levels or executable transforms in PA1. Source
preservation, maximal munch, Unicode and raw-mode rules supply lexical legality;
avoiding table growth improves retained storage without changing tokens. All
later ABI/debug/loop/spill and optimization work/growth obligations remain
explicit later-stage scope, with no claimed validation from fewer tokens/nodes.

## Findings and changes

| Finding | Resolution and evidence |
| --- | --- |
| A completed name triggered table doubling at half occupancy. | Reproduced by filling eight IDs and reinterning the eighth: the storage assertion failed on the baseline. Moved occupancy growth after the miss check. The same check passes ordinarily and under ASan/UBSan; the ninth new ID still grows normally. |
| Growth work was missing from probe telemetry. | Added a separate rehash counter at the table owner; exposed source and scratch capacities without an extra analysis or production representation. |
| Existing evidence omitted long-token/raw-near-match and dense-name scaling. | Added explicit 2 MiB boundary checks and fixed 8 MiB benchmark cases, a capacity-boundary hit workload, and 200k/800k/3.2M unique-name scaling. |
| Review marker still named the unimplemented baseline. | Replaced the stale plan marker and recorded every intervening handoff below. |
| Benchmark warmups/timing had no timeout. | Added a 60-second watchdog to hashing and a 60-second process-group timeout to measurements. Frozen A/B source/build/harness identities and reusable input manifests make comparisons reviewable. |
| The comparison harness's repeated-input scaling gate checked A only. | Both A and B are now checked for every scaling pair. All prior audit observations were independently recomputed against both variants' final work/memory/scaling gates; all pass. |

No course fixture, reference, harness contract or expected output was changed.

## Handoff ledger

| Handoff since the previous marker | Independent disposition |
| --- | --- |
| `fee23cc04`: initial architecture and 0/54 baseline | Read alongside the actual baseline and assignment contract; superseded by the final architecture map. |
| `a03163343`: all shared PA1 implementation | Read every new source/header, entry-point replacement and source-list registration. Traced representative paths above. |
| `78a0872d6`: EOF, BOM and escaped-UCN boundaries | Read the changes and reran the independent identity/location, syntax and sanitizer checks. |
| `dccd027f1`: frozen performance evidence and checkpoint | Verified the archived binary equals the incoming binary by SHA-256; reviewed protocol, observations, outputs, flags and input hashes. Extended coverage and corrected the scope of the memory envelope. |
| `29e95571f`: final implementation changes | Table-hit growth fix, telemetry and resource cases reviewed and validated. The following consolidation adds benchmark controls and audit records, with no compiler change. No unaudited PA1 handoff remains. |

## Performance and final validation

The [performance report](../student.tests/pa1/performance.md) retains all 360
observations from two frozen before/after campaigns and a final telemetry
campaign, plus the historical 60-observation baseline. Each campaign uses ten
inputs, two A/A pairs and two ABBA blocks per input. The repeat and telemetry
runs pin CPU 0. Every complete output hash agrees; archived source/build/harness,
binary and input identities were verified against the final implementation.

The table fix saves exactly 2 MiB of retained identifier storage at the boundary
and about 4 MiB peak RSS in both A/B campaigns. Timing spread and small positive
costs are disclosed; no latency speedup or generated-code improvement is claimed.
The memory benefit, one avoided rehash, passing whole-pipeline growth bounds and
488-byte (0.85%) increase in host-tool text justify the change. No additional
optimization work is justified by this evidence. See the report for the corrected
dense-name memory budget and explicit comparison/telemetry regression disclosures.

| Final check on the audited implementation | Result |
| --- | --- |
| `perl scripts/cppgm_file_audit.pl --stage pa1 --paths dev/src` | PASS, 24 files, no warnings |
| Additional file audit of `dev/pptoken.cpp` | PASS |
| `make test-pa1` | PASS, 54/54 |
| `make test-report-through-pa1` | PASS, 54/54, 1/1 stages; no earlier PAs |
| Standalone ASan/UBSan binary, all course fixtures with `CPPGM_BATCH_TESTS=0` | PASS, 54/54 |
| `python3 student.tests/pa1/check.py` and `--sanitize` | PASS, 64 cases each, plus cursor ownership/identity/location/streaming/hit-allocation checks |
| All benchmark output, work, memory, fourfold scaling and 60-second timeout gates | PASS; both A and B independently recomputed for all three campaigns |
| `git diff --check` | PASS |

The compact [plan](plan.md) names the last reviewed implementation commit. The
implementation and this audit consolidation are the intended commits; generated
objects, logs and `.my*` outputs remain untracked/ignored. No PA1 item is deferred,
and this audit does not advance to PA2.
