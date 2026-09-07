# PA6 completion plan and ledger

Target: **PA6 full-stage**. Phase: **complete**.
Entry: **0/105**. Final: **105/105**; PA1–6 through: **498/498**.
Stage base commit: `9249196518f45492822fb2e3da4eb5d82af0ed13`.
Last reviewed commit: `9249196518f45492822fb2e3da4eb5d82af0ed13`.
Both review markers are preserved for the external review process.

## Design / spec alignment

| Owner | Data flow and invariant | Complexity / evidence |
| --- | --- | --- |
| Graph / driver | Streaming PA1–5 cursor -> one source-faithful graph; semantic facts attach to NodeId; independent TU ownership; output is a view | Each declaration parsed/analyzed once; all course suites and lifetime APIs |
| Types / declarations | Structural canonical IDs, source parameters separate from adjusted signatures; array completion preserves aliases; definitions retain source/scope identity | Flat interning, completed signature caches; composed/array/reference/function contracts and direct API |
| Scopes / lookup | Interned-name indexes, explicit using edges, namespace reopening, qualified and anchored unqualified lookup; no snapshots/global invalidation | Relevant lexical scopes/edges and geometric ancestor links; namespace/shadowing/using contracts plus cycle/point probes |
| Classes / constants | Only complete-class bodies defer, with owned queue intervals; sparse constant facts; class-only layout/constructor arena | Demand only required facts; constant and signature work bounds; enum/template/class/rejection contracts and sanitizers |

The [architecture audit](audit.md) traces identity, declaration points, source
views, caches, template parameter environments and release boundaries. PA7+
expression resolution, instantiation, LowIR/MIR/ELF and executable optimization
remain at their owning stages. No current-stage behavior group remains.

## Performance evidence

[Full report](performance.md) and raw frozen datasets retain every observation.
Final binary: `1edcbe5db`. AST A: stage base; semantic A: `5749f43b4`.
Budgets: wall <=10% + A/A noise, RSS <=15% +1 MiB, compiler text <=35% above
stage base / <=5% above first PA6; fourfold input <6x wall / <5x RSS +1 MiB.
The initial 25% feature-text forecast was revised before timing; no failed
memory budget was relaxed. Generated executable runtime/text and self-hosting
are N/A at PA6.

Final campaign: 280 ordinary +16 startup +40 phase/work +28 telemetry runs;
all hashes, output-equivalence, startup-separation and budget checks pass.
Repeated-signature compilation is about 22% faster; other semantic workloads
cost about 1–4% more than first PA6. Largest template RSS grows about 11%.
Host text is 275,974 bytes (+32.35% over PA5, +2.40% over first PA6).
No broad frontend or generated-program speedup is claimed.

## Handoff ledger

| Increment / check | Result |
| --- | --- |
| `f6c4ddf33` | Recorded clean stage base/review markers; all 105 initial failures were dispatcher-stub failures |
| `5749f43b4` | First complete semantic implementation; initial run 101/105, four owner fixes reach 105/105 |
| `78df00b67` | Anchored lookup, enum transitions, scoped conversions, canonical fact reuse and class layout |
| `3f6de92f5` | Free/local-class declaration points, qualifier eligibility and explicit definition/body identities; first timing campaign stopped on the correctness finding |
| `1edcbe5db` | Fix measured template RSS failure by packing common records and separating class demand state; Entity 96 ->56 bytes, Constant 24 ->16 |
| Required checks | PA6 105/105; prior PA1–5 393/393; root through PA6 498/498; file audit 71 files; whitespace check passes |
| Personal / sanitizers | 33 PA6 cases, semantic identity/lifetime API, inherited PA5 API; ASan/UBSan/leak checks pass both APIs and all 105 unchanged PA6 fixtures |
| Inherited personal | PA5 core 10, extended 15 and audit 19 pass |
| Performance | Exact final frozen binary passes full verifier; interrupted and failed earlier observations retained |

Handoff reason: **full-stage completion**, with zero existing failures and
unchanged course coverage. No fixture/reference/harness/discovery/timeout
changes. All implementation sources are registered. Intended code, tests,
audit and measurement evidence are committed; generated objects, execution
logs and `.my*` outputs remain uncommitted. The final evidence commit is
followed by a clean-status check. Remaining work: **none**.
