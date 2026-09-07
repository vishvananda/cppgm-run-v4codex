# PA8 implementation audit

Scope: the PA8 handout and its unchanged 106 specification plus three behavioral
fixtures, under the current-stage requirements of the root spec. Stage entry
was 0/109, independently rerun despite the incoming stale 0/127 summary.
PA1–PA7 remain in the cumulative compiler and have no implementation changes.

## Ownership and data flow

- `dev/lowir.cpp` owns one `Program` per invocation. Each immutable file buffer
  feeds `Reader`'s single non-owning lexical view and is released before the
  next file. Only names, debug paths and object spellings enter the existing
  flat interner. Transient numeric and keyword strings belong to this explicit
  text adapter; no token vector or retained textual instruction body exists.
- `lowir/model.h` stores an eight-byte canonical type identity, distinct typed
  IDs for functions/symbols/values/slots/blocks/signatures, and unit-owned flat
  pools. Instructions own no vectors or pointers. Slices preserve parameter,
  instruction, phi operand, switch arm, structured data and source order.
  Each local name index belongs to one function builder and dies with it;
  global symbols share one unit index across input files. Pool IDs survive
  allocation growth and no process-global mutable cache exists.
- A forward direct call interns one `SymbolId`. A later file binds that same
  record to its function and `SignatureId`. Validation follows those IDs in
  constant time; the writer emits that definition once. No declaration is
  synthesized at the earlier file boundary. Unused real declarations survive.
- A loop phi interns forward `BlockId`/`ValueId` references. Constructors enforce
  local arity/result/terminator shape. At the external-input boundary the
  validator resolves types, ownership and uses, gathers ordinary CFG edges,
  sorts/deduplicates them once, then checks each phi against exactly its block's
  predecessor slice. Handler-registration edges do not become phi predecessors.
  Predecessor stamps avoid per-block clearing of whole-program arrays.
- Direct calls consume signatures by symbol identity; indirect calls carry a
  `SignatureId`. Object values retain complete byte/alignment boundaries;
  ordinary scalar slots retain their scalar value type and do not become
  pointer arguments implicitly. Passing, alias, extent, effects, unwind, query,
  linkage, storage, TLS, debug, atomic and exception records remain typed facts.
  Unknown effects stay conservative; metadata does not trigger optimization.
- The writer traverses these facts, not original source text. The exercises use
  `FunctionBuilder` directly. Sum's specified domain proves the chosen
  `n*(n+1)/2` algorithm fits i64; swap reads both values before either write;
  call-twice shares an explicit indirect signature and preserves both calls.
  The same writer serializes all three. Only the external test/measurement
  harness invokes `lowir2native-ref`, as expressly required by the assignment.

## Complexity, lifetimes and later design

Construction and writing are linear in input/output plus expected constant-time
interning. Type identity is inline, never a rendered spelling key. Full external
validation is O(IR + E log E), with O(IR + E) scratch space; only CFG edges are
sorted. There is no repeated fixed point, optimizer, semantic reconstruction,
host compiler delegation, ordered hot map or per-instruction child allocation.

The explicit roundtrip tool necessarily retains the requested unit model until
writing. Source buffers and function name indexes expire earlier, CFG scratch
expires after validation, and all remaining pools expire at invocation end.
Later source lowering can construct this same model directly, with text parsing
and writing as optional views. PA8 does not add a production serialize/reparse
path or make native optimization, ABI encoding or debug encoding prerequisites.

`--stats` reports read/build, full validation and writing times, peak RSS,
source/tokens, symbols/values, instruction/operand counts, validated work,
ordinary CFG edges and name storage. Pool allocation counters count growths of
14 IR pools, not every host-library allocation; capacities are reported
separately. Counters observe existing work and telemetry preserves output.

## Evidence and limits

All original course failures are eliminated. The 76 explicit personal semantic
cases, typed API checks and final ASan/UBSan run pass. The first working binary
fails the added signalling-NaN fidelity assertion; the final writer preserves
that distinction and the sign of wide integer literals. Native probes cover the
entire permitted sum domain and runtime work in all four benchmark families.
The cumulative gate is 793/793 (684 earlier tests + 109 PA8 tests); file audit
checks 97 implementation files without warnings. No course test, reference or
coverage selection changed.

[Performance evidence](performance.md) retains the cost of the additional
validation and literal fidelity, including noise and regressions. There is no
optimization benefit claimed. The native backend is still the supplied PA8
harness component; our own native implementation remains PA24 work. External
Ralph review markers in the plan are preserved at stage entry for its full audit.
