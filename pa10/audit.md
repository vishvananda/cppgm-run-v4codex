# PA10 independent final audit

Audit starts at `a5e10cfd`, after the complete PA10 commit sequence rooted at
`c2a4786e`. The source, handout, LowIR contract, spec, commits and original
measurements were read independently. Earlier checkpoint conclusions were leads,
not exit evidence. The plan's old review pointer still named the stage base;
this record closes the handoffs through implementation, reference corrections,
ownership fixes, sequencing fix and all measurement continuations.

Reviewed final compiler implementation: `60f7088f`. The optional candidate
`4fc61de6` was measured and reverted; the final binary exactly matches the
frozen `60f7088f` binary (SHA256 in the final performance report).

## Spec Alignment

PA10 owns procedural C++ to typed LowIR at O0. Source template generation,
class helpers, exception lowering, native MIR/selection/allocation/ELF, optimized
levels and self-hosting remain with PA11–34 as mapped by the handout. Earlier
syntax, type, semantic and ABI tools remain required. Native execution below
uses only the explicitly allowed PA8 reference backend in personal harnesses.

| Spec surfaces | Actual ownership and data path inspected |
|---|---|
| §§1–2 source/parser/identity | `SourceBuffer::bytes` is immutable; `Preprocessor` owns input frames and interned identifiers. `PostTokenCursor` and the syntax ring cursor discard consumed tokens. `Parser::translation_unit` calls `Analyzer::consume` on each completed declaration. One vector-backed source graph carries NodeIds and source locations; semantic type/entity/expression/conversion records attach to it. No copied semantic tree. Source spellings retained for the shared syntax views are interned, not owning token strings. |
| §§2–5 lookup/facts/demand | Semantic `Index` uses compact scope/name keys with flat open addressing; explicit namespace edges restrict visits. Types and adjusted signatures are canonical. Overload candidates are filtered and every required viable candidate is inspected; selected declaration, argument NodeIds, default expressions and conversion ranges are recorded. Defaults retain their declaration scope. Static initializer cache ownership is the sealed TU, keyed by `(NodeId,target TypeId)`, with active/success/failure states. No global invalidation or retry loop. |
| §§3–5 inherited templates | `template_call.cpp` interns argument packs and keys specialization declarations by canonical pattern entity and pack. The pattern owns the fixed template environment; per-substitution bindings/cache are local overlays. Nondependent TypeIds are reused; only dependent type structure is transformed. Declaration success/failure and emission demand are separate monotonic facts. `finish()` consumes a deduplicated member-demand queue. No template body generation is claimed for PA10. |
| §6 typed lowering | `Procedural` reads semantic facts, creates PA8 operands/instructions through `FunctionBuilder`, and uses PA9 typed targets directly. No ABI fact reader, LowIR reader, host compiler or reference process is in this call path or its linked source lists. Constructors check local instruction shape; `--validate-lowir` performs one optional whole-program audit. Text is written only as the requested output. |
| §§2,6,8 linkage and release | Per-TU maps translate semantic identities to program SymbolIds. A program-owned typed ABI graph/index joins externally linked declarations across source files; C linkage joins across namespaces too. Internal identities and object labels remain distinct. A declaration is upgraded to its definition once; repeated inline definitions reuse the emitted entity. The external index is unnecessary for a single C++ TU and is skipped there. Manglings are display/export data, never lookup keys. |
| §§7–9 work/storage | Nodes, facts, IR records and operand slices use geometric contiguous pools, without owning children or recursive destruction. A function builder dies after its body; live call scratch is popped after each call and its capacity is reused. Frontend/semantic/local mapping arenas die after each TU. The shared ABI/linkage summaries and Program survive until explicit LowIR output completes. No global mutable cache. Statement entry flags and discarded-access memoization are dense NodeId facts, bounded by the parsed graph. |
| §§9–10 evidence/self-containment | Phase/work telemetry observes existing work, including semantic candidates, constants, demand, source cursor, control traversal, static cache, linkage and IR pools. Timing runs omit telemetry and audit validation; separate telemetry runs check work. Reference tools only validate/execute compiler-produced output in harnesses. No fixtures, answer tables, host compilation or cached output implement required behavior. |

## Representative traces

A namespace function taking `int&` and an `int` default follows interned source
names → canonical Function TypeId/EntityId → selected call Fact and recorded
reference/default conversions → PA9 function target and one SymbolId → `ptr
[pass=by_address, object_bytes=4]` signature. Calling it on `a[1]` creates a typed
`index i32 [projection=array_element]`; the existing address is passed once.
The callee's reference slot stores a pointer; subsequent lvalue accesses load
that pointer then load/store the referred i32. Reference returns preserve the
address. `control-defaults`, `nested-calls`, `return-references` and the new
multi-file test check these paths through native execution.

`const int&r0=value; const int&r1=r0;` follows resolved initializer NodeIds
through one cached address fact per complete target key to structured `DataItem`
relocations containing SymbolId/addend. No startup function or name-based search
is needed. Array addresses now handle both operand orientations and reference
indirection; scalar conversions are applied after every static-value path.

For inherited template demand, repeated `consume(1)` selects the same parsed
`template<class T> void consume(T)` declaration, interns the `int` argument pack,
substitutes its dependent function parameter once, records declaration success,
and reuses that specialization. Nondependent `void` is shared. This trace ends
at the PA7 semantic declaration/demand boundary, not a claimed template ELF.
The fixed template semantic benchmark verifies that boundary at two sizes.

For control flow, a semantic goto records its target NodeId after checking
initialization-prefix ancestry. Lowering maps it to one BlockId. Entry-path
marking preserves labels inside otherwise unreachable if/loop/switch regions;
only their unreachable headers get fresh disconnected blocks. A bypassed
uninitialized scalar obtains its slot by EntityId when first referenced.
Initialized condition bindings extend the semantic prefix and forbid entry
from outside. Native reducers include nested labels, switch cases and loops.

## Findings and fixes

- Nested labels disappeared after a terminator because lowering returned before
  visiting their enclosing statement. Statement-only entry marking plus normal
  structured lowering fixes goto and switch dispatch across the full path.
  Related uninitialized local storage and condition-initialization bypasses are
  fixed at their lowering and semantic owners respectively.
- `bool b=0.5` produced false, and early address/conditional returns skipped
  conversions. Static fact publication now applies conversions consistently;
  floating-to-bool compares with zero directly. `1+array`, `&1[array]` and
  `*(array+1)` reference bindings retain typed addresses and addends.
- A cached assignment/increment result suppressed required volatile readback.
  Cache reuse now requires a nonvolatile value. Discarded conditional glvalues
  read only when both arms qualify; prvalue arms retain their conversions.
  The form predicate memoizes positive and negative results by NodeId, avoiding
  repeated subtree scans. `volatile-value-contexts` checks seven necessary
  volatile loads and execution results.
- Multiple input TUs formerly gave a declaration and definition different IR
  symbols, leaving native calls unresolved. Typed program linkage now coalesces
  them, updates declaration records, and emits each body once. Same-name static
  objects/functions and reference temporaries remain isolated. Global suffix
  allocation checks collisions on source and generated names with a monotonic
  disambiguator, making total collision probes linear in encountered symbols.
- Internal object labels need disambiguation when several TUs become one output
  program. Redundant `object=source_name` is omitted when the ordinary LowIR
  name already supplies it, avoiding duplicate labels in the supplied backend.
  C exports retain their exact spelling; the focused course controls still
  locate source-named functions. All comparison rules remain unchanged.
- The indirect-call probe initially assumed callee-before-arguments. C++11 does
  not require that order; the course's function-pointer fixtures explicitly
  fetch the callee after argument evaluation. That convention is preserved and
  independently tested with observable calls. No reference correction was made.

Language proof is the checked-in [C++11 draft](../doc/n3485.txt): §4.12
[conv.bool]/1 (zero versus every nonzero value); §4.1 [conv.lval]/2 and §5.17
[expr.ass]/1 (an assignment yields an lvalue whose value conversion accesses the
object); §5 [expr]/11 (discarded volatile forms, including both conditional
arms); §5.2.1 [expr.sub]/1 and §5.7 [expr.add]/4–6 (subscript and pointer
arithmetic); §6.6.4 [stmt.goto]/1 and §6.7 [stmt.dcl]/3 (label transfer and
initialization barriers); §3.5 [basic.link] and §7.5 [dcl.link] (program identity,
internal isolation and C linkage). PA8's top-level SymbolId and `object`
contracts govern the direct representation.

The two inherited reference corrections at `55b33a44` were independently
rechecked against those source fixtures, the pinned bundle and §3.6.2/§5.19.
Both dynamic-order reducers still expose the pinned binary's incorrect result.
[Proof and bundle revision](reference-corrections.md) remain intact; this audit
changes no course inputs, sidecars, comparison rules or reference payloads.

## Optimization legality and budgets

There are no optional optimization passes or O1/O2/O3 policies in PA10. The
pointer-difference reduction consumes semantic pointee size and the language's
same-array exact-divisibility precondition. Positive powers of two use arithmetic
right shift after subtraction; other sizes retain signed division. Invalid
pointer differences do not license inventing alias/range facts. Both negative
power-of-two and size-three differences are tested. The native handoff consumes
these actual LowIR operations; runtime gains are not inferred from node counts.
The standalone [trace](../student.tests/pa10/lowering-trace.cpp) compiles with
`--validate-lowir`, executes with exit 0, and was inspected through native
disassembly. The four-byte case encodes subtract plus `sar 2`; the size-three
case retains signed LowIR division and the supplied backend encodes a reciprocal
multiply plus signed correction. The pointer routines use register operands
and 16-byte frames; main uses a 96-byte frame. These backend choices are observed
handoff facts, not implemented optimizations or new allocator exit gates.

Immediate widening and nonvolatile cached assignment values require exact value
preservation. Volatile accesses always retain their required effects. Branches
preserve source sense and short-circuit conditions avoid a boolean result slot.
These are local construction decisions with constant work/growth per consumed
node, no fixed point or invalidation pass, and conservative operations when the
proof is absent. New control traversal and discarded-access facts are O(nodes);
static facts and linkage use expected O(1) lookup. The entire ordinary pipeline
remains O(consumed/produced work), with O(n log n) allowances only where already
documented for semantic ancestry. Explicit array element stores count as
produced IR. There is no new unbounded search, cloning, inlining or unrolling.

Register allocation, spills, MIR/debug encoding, native floating/variadic parity
and ELF ownership are later-stage limits. The benchmark harness still checks
native loops, calls, memory and floating code now; those constraints are not
used to excuse an avoidable PA10 frontend regression.

## Validation and performance ledger

- `make test-pa10`: 121/121 and 5/5 focused controls, exit 0.
- `make test-report-through-pa10`: all ten stages, 1025/1025 oracle tests,
  plus the separately reported PA10 controls, exit 0.
- `perl scripts/cppgm_file_audit.pl --stage pa10 --paths dev/src`: 126 files,
  exit 0. `git diff --check`: clean.
- `check.py`: 14 native programs and nine semantic rejections; `check_multifile.py`:
  both source input orders; `check_ir.py`: 123 successful source/control inputs.
  All pass normally and under the actual driver built with ASan/UBSan,
  `detect_leaks=1` and `halt_on_error=1`.
- `check_reference_corrections.py`: both pinned-reference order reducers still
  return the documented incorrect 1 instead of required 0. No new sidecar edits.
- [Linkage growth](../student.tests/pa10/linkage-growth.json): 4/16 TUs with 512
  repeated external declarations each produce 512 declarations and one main.
  Requests/hits are 2049/1536 and 8193/7680. ABI graph stays at 1026 nodes and
  154166 bytes; IR capacity remains 131200 bytes. Work tracks input declarations,
  retained linkage storage tracks unique entities. This is structural evidence;
  its diagnostic phase times are not a performance comparison.
- [Final performance](final-audit-performance.md): two complete frozen A/A/ABBA
  campaigns, each with 108 compiler, eight startup and 36 native observations.
  All generated executable pairs are byte-identical. The optional display-check
  change saved 640 text bytes but slowed the large floating/memory workload
  2.1–5.0% beyond its 1.74% A/A spread; it was reverted. All data is retained.
  The final compiler matches the already measured binary byte for byte.
- Completion: no remaining PA10 defect or unaudited handoff was found in the
  reviewed contract/architecture paths. Required exit checks pass, the plan and
  audit are consolidated, and final changes are committed with a clean tree.

The supplied Ralph primary log and direct root output count 1025 oracle tests;
the user-facing status says 1049. That display was not used as evidence. Record
the authoritative command output and separately named controls, without
inferring the reason for the discrepancy or changing test discovery. Historical performance observations and the
original stage ledger remain preserved in [performance.md](performance.md), with final acceptance
and the rejected-candidate ledger in [final-audit-performance.md](final-audit-performance.md).
