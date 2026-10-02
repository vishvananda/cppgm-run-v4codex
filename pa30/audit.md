# PA30 accumulated checkpoint audit202

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `378d1bd83df4dd3f7a9c1693c18afa7e7c7bd61f`.

The complete accumulated range is `4a081cb0..56ecc31c`, extended through
`378d1bd8` for the validated audit repairs. Both entry markers agreed with
history; the previous audit's code tip is the boundary, not the latest handoff.
All 14 intervening commits and their combined source changes were reviewed.
This completes checkpointAudit; PA30 remains incomplete at **151/153**, with
the same two required failures as entry. No stage advancement is authorized.
The previous goal turn was progress (committed implementation and verified
evidence); no inherited compiler/test process was live at audit entry.

## Complete range and dispositions

| Commit | Reviewed content and disposition |
|---|---|
| `d9a62584` | Prior audit records and performance198, preserving the code boundary. Historical audit198 remains available at this revision. |
| `a7a6e3a8` | Runtime first array extents separated from canonical element types; dependent protected access and current-instantiation friendship; inherited aliases; fixed vector declarations and representation conversions. Bound conversion gap repaired below. |
| `6b5645c0` | Complete-class lookahead preserves member-template function boundaries. Removes a nonexistent byte-extraction builtin; byte construction is still checked through a byte copy, without reducing course coverage. |
| `c8526635` | Volatile vector operands snapshot every lane before extraction or representation transfer. |
| `d6408429` | Static/omitted vector zero initialization emits typed aggregate byte extents. |
| `377d92a0` | Implementation199 handoff: 138/153, source/binary bindings, runtime/text and compile/scaling evidence. |
| `6dcff26a` | Aggregate child destruction demand and enclosing complete-class body scheduling. |
| `e171a838` | Concrete member-template identity distinguished from dependent provenance in integer-sequence queries. |
| `37b6729d` | Destructor access/demand belongs to selected aggregate children; scalar-new root destruction remains separate. |
| `b0790726` | Implementation200 handoff: 148/153, demand/access controls and performance evidence. |
| `acffe94c` | Automatic object use/capture cannot cross an ordinary function boundary. |
| `3d02099c` | One replacement-new reference status correction, with reduced sources, preprocessing and standard proof. Confirmed below. |
| `fc8ea8d6` | Typed noreturn attributes, default-lambda contexts and function-local non-void CFG checking. Constant conditional joins required the repair below. |
| `56ecc31c` | Implementation201 handoff: 151/153, full validation, controls and performance. Entry verifier passes 1,283 checks. |
| `378d1bd8` | Audit repair: contextual array-bound conversion and sparse constant-join reachability, with explicit controls and registered implementation source. This is the reviewed code tip. |

Historical bindings are checked at their own handoff/code revisions, including
tracked wrapper symlinks. They are not incorrectly compared with newer source.
[Bindings](../student.tests/pa30/evidence202/source-binding.json) enumerate the
entire range and bind 81 current sources, the dev tree and all frozen binaries.
The final records commit changes only Markdown and JSON evidence after this tip.

## Findings and fixes

1. **First new[] extents rejected valid class conversions.** The new query
   separated runtime extents correctly but copied the existing integral-only
   test. C++11 [expr.new]/6 permits a class with a unique non-explicit conversion
   to an integral or unscoped-enumeration type. `array_bound_conversion` now owns
   that rule for both ordinary allocation and type queries. Queries validate
   access/deletion without demanding the conversion body; evaluated uses record
   the selected conversion once. Lowering consumes its actual signedness.
   A constant class object is not mistaken for the integer returned by its
   conversion. [N3337 §5.3.4/6](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf)
   supplies the rule; compiler agreement is not the proof.
2. **The new fallthrough check rejected constant conditional loops.**
   `int f(){while(1 ? 2 : 0){}}` and nested arithmetic/conditional forms were
   accepted before implementation201, then rejected because a join slot hid the
   condition from its single-definition integer proof. The repaired owner tracks
   executable edges, integer temporaries and eligible whole scalar slots through
   a deduplicated instruction worklist. Facts move Pending → Constant → Varying;
   unresolved cycles are seeded conservatively once. Volatile, escaped, partial,
   noninteger and unknown storage cannot prove an edge dead. Noreturn normal
   exits and throwing/resume edges remain separate. The proof never rewrites IR.
   An initial branch-lowering approach changed a valid PA14 structural fixture;
   it was removed, and the complete earlier report was rerun successfully.
   [Intermediate report](../student.tests/pa30/evidence202/validation-intermediate.json)
   preserves that failed check; no reference/comparator was relaxed.

[Entry reducers](../student.tests/pa30/evidence202/reducers-before.json) retain
source text/hashes and the frozen entry outcomes. The **35 new control commands**
cover nested/templated constant joins, actual reachable fallthrough, slot mutation,
escape and volatility, conversion effects and temporary lifetimes, integral and
reference-returning bounds, inheritance, invalid/private/deleted/ambiguous/explicit
conversions, and a query whose dormant conversion body must remain undemanded.
No known defect in these repaired groups is deferred to a later audit.

## Architecture and interaction trace

[interactions.cpp](../student.tests/pa30/source202/interactions.cpp) combines the
three handoffs: concrete member alias generation and constructor packs, deferred
nested class bodies, protected dependent bases and friendship, selected union
subobject cleanup, converted array allocation, volatile vector extraction,
default/nested captures and a noreturn boundary. It executes successfully both
from a direct object and from serialized LowIR rebuilt by the compiler's native
backend. The serialized path is an explicit test adapter, not production transport.

| Spec surface | Ownership and reviewed evidence |
|---|---|
| Source/parser (§1) | Immutable TU buffers → streaming `PostTokenCursor`/ring cursor → interned identifiers and retained source nodes. Parser and Analyzer cooperate through `translation_unit(&sem)`. `predeclare_class` does bounded category/delimiter lookahead; the new suffix boundary does not parse bodies again. |
| Canonical facts (§2) | Types, template arguments, declaration/scope IDs and queries are compact keys. Source/context occurrences project the retained graph, not cloned syntax trees. `no_return`, builtin intrinsic IDs, selected conversions and vector storage extents are typed facts, never reconstructed through names in lowering. |
| Lookup/access (§3) | Indexed lexical/base/friend edges restrict lookup. Inherited signature aliases use actual member lookup. Dependent protected privilege remains a source-context obligation and is rechecked after substitution; fixed current-instantiation friendship cannot grant other specializations access. New bound-query rejection is a compact conversion/fact result. |
| Demand (§4) | Class specialization uses `(pattern, interned arguments)` and a retained environment; completion observes monotonic states. Nested template bodies remain in the enclosing completion interval. Materialized member aliases retain provenance without staying spuriously dependent. Selected aggregate children demand destructors; inactive alternatives and scalar-new root destruction stay distinct. The existing integer-sequence cap remains 1,048,576. |
| Scheduling/caches (§5) | Existing declaration/body/default queues and explicit query reverse dependencies own demand. Completion invalidates only consumers of the changed class/query. No global retries, epoch-wide cache clearing, grammar replay or new process-global state were added. Capture identity is `(closure, object)`; default boundaries remain explicit. |
| Typed lowering (§6) | `build_program` passes its in-memory `Program` directly through native preparation and `compile_object` to `HostElf`. Construction, cleanup, conversions and call boundary modes are consumed from selected facts. The vector cast uses typed equal-size byte transfer; zero storage uses an aggregate byte span. Text readers/writers remain inspection adapters. |
| Work and lifetime (§5,8,9) | TU source/semantic slabs and flat ID indexes die after TU lowering. Candidate buffers and the new flow arrays/worklist have local owners. The proof uses only this function's instruction/value/slot/block ranges; at most two fact transitions notify each indexed use. It is O(instructions + operands + slots + CFG edges), with no IR growth, full-program scan or per-node owning allocation. Native MIR/selection scratch is released per function. |

The combined trace records **700 tokens**, maximum cursor lookahead **71**, and
**308 LowIR instructions**. Its one fallthrough proof visits 87 instructions
and 43 edges/operands. These counters corroborate the source ownership review;
they are not substitutes for it. [51 trace commands](../student.tests/pa30/evidence202/trace.json)
retain LowIR validation/roundtrips, direct and rebuilt objects, successful linked
execution, symbols, disassembly and unwind frames. Telemetry on/off objects match.

## Optimization, emitted code and performance

The [20-command inspection](../student.tests/pa30/evidence202/optimization-trace.json)
traces the dependent type in `combine<int>::apply` into its selected forced-inline
callee, legality/admission, prepared IR, consumed MIR and ELF. Eligibility rejects
unsupported frame semantics and recursion; budget refusal retains a valid call.
Limits remain depth 64, 262144 reserved work per caller and 4194304 per program.
The trace performs one expansion (17 actual units, 33 reserved), produces 64
prepared instructions and 420 standalone text bytes, and preserves the volatile
operations, real noinline call and CFI. Ordinary O0 loop storage/spill costs are
visible. The constant-flow proof changes no code and needs no runtime-profit
hypothesis; no optional optimization was introduced.

[Performance202](performance202.md) applies the spec's stage-scoped protocol:
576 raw observations, 32 launchers, frozen images/inputs/flags, A/A calibration,
six ABBA blocks, separate checked runtime and compiler latency/RSS, and text size.
The full-range common and conditional-owner A/B images are byte-identical.
Owner counters are exactly N functions, 119N instruction visits and 58N
edge/operand visits. Corrected bounds have measured final-only cost, because
entry rejects the valid inputs. Supplemental equivalent heavy-header A/B ratios
are 0.986 (fstream) and 1.011 (regex), with all outliers retained.
Final hosted measurements remain at most **5.390 s / 177028 KiB**.

All 27 independently rebuilt objects traced by implementations199–201 also match
their historical bytes, including vector, cleanup, template and exception cases.
[Image bindings](../student.tests/pa30/evidence202/historical-images.json) support
reuse of those applicable runtime/text observations. Unsupported historical 15%
latency and zero-growth targets remain diagnostics, not extra gates. The mandated
45-second compile limit, correctness, coverage and inherited expansion budgets
are unchanged. PA31–34 obligations are not invented PA30 exit gates.

## References, acceptance and remaining work

The only protected-contract change across the review range is the previously
committed replacement-new `.ref.exit_status`. [Proof201](reference-correction201.md)
provides both renamed/header-faithful reducers, the actual preprocessed unrestricted
declaration, C++11 [except.spec]/3–4 and the pinned bundle revision/hash. A later
restricted declaration is incompatible with that earlier declaration; the library's
allocation-failure behavior does not exempt redeclarations. The positive matching
replacement-new and throwing/catching controls still pass. No further reference
change was needed by this audit. All fixture sources, discovery, required behavior,
timeouts and comparison rules remain unchanged.

[Final validation](../student.tests/pa30/evidence202/validation.json) retains exact
commands, output, status and hashes. The required prior-through command passes
**4941/4941**; file audit passes with its four inherited substantial-header
warnings. `make test-pa30` returns 2 with **151/153**, and through30 reports
**5092/5094**. The same two random fixture identities fail at
`__builtin_ia32_packsswb`; no new failure, removed case or compensating pass is
used. Entry's supplied 154-case summary is preserved and reconciled to the
primary log and actual 153-case inventory.

All **356 inherited** and **35 new** explicit control commands pass, along with
51 source/adapter traces and 20 LowIR/MIR/ELF inspections. The final
[verifier](../student.tests/pa30/verify202.py) passes **1506 checks**, including
historical and current bindings, all case identities, reference protection,
raw observations and both review markers.

Remaining implementation is grouped in [the compact plan](plan.md): complete
typed packed SIMD and vector lvalue/storage behavior, then finish the full-stage
report. The two random cases and the inherited valid vector-subscript reducer
remain required work. The three handoffs grouped useful semantic owners, but
separated array-bound legality, control-flow joins and their downstream checks
from the initial fixes. That avoidable fragmentation left these two owner defects
for audit. Future vector work should close lane, conversion, volatility and
storage interactions together before recording a handoff.

## Audit ledger

| Audit | Reviewed range / code tip | Findings and disposition | Checks / remaining |
|---|---|---|---|
| 198 | `27029f9..c0b26910`, through `4a081cb0` | Base typedef identity and LowIR role presentation repaired; reference proof and stage-scoped performance reviewed. | Prior 4941/4941; file audit pass; PA30 132/153 with unchanged 21 failures. Historical record at `d9a62584`. |
| 202 | `4a081cb0..56ecc31c`, through `378d1bd8` | Converted array bounds and constant-join flow proof repaired; full accumulated ownership, reference proof and performance reviewed. | Prior 4941/4941; file audit pass; PA30 151/153, identical two failures/153 cases; 391 controls, 71 trace/inspection commands; SIMD/vector completion remains required. |
