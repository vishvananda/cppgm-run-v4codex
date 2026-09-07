# PA1 implementation plan

Stage base commit: 1b05951a54c7803f3ac1a1db87b7213ec2b426f1
Last reviewed commit: 1b05951a54c7803f3ac1a1db87b7213ec2b426f1

## Design/spec alignment

- `SourceBuffer` owns immutable bytes/file identity. `CharacterCursor` performs
  UTF-8, UCNs, trigraphs, splices and final newline handling with an 18-slot ring;
  literal scanning controls raw reversion and escaped-backslash recognition.
- `PPTokenCursor` owns one reusable transformed-spelling buffer and emits typed
  tokens with physical ranges/locations. Unchanged spellings borrow source bytes;
  comments do not allocate spellings. All cursor temporaries die with the cursor.
- `IdentifierTable` owns canonical name/suffix IDs and flat geometric byte/entry
  storage for one TU; IDs survive growth. No process-global mutable cache, token
  vectors or serialized interphase transport. The PA1 writer is an explicit view.
- Work is linear in source/token bytes with bounded punctuation/raw-delimiter
  lookahead and expected linear interning. Memory is source + unique names +
  longest transformed token. Later semantic/lowering/backend work remains later PA scope.

## Remaining groups and performance

No PA1 groups remain. All translations, maximal-munch tokens, literals/suffixes,
comments, Unicode boundaries, include context and required rejection cases pass.
[Evidence](../student.tests/pa1/performance.md): frozen flags/binary/inputs, complete
output equivalence, two A/A pairs and two ABBA blocks on five workloads; all 60
observations retained. At 8/32 MiB, median compiler latency was 1.503/5.999 s,
maximum RSS 12096/36668 KiB, and identifier storage stayed at 618 bytes. The 3.99x
latency ratio passes the <=6x budget; decoded work <=2*bytes+64 and peak RSS
<=4*bytes+32 MiB also pass. Timing noise is disclosed; no optimization speedup is
claimed. Generated executable runtime/text size: N/A, because PA1 emits tokens.

## Handoff ledger

- Entry (`fee23cc04` plan): clean baseline, 0/54 passing; previous failure evidence
  established missing implementation, with no process to await.
- Core (`a03163343`): all shared semantic groups implemented; 54/54 course pass.
- Boundaries (`78a0872d6`): fixed trailing-splice newline, BOM-only input and escaped
  UCN recognition; 59 personal cases plus identity/location/streaming checks pass,
  also under ASan/UBSan. Self-tokenization succeeds for all implementation sources.
- Exit: `make test-pa1` and `make test-report-through-pa1` pass 54/54; earlier PAs
  pass 0/0; `perl scripts/cppgm_file_audit.pl --stage pa1 --paths dev/src` passes
  24 files; whitespace audit passes. Course fixtures/coverage remain unchanged.
- Handoff reason: full PA1 completion, including related boundary groups and
  compiler evidence. No incomplete behavior group or deferred PA1 task remains.
