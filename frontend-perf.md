# Frontend performance plan

Bring the host-built compiler's frozen-workload frontend ratio to **equal or
better than the reference**. The acceptance target is
`user_time(v4opt) / user_time(reference) <= 1.00`, confirmed by repeated paired
measurements. Track peak RSS and emitted output size alongside time: both can
explain excess work and guide improvements. A smaller intermediate improvement
is a checkpoint, not completion.

The implementation and validation record below accompanies the original plan.
The baseline and experiment procedure remain available for reproducing the work.

## Implementation record

The retained changes reduce repeated work without changing the frozen TU's
emitted object:

- AST readers can request an individual scalar or edge. Reading a node kind,
  spelling, location or literal no longer projects four unrelated edges. The
  semantic and lowering call sites use these accessors; 137 changed source files
  contain only these mechanical substitutions. Ambiguous declaration roles and
  pack expansion overrides remain part of the relevant accessors.
- Template occurrence indexes are local to each substitution context and store
  four-byte occurrence IDs. Their keys already exist in the occurrence records.
  A bounded, positive cache holds one recent occurrence per parsed source node;
  misses are never cached because later demand can publish more of a region.
  Lookup and insertion share a probe, and known region sizes reserve capacity.
- Sparse `IdIndex` entries occupy 12 bytes instead of 16. Updates avoid growth,
  rehashing inserts directly, empty queries return immediately, and the table
  retains its mask. Removal still closes the affected probe cluster.
- Character traversal consumes ordinary ASCII identifier, whitespace and comment
  spans directly. Special characters, pending lookahead, translation phases and
  non-ASCII input retain their existing paths. Small character/token lookups and
  spelling comparisons are inline, and the token ring uses its power-of-two size.
- Canonical types cache fundamental IDs, skip redundant qualification and pack
  disjoint fields before hashing. Full type comparisons still resolve collisions;
  the canonical type count and identities remain stable.

No allocator or release build flags changed. Inlining the AST edge projection
path did not improve its timing screen and was reverted. A separate jemalloc
control increased user time despite a small RSS reduction and was not adopted.
The output-size investigation found substantially more emitted weak/inline
functions and unwind data than the reference. Pruning them is a separate semantic
demand problem: unused inline definitions must still receive required validation.
This change preserves their output and required diagnostics.

### Final frontend measurements

The final seed is the canonical GCC 15.2.0 release build, SHA-256
`049c8cabde7e1ad37cc21fa841cbaa26971e03b336c90296ed36074a3ec9a96b`.
The pinned reference executable is
`61b81983a980887b8f3685f6c6fb04b057f55adb92c824fb0dc1fd25a66586aa`.
Final runs used CPU 4 (SMT sibling 48), serially after builds and tests. Every
comparison used the same frozen source/include closure and `-O0 -std=gnu++11 -c`.

| Paired comparison | ABBA blocks | Median user-time ratio | Median wall-time ratio |
| --- | ---: | ---: | ---: |
| Initial v4opt / reference | 3 | 1.860 | — |
| Final v4opt / reference, batch 1 | 3 | **0.978** | 0.993 |
| Final v4opt / reference, batch 2 | 3 | **0.991** | 1.002 |
| Final v4opt / reference, extended confirmation | 7 | **0.958** | 0.976 |
| Final v4opt / original v4opt | 3 | **0.495** | 0.514 |
| Final seed / itself, calibration | 3 | 1.000 | 1.004 |

Ratios are medians of per-block paired averages, not ratios of separately
reported medians. The final reference comparisons contain 26 observations per
compiler. The seven-block confirmation had user ratios
`0.933, 0.958, 0.992, 0.954, 0.941, 1.040, 0.975`. Calibration had one 6% outlier
block, so the evidence supports reaching parity with a small advantage on this
workload, rather than a precise universal percentage lead. The original-to-final
paired comparison shows **50.5% less user time**. All runs succeeded and emitted
deterministic output; comparisons with the original seed required exact bytes.

Fresh three-block GCC pairings give frontend/GCC user-time ratios of **0.438**
for v4opt and **0.465** for the reference using `ralph:bench.md`'s ratio of
medians. The corresponding median paired-block ratios are 0.444 and 0.462.
The GCC pairings were separate batches, so the direct reference comparisons
above remain the acceptance evidence. Their logs are `final-gcc-candidate.*`
and `final-gcc-reference.*`.

The region-reservation screen initially appeared slower amid host variability.
A subsequent three-block before/after comparison gave a 0.931 user-time ratio
with unchanged RSS/output, supporting its retention alongside the separately
observed reductions in instructions and branch misses. Small AST-edge inlining
and allocator experiments remained rejected. Raw screen and confirmation runs
are retained, including unsuccessful measurements.

| Space measurement | Original v4opt | Final v4opt | Reference |
| --- | ---: | ---: | ---: |
| Peak RSS, MiB | 636.8 | 515.2 | 364.4 |
| Frozen object bytes | 7,552,664 | 7,552,664 | 2,944,688 |
| Frozen executable `.text*` bytes | 1,227,419 | 1,227,419 | 687,749 |
| Frozen `.eh_frame*` bytes | 403,104 | 403,104 | 119,816 |
| Frozen `.gcc_except_table*` bytes | 41,521 | 41,521 | 24,316 |
| Host-built compiler file bytes | 5,159,256 | 5,167,408 | 8,379,360 |
| Host-built compiler GNU `size` text | 4,765,023 | 4,770,890 | 7,440,392 |

RSS falls about **19%** against the original, but remains about **41% above the
reference**. Host-built compiler text grows only 0.12%. The occurrence index,
including its context descriptors and recent cache, occupies 37,310,976 capacity
bytes; the old global occurrence hash table alone needed 128 MiB. Parsed nodes
(808,942), template occurrences (2,723,959), canonical types (205,813) and IR pool
growths (226) are unchanged. Type-intern probes fall from 1,353,575 to 1,131,608.
These counts help distinguish avoiding redundant lookup work from omitting
semantic analysis or output.

The unchanged object contains 9,608 weak function definitions versus 4,091 in
the reference, including standard-library regex helpers reached through inline
definitions. Its relocation sections occupy 1,045,728 versus 594,000 bytes;
symbol and string tables also contribute to the file-size gap. Executable code
and unwind data are reported separately above rather than treating GNU `size`'s
entire text column as machine instructions. `final-sizes.json` and
`final-symbols.json` retain the full breakdown.

### Counter confirmation

These final diagnostics used CPU 4 after timing. Reference and candidate ran
in alternating ABBA order, with work and cache events in separate sets; all
events reported 100% running time. The table uses the two-run medians for final
v4opt/reference and one fresh original run. Counters explain the result rather
than replacing the paired timing acceptance checks.

| User-mode event | Original v4opt | Final v4opt | Reference |
| --- | ---: | ---: | ---: |
| Instructions, billions | 24.916 | 12.123 | 11.358 |
| Cycles, billions | 17.222 | 8.908 | 9.008 |
| Branches, billions | 4.199 | 2.172 | 2.143 |
| Branch misses, millions | 96.81 | 50.34 | 42.76 |
| L1 data-load misses, millions | 189.75 | 130.39 | 146.50 |
| LLC load misses, millions | 14.18 | 7.48 | 7.02 |
| dTLB load misses, millions | 14.92 | 6.64 | 6.12 |

Retired instructions fall **51.3%**, with cycles down 48.3%. IPC changes from
1.45 to 1.36; the gain comes from doing less work. L1 misses per thousand
instructions rise from 7.62 to 10.76 as the instruction denominator halves,
while absolute L1 misses fall 31%. The final branch-miss rate is 2.32%, similar
to the original 2.31%. The cache and TLB results support keeping context-local
occurrence indexes rather than adding a large global lookup cache.

The final short cycle profile has 584 samples and no lost samples. Remaining
exclusive hotspots include `IdIndex::find` (10.4%), `IdIndex::put` (6.0%),
`Ast::edge` (2.7%) and `CharacterCursor::fill` (2.6%). Future work can investigate
semantic lookup/binding indexes and definition demand from this new baseline.
Raw counts and profiles are in `final-*-counters-*`, `final-counters.json` and
`final.profile.*`.

### Self-built compiler tradeoff

Three-block comparisons with exact object checks give a final self/host
user-time ratio of **3.042** (wall 2.815), versus the earlier 2.333 (wall 2.224).
A fresh reference self/host comparison gives 1.703 (wall 1.624). The relative
backend gap therefore widens: the GCC-built compiler benefits more from this
implementation than the compiler built by our own backend.

The self-built compiler nevertheless improves in absolute performance. A direct
three-block old-self/new-self comparison gives a user-time ratio of **0.706**
and wall ratio of 0.710, about **29% faster**, with identical objects. Its RSS
ratio is 0.820. This separates the changing denominator in the backend ratio
from a slowdown of the self-built executable. Backend code-quality parity is
still future work; it was not the acceptance target for this frontend change.

The final seed/self GNU `size` text columns are 4,770,890 / 9,595,915 bytes
(2.011 ratio). Their executable `.text*` sections are 4,248,326 / 8,534,693 bytes
(2.009 ratio). Self and inception executable SHA-256 is
`d25168c70621201c2e2147985455526355bd9be8c08b904c571845d11ff25d22`.
See `final-backend.*`, `final-backend-reference.*`, `final-self-original.*`
and `final-metadata.json` for runs, sizes, hashes and build/host details.

### Normal fixtures

Two new fixtures use the existing course harnesses and reference-generated
sidecars; neither requires a custom test runner:

```sh
make -C pa1 check TEST=../student.tests/pa1/200-ascii-span-boundaries.t
make -C pa19 check TEST=../student.tests/pa19/400-interleaved-template-regions.t
```

The PA1 fixture checks transitions between ASCII runs and UCNs, UTF-8, line
splices, trigraphs, comments, raw-string delimiters and literal suffixes. The
PA19 fixture interleaves class specializations, nested-class demand, defaults
and empty/nonempty member-template packs, while retaining an unused dependent
body that must not be instantiated. Both are suitable for inclusion in their
assignment's normal fixture collection.
Both also pass with the pre-optimization tools. They cover the changed paths;
neither was added for a correctness bug discovered during optimization.

### Correctness evidence

- `make test-report CXX=g++ CPPGM_HOST_CXX=g++`: **5,454/5,454 pass**.
- `make inception CXX=g++ CPPGM_HOST_CXX=g++`: **420 matching objects**, followed
  by **`MATCH cppgm++`** for the self and inception executables.
- Frozen TU seed/self objects match byte for byte at **O0/O1/O2/O3**. O0 also
  matches the pre-optimization seed's output exactly.
- The overflow regression's LowIR and objects match at all four levels. Its
  runtime fixture, the conversion-member-demand fixture and the new PA19 fixture
  pass with both seed and self compilers; the new PA1 fixture passes normally.
- The additional root `make test-debuginfo` run passes PA32/PA33 and the PA8
  object/DWARF/debugger checks. It still reports five PA8 textual differences
  from the reference: one LowIR whitespace case, three source LowIR cases, and
  one machine-IR register-choice case. All five outputs are byte-identical to
  the pre-optimization implementation, verified with its saved seed and tools
  rebuilt from `291e8e9e`. No fixture or reference was changed to suppress them.

Raw correctness logs, output hashes, the mechanical access-substitution audit
and baseline debug comparisons are in `obj/frontend-perf/`, under the
`final-*`, `mechanical-audit.diff` and `debug-baseline-*` names. These generated
artifacts are intentionally excluded from the commit.

## Baseline and measurement contract

Use the frontend benchmark from `ralph:bench.md`: a compiler built by host
`g++ -O3` compiling the frozen TU with `-O0 -c`. This includes preprocessing,
parsing, semantics, lowering, native code generation, and object writing.
An improvement to an isolated frontend phase must improve this complete command.

The fixed baseline is v4opt commit `291e8e9e6dbb29d1907f1ccec563a85edd228a82`;
the reference is `~/cppgm-extended` at
`3299e7ff314b072425c1466391f51fd278d45eb1`. Existing three-block ABBA results
are in `obj/v4opt-bench-20261002-fixed/`.

| Measurement | v4opt | Reference |
| --- | ---: | ---: |
| Median frontend user time | 5.090 s | 2.675 s |
| Frontend ratio to the paired GCC baseline | 0.837 | 0.453 |
| Median peak RSS | 636.74 MiB | 364.05 MiB |
| Frozen TU object file bytes | 7,552,664 | 2,944,688 |
| Frozen TU GNU `size` text column | 1,672,044 | 831,881 |

The time/RSS rows come from the completed benchmark; object sizes were checked
again while preparing this plan. The reference and v4opt had separate GCC
pairings, so their raw time ratio of approximately **1.90** is an estimate of
the direct gap. Establish a direct reference/v4opt ABBA baseline before editing.
At the recorded reference time, parity requires approximately a **47% reduction**
in v4opt user time. Re-measure the reference; do not make 2.675 seconds a fixed
threshold independent of host conditions.

Preserve the original frontend/GCC ratio in reports, using fresh GCC pairings.
Use the direct v4opt/reference user-time ratio for optimization decisions. Keep
the self/host backend ratio as a secondary final check.

Freeze the source and all 51 headers using
`~/cppgm-extended/benchmarks/self_compile/stable/PERF_EPOCH.json`:

- Source: `semantic_overload.cpp` in that directory.
- Include root: that directory's `include/`.
- Epoch: `9764b3835e3c6996b6b80803054f80e1cf50f98e`.
- Source SHA-256: `ab00b2e1c3c7463baf9d8e1e7fc754b9cde2c18749568616062011f31e7daba2`.
- Header closure SHA-256: `7c8a5445f33f04b314de98e6a099de4d75124b4bb032fc97ee5055e56d4827c8`.

Retain the reference executable snapshot, compiler hashes, build flags, host
configuration, library versions, allocator, affinity, and per-run measurements.
Store generated artifacts under `obj/`. Keep the frozen workload unchanged.
Use the same hosted includes and macros throughout.

The reference seed links jemalloc and has `-fno-strict-aliasing`; our default
seed uses glibc allocation. Keep these known configurations for the principal
comparison. Run matched allocator controls separately when investigating memory.
The previous jemalloc control did not close the gap; changing the allocator is
not the leading hypothesis. Compare implementation changes with identical build
flags on both sides of each before/after experiment.

## Initial counter and profile evidence

This host is a dual-socket Xeon E5-2696 v4 with 44 physical cores and 88 hardware
threads. `perf` 7.0.14 successfully records user-mode hardware events with the
current permissions. A diagnostic run pinned to CPU 2 collected:

| User-mode event | v4opt | Reference | v4opt/reference |
| --- | ---: | ---: | ---: |
| Instructions | 24.916 billion | 11.359 billion | 2.19 |
| Cycles | 17.366 billion | 9.027 billion | 1.92 |
| Branches | 4.199 billion | 2.144 billion | 1.96 |
| Branch misses | 96.71 million | 42.41 million | 2.28 |

These are single diagnostic observations, not acceptance measurements. All four
events reported 100% running time. The instruction excess makes reducing work
a stronger initial hypothesis than improving IPC alone.

One separate `--stats` run reported 5,194 ms in `frontend_ms`, 832 ms in
`lowering_ms`, and 6,323 ms in `driver_ms`. `frontend_ms` includes preprocessing,
parsing, and semantic analysis; nested `semantic_ms` is not additive. Statistics
collection adds overhead, so use these numbers to locate work, and time normal
commands without `--stats` to judge speed.

A short cycle profile had approximately 1,000 samples and no lost samples:

| Function, exclusive samples | Share |
| --- | ---: |
| `IdIndex::get` | 27.2% |
| `CharacterCursor::peek` | 9.9% |
| `Ast::project_view` | 6.4% |
| `IdIndex::put` | 3.8% |
| `CharacterCursor::take` | 3.2% |

Many `IdIndex::get` samples came through AST projection and instantiation.
The stats run recorded 3.53 million AST IDs, including 2.72 million template
occurrences; IDs do not represent that many full copied syntax nodes. It also
reported 30.8 MB of semantic fact storage and 55.1 MB of IR pool capacity.
Measure the remaining capacities, especially occurrence indexes, before choosing
a memory representation. These observations live in `obj/frontend-perf-plan/`.

## 1. Establish a repeatable, short experiment loop

1. Copy the current seed into an immutable baseline path under `obj/`. Retain
   the existing reference snapshot. Record hashes and verify the workload manifest.
2. Select an idle physical core and keep its SMT sibling idle during timing.
   CPU 2 was used for the diagnostic run; its sibling is CPU 46. Recheck host
   load before choosing it again. Pin the runner so its compiler children inherit
   affinity and stay on one NUMA node. Run comparisons serially, after builds
   and test jobs finish. Warm each binary once; do not drop system caches.
3. Run an A/A calibration with the same executable in both slots. Earlier
   calibration sometimes showed about 7% noise, so small wins need confirmation.
   Record individual samples and block ratios, not just the fastest run.
4. Use one ABBA block for screening a change, then three blocks for a promising
   candidate. Recheck both the original baseline and the reference periodically
   so cumulative progress remains visible. Around parity, confirm in a second
   independent batch; extend sampling if the observed spread leaves the outcome
   ambiguous.
5. Collect user/wall/system time, peak RSS, object bytes and output hashes on
   every run. For retained candidates also collect instructions, cycles, branch
   misses, section sizes, symbol counts, and relevant phase/work counters.
   Record build/test time separately from compiler benchmark time.

Example direct reference comparison, run from the repository root in Bash:

```sh
PERF_REF=/home/vishvananda/cppgm-extended
PERF_FROZEN="$PERF_REF/benchmarks/self_compile/stable"
PERF_OUT="$PWD/obj/frontend-perf"
PERF_CPU=2  # Recheck availability and its SMT sibling first.
mkdir -p "$PERF_OUT"
make -C dev cppgm++ CXX=g++

taskset -c "$PERF_CPU" python3 "$PERF_REF/scripts/run_ab_compile_benchmark.py" \
  --repo-root "$PERF_REF" \
  --compiler-a "$PWD/obj/v4opt-bench-20261002-fixed/bin/reference-gcc" \
  --compiler-b "$PWD/dev/cppgm++" \
  --source "$PERF_FROZEN/semantic_overload.cpp" \
  --include "$PERF_FROZEN/include" \
  --compiler-arg=-O0 --compiler-arg=-std=gnu++11 \
  --abba-blocks 3 --output-mode deterministic --timeout-sec 1800 \
  --output-prefix "$PERF_OUT/reference-vs-candidate"
```

The runner reports median per-block candidate/baseline ratios as well as
per-compiler medians. Read the **user_seconds** ratio for the primary metric.
Give each experiment a new output prefix. Reference and v4opt may emit different
valid objects, so this comparison checks determinism within each compiler.
For seed/self comparisons use `--output-mode exact`. For a before/after change
that should preserve emission, require exact equality too.

Build only `dev/cppgm++` during small experiments. Keep edits localized to avoid
unnecessary shared-header rebuilds. Run a reduced fixture and the affected
contract checks before investing in a longer benchmark. Rebuild self-hosted
generations at meaningful checkpoints and for final validation; a seed change
invalidates their old objects.

## 2. Use counters to select and verify each change

Use normal release binaries for timing. Profile both implementations separately
to identify excess work in v4opt. Start with the following commands, using the
variables above and a unique output prefix for each run:

```sh
taskset -c "$PERF_CPU" perf stat -x, \
  -e cycles:u,instructions:u,branches:u,branch-misses:u \
  -o "$PERF_OUT/candidate.counters.csv" -- \
  dev/cppgm++ -O0 -std=gnu++11 -c "$PERF_FROZEN/semantic_overload.cpp" \
  -I "$PERF_FROZEN/include" -o "$PERF_OUT/candidate.o"

taskset -c "$PERF_CPU" perf record -F 199 -e cycles:u \
  --call-graph dwarf,8192 -o "$PERF_OUT/candidate.perf.data" -- \
  dev/cppgm++ -O0 -std=gnu++11 -c "$PERF_FROZEN/semantic_overload.cpp" \
  -I "$PERF_FROZEN/include" -o "$PERF_OUT/profile.o"
perf report --stdio -i "$PERF_OUT/candidate.perf.data"

stat -c '%s %n' "$PERF_OUT/candidate.o"
size -A "$PERF_OUT/candidate.o"
readelf -SW "$PERF_OUT/candidate.o"
```

Repeat counter measurements in alternating order for baseline/candidate and
reference/candidate. Measure L1/LLC and dTLB load misses in separate small event
sets supported by `perf list`. Check event running percentages and split sets
when multiplexing would obscure the result. Correlate misses per thousand
instructions, IPC, and branch miss rate with absolute counts and RSS.

Use caller stacks and `perf annotate` on the dominant paths. The existing
release symbols suffice for function attribution. If source-line attribution
requires an `-O3 -g` build, use a separate checkout/object root and retain the
canonical build for timing and final checks. Keep profiler and `--stats` runs
separate from official timing. Add diagnostic-only counters only where existing
telemetry cannot answer a specific question.

## 3. Prioritized implementation experiments

Work on one hypothesis at a time, re-profile after each accepted improvement,
and adjust this order when evidence changes. Preserve the C++11 and assignment
contracts, and read the owning handout before editing its implementation.

### A. Reduce AST projection and index work

Start with `dev/src/syntax/occurrence.cpp`, `dev/src/syntax/ast.h`, and
`dev/src/support/id_index.{h,cpp}`. Together the sampled index/projection
functions account for roughly 37% of cycles, making this the first experiment.

- Count lookup calls, hit/miss probes, rehashes, live entries, and allocated
  bytes per index. Distinguish the large `(context, source)` occurrence index
  from sparse metadata indexed by a single node ID.
- Avoid projecting all four node edges when a hot caller needs only one edge
  or the node kind. Avoid repeatedly projecting the same view within a traversal.
- Evaluate compact per-region/context mappings for immutable source topology,
  and direct or chunked ID tables for sufficiently dense metadata. Compare their
  memory costs with the current open-addressed indexes before adopting them.
- Investigate redundant probes and growth on updates in `IdIndex::put`, plus
  measured hash/probe overhead. Do not replace every index indiscriminately.
- Preserve zero/absent-edge behavior, deferred region demand, pack expansion
  overrides, collision-safe removal, and source/context identity. A projected
  edge can change when another region becomes demanded: any cache needs a valid
  lifetime or invalidation rule. Preserve deterministic traversal/emission order.

Expected evidence: fewer lookups/probes and retired instructions, lower or stable
RSS, unchanged required behavior and reproducible objects. Exercise template
instantiation, nested classes, packs, defaults, and ambiguity resolution using
the affected PA5 and PA14–PA19 fixtures, plus later features using this machinery.

### B. Reduce character and token traversal overhead

Inspect `dev/src/preprocess/source.cpp`, `token_cursor.cpp`, and the syntax
cursor. `peek`/`take` alone account for about 13% of sampled cycles.

- Measure repeated lookahead and consumption, ring-buffer indexing, and calls
  crossing translation units. Try a cheaper common zero-lookahead path and
  cursor advancement before a broad tokenizer redesign.
- Consider contiguous ASCII identifier/whitespace scanning only where it
  preserves translation phases and falls back correctly for special input.
- Retain trigraphs, line splicing, UCN/UTF-8 handling, raw strings, mode switches,
  locations, and EOF behavior. Validate PA1–PA5 contracts and hosted-header cases.

Expected evidence: fewer instructions per input byte/token, byte-identical
preprocessor/token outputs where required, and an end-to-end compile-time win.

### C. Reduce retained memory and repeated semantic work

Use capacity and allocation measurements to rank occurrence metadata, fact
stores, entities/scopes, substitutions, and IR storage. The baseline RSS is
about 1.75 times the reference; avoid trading a small CPU win for an unbounded
cache or a full copied AST per instantiation.

- Reduce redundant materialization and temporary copying; use compact stable
  IDs and shared immutable records where the existing ownership model permits.
- Size pools from actual demand, avoid excessive capacity growth, and release
  phase-local data only after proving that later lowering no longer needs it.
  Profile allocation/destruction before changing allocators or introducing arenas.
- If semantic query work remains hot, examine canonical type/argument interning,
  substitution, lookup, overload candidates, and class/body completion. Existing
  caches already cover some paths; measure missed reuse rather than adding a
  second cache blindly.
- Preserve context-dependent lookup, specialization identity, completion states,
  access checks, and SFINAE failure semantics. Keep successful, failed, active,
  and not-yet-demanded states distinct.

Expected evidence: smaller live/capacity counts and RSS, fewer repeated semantic
operations and cache/TLB misses, and stable or better compile time. Keep normal
fixtures for any correctness boundary uncovered by these changes.

### D. Explain and reduce the emitted-output gap

The object is 2.56 times the reference size, while the GNU `size` text column is
about 2.01 times as large. Break this down before attributing all of it to code:
separate executable `.text*`, read-only data, unwind/exception data, relocations,
symbol/string tables, and other sections. GNU `size`'s default text total includes
more than executable instructions.

Compare defined functions and their sizes, template bodies demanded, native
functions, IR instructions/operands, and relocation counts. Investigate whether
unnecessary definition demand or duplicate lowering explains both larger output
and extra frontend work. Preserve required externally visible definitions,
address-taken functions, virtual tables, RTTI, initialization, and host ABI data.
Any legitimate emission change needs runtime/link/debug validation and exact
seed/self agreement, although its bytes may differ from the previous revision.

Track compiler executable size separately, especially after changing inlining
or data structures. Output-size improvement is valuable when it removes real
work; retain full required output in the benchmark. Native instruction-selection
work should follow evidence that it materially affects this frontend metric.

## 4. Keep/revert decisions and progress records

For each experiment record its hypothesis, patch/commit identity, build flags,
affected fixtures, before/after user time and ratio, peak RSS, output bytes,
instructions/cycles, and the work counter expected to change. Keep the raw runs.

Retain a performance change when its mechanism is supported by counters, the
paired time gain exceeds measured noise, and correctness checks pass. Investigate
RSS or output growth explicitly; avoid accumulating unexplained regressions.
Revert unsuccessful experiments before trying the next hypothesis. Re-profile
the cumulative candidate so improvements target the remaining bottleneck.

Use intermediate ratios such as 1.50 and 1.25 to assess progress, with **1.00 or
better** as the completion target. The initial hotspots cannot realistically
yield the entire required reduction from a single local edit; plan for multiple
measured improvements. If progress stalls, revise the hypotheses using new
profiles and account for the remaining phase time rather than relaxing the target.

## 5. Correctness and final acceptance gates

For each behavioral issue discovered, add a minimal normal fixture under
`student.tests/paN/` using the owning assignment's harness and sidecars. Run
personal fixtures explicitly; root test targets do not discover them. Use the
existing benchmark runner for measurements and normal fixtures for language
regressions. Read [Testing and references](TESTING_AND_REFERENCES.md) and the
owning handouts; keep all required behavior, debug, and inspection checks.

Before declaring the optimization complete, validate the final source revision:

1. Run all added fixtures and affected assignment suites, including required
   through reports. Re-run the conversion-member-demand and overflow-emission
   regressions already added on this branch.
2. With the canonical release configuration, run these root targets successfully:

   ```sh
   make test-report CXX=g++ CPPGM_HOST_CXX=g++
   make inception CXX=g++ CPPGM_HOST_CXX=g++
   ```

   The starting revision passed 5,454/5,454 report tests and printed
   `MATCH cppgm++`. Require fresh success on the final revision. Respect PA34's
   normal generation tracking and resource limits; do not combine objects
   produced by different seed revisions.
3. Compare the frozen workload's objects from the final seed and newly rebuilt
   self compiler at **O0, O1, O2, and O3**. Also perform the normal runtime and
   LowIR/object byte comparisons in
   [the overflow regression](student.tests/pa29/overflow-emission-order.md).
   Inception checks the repository's own compiler sources; it does not replace
   these workload-specific checks.
4. After validation jobs finish and the host settles, rerun direct
   reference/candidate ABBA comparisons and the original GCC-normalized frontend
   benchmark. Confirm a user-time ratio **<= 1.00** against the frozen reference
   in two independent batches of at least three ABBA blocks, with individual
   samples and calibration sufficient to resolve parity. If noise straddles the
   threshold, collect more evidence before claiming success.
5. Report final RSS and emitted file/section sizes versus both the original v4opt
   and reference, and explain material tradeoffs. Repeat the self/host backend
   comparison with exact output checks to detect a self-built performance
   regression. Preserve logs and hashes under `obj/`; commit source, normal
   fixtures, and documentation rather than generated artifacts.

If a final check fails, reduce and fix it, then rerun the affected checks and the
final gates on the corrected revision. The work is complete only when frontend
parity or better and all required correctness/reproducibility checks hold together.
