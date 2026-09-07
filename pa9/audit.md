# PA9 independent final audit

Scope: **PA9 full-stage**, reconstructed from `spec.md`, the complete handout,
`TESTING_AND_REFERENCES.md`, the local Itanium grammar, every stage commit
`7442a3192`–`6ec119239`, and the implementation (not checkpoint conclusions).
Final implementation: `5aad60656` (ownership repairs `ad9c2c907`). The audit also covers the previously
unaudited serializer, telemetry, performance and documentation handoffs.

## Final Spec Alignment

| Spec surface | Actual PA9 architecture and evidence |
| --- | --- |
| §1 source / phase boundary | CLI reads each explicit fact record once, interns binders/spellings, and constructs graph nodes immediately. Its per-line words are an input adapter, not a production token pipeline or retained syntax tree. `parse_fact_text` is a separate inspection API. No C++ grammar is parsed here. |
| §2 identity | `Graph` owns compact IDs, immutable canonical nodes and child slices. Hash keys contain kind, typed fields, integral bits and ordered child IDs. Values, cv/function qualifiers, tags and function template shapes canonicalize at the shared construction boundary. Text never keys semantic equality. |
| §§3–5 lookup, demand, caches | Dense case-local binders index interned IDs. Graph interning and symbol-local substitutions use flat tables at ≤1/2 occupancy. Encoding visits demanded facts only; 100k unrelated nodes do not change visited-node counts. Published facts never mutate, so graph insertion invalidates no existing identity or substitution. C++ overloads, instantiation, completion and scheduling are not PA9 operations. |
| §6 typed handoff / validity | Future callers construct `Graph`/`Target` and call `mangle` directly. Immediate edge kind/range, spelling, qualifier and function-record checks run before graph publication; backward edges ensure a DAG. No whole-graph validator or serialized intermediate is inserted into ordinary encoding. |
| §7 optimization / encoding | The target is an ABI name, not executable IR. Substitution is mandatory ABI grammar with ordered canonical keys, not a profitability heuristic. No O-levels, machine selection, allocator, spills, loops, ABI lowering or ELF writer exist at this milestone. |
| §8 ownership | Identifier bytes, nodes, hashes and edges are geometrically grown pools; no published node owns a heap child/vector/shared pointer. Per-record words, construction vectors and per-symbol scratch have shorter lifetimes. CLI resets graph/binders per case and checks output close errors. |
| §9 complexity / evidence | Construction and validation track bytes, facts and immediate edges; encoding tracks consumed facts and output. Tags have local O(tags log tags) ordering. Frozen whole-stage and checkpoint A/B runs measure compiler wall/RSS/text separately; executable runtime/text is N/A. See `performance.md`. |
| §10 self-containment | Inspected entry point, source lists and all ABI implementation files: only explicit input files are opened; no reference/host compiler, object inspection, subprocess, answer cache or fixture dispatch supplies names. Host `size` in the benchmark measures the compiler binary only. |

## Reconstructed ownership and representative traces

`abimangle.cpp` owns invocation/output. `parse_fact_stream` reads one line,
`FactReader` resolves typed binders and constructs canonical `Graph` nodes,
and the completed `Target` calls `mangle`. `Encoder` writes one append-only
symbol. At each case boundary the target, graph and substitutions release; binders
clear and retain at most the largest case's capacity until the file ends. The
line buffer likewise retains its high-water capacity. Output and optional
aggregate counters remain across cases.
Whole-file parsing/serialization deliberately retain graph/cases as explicit
inspection operations and are never called from this streaming path.

For `ns::Box<int>`, the graph contains `Name(ns)`, `Name(Box,parent=ns)`,
`Builtin(int)`, `TypeArgument(int)` and `Template(Box,[argument])`. Repeated
parameters point to the same ID. The encoder enters namespace and template
prefixes before arguments, enters the completed specialization after arguments,
and emits later references by its numbered substitution. Builtins and standard
substitutions acquire no ordinary slot. Scope/name spellings are borrowed
from the identifier arena during emission; they are never reparsed.

The nontrivial demanded-template trace is the checked
`600-function-template-local-class-arg.t`: function template `g` has a type
argument `Local(context=FunctionEntity(f), source=X, ordinal=0)` and result
`int`. The shared function shape canonicalizes both inline template-name and
separate argument-list forms. Encoding produces `_Z1gIZ1fvE1XEiv`: enter `g`'s
prefix, enter `I`, encode `Z` + enclosing function `f` + `E`, encode `X`, close
template arguments, then result `i` and empty parameters `v`. The local context
uses the enclosing symbol's substitution sequence. This traces the available
ABI demand; C++ template parsing/instantiation and source-to-ELF are later PAs.

For `500-equivalent-integral-decltype-substitution.t`, both member expressions
retain the same parameter type ID, source ID `value`, literal `1`, and binary
`pl` operation. Their `Decltype` IDs coincide, so the second result argument
uses the completed type's substitution. Changing the literal or a type-trait
operand produces a distinct ID, as the neighboring distinction fixtures check.
The audit's unsigned expression probe additionally proves that `uint -1` and
`uint 4294967295` converge *before* hashing, emitting
`_Z1fDTLj4294967295EES_` rather than two equivalent spelled-out types.

A const, lvalue-ref-qualified `int()` type is one `FunctionType(result=int,
qualifiers=5)` fact. It emits `KFivRE`: cv before `F`, ref before `E`. Its
unqualified counterpart has a separate identity; it cannot reuse a substitution
invented while encoding the qualified type. The direct API and text roundtrip
both exercise that invariant. This is required ABI fidelity, not an executable
optimization or a claim that shorter names make programs run faster.

External entity literals retain an independent substitution table, but append
to the same destination string and inherit the caller's recursion depth.
Nested external names therefore no longer copy every completed inner symbol at
each enclosing level. Local contexts intentionally retain the outer table.
Covariant thunks retain this/fixed-result/virtual-result-slot adjustments;
virtual-base thunks retain both fixed this and vcall offsets. No ABI decision
is reconstructed from mangled spelling.

## Findings and repairs

| Finding | Owning repair and independent check |
| --- | --- |
| F1: value canonicalization existed only in argument parsing; equivalent expression/direct values produced different substitutions | Move unsigned-width/bool normalization into `Graph::make`. Remove adapter duplication. Exact expression output, direct value identity and serializer checks cover both paths. |
| F2: direct cv wrappers and function types could disagree on identity; function ref qualifiers emitted in the wrong position | Canonicalize cv into the function type fact and emit indivisible cv/ref function types according to local ABI §5.1.5.3. Check qualified/unqualified repeated parameters and full qualifier roundtrips. |
| F3: direct producers could publish wrong-kind, forward/cyclic or invalid string links | Validate immediate fields before publication. Direct API probes require exceptions for invalid IDs, expression/type confusion, function child counts and conflicting qualifiers. Invalid links fail before encoding or serialization. |
| F4: vendor and other type constructors, enclosing contexts and isolated external names bypassed nesting guards | Universal type-entry guard, guarded function entry and inherited external depth; iterative prefix lookahead. Before: 100k vendors terminated by SIGSEGV. After: 1500-level probes across eight grammar routes exit 1 without signals/timeouts, also under ASan/UBSan. Existing 20k compact modifier success remains. |
| F5: nested external encoders owned/copied whole inner strings | Share the append destination while isolating only substitutions. Each output byte is appended at its final destination. Preserve all entity/context fixture results and bound the recursive path. |
| F6: large local discriminators and tagged function terminal shapes encoded incorrectly | Emit `__number_` at number ≥10. Share function template/tag decomposition between interning, encoding and serialization; canonicalize function tags once. Probe ordinals 10/11/uint64-max and equal function facts built by different API shapes. |
| F7: inspection roundtrip lost function qualifiers, template-parameter substitution eligibility, standalone function contexts and fixed virtual-thunk adjustment | Extend normalized typed adapter forms and preserve all fields. Direct graph-to-text-to-graph assertions verify names, including `KFivRE` and `_ZTvn8_n32_N1C1fEv`. No raw-name recovery added. |
| F8: completed specializations could be reused as template prefixes, allowing unbounded recursive tag normalization | Reject the invalid prefix at publication; a member template retains a proper `Name(owner=specialization)` edge. Before: a 100k chain terminated by SIGSEGV. The reduced adapter/direct tests now reject immediately. All preliminary timings are retained and repeated for the corrected binary. |

Shared tag sorting also removes duplicate comparator implementations and
borrows interned bytes. This consolidates the owner and keeps compiler text
within the pre-existing budget. No correctness fixture, reference, harness,
earlier stage source or inherited attribution was changed.

## Work, validity and growth budgets

- New-node validation checks only immediate edges and fixed metadata. Hashing
  checks the same typed key; neither operation scans unrelated graph contents.
  Canonical rewrites are local and bounded (merge cv/tags, normalize literal,
  decompose one function-template terminal). No fixed point or global retry.
- Graph tables grow geometrically; each symbol's sparse substitution table
  scales with entered candidates, not translation-unit size. IDs remain stable
  on pool growth; temporary references/views are reacquired after mutation.
- Common modifier chains use iterative storage. All recursive grammar edges
  share the 1024 nesting budget, including external substitution islands. On
  exhaustion, reject the name rather than silently changing its semantics.
- Encoding is proportional to facts actually expanded and bytes required by
  the grammar; duplicated expression occurrences may require duplicated output.
  External-name emission no longer adds repeated enclosing-string copies.
- No instruction transform, generated-code growth, analysis cache invalidation
  or optimizer fallback applies. Substitution legality follows ABI identity and
  insertion order; no profitability choice may skip it. Runtime/text and
  spill/loop costs for generated executables remain explicitly N/A.
- Compiler budgets remain wall ≤1.25x A, RSS ≤1.20x A +16 MiB, fourfold wall
  ≤5.5x/work ≤4.5x/RSS ≤5x, and total text growth ≤100 KiB versus first-correct.
  Two independent frozen comparisons check whole-stage and audit-delta costs.

## Validation and handoff

All 117 checked-in status/output cases and successful serializer roundtrips,
11 original valid/15 invalid probes, eight final exact/roundtrip probes, eleven
controlled-rejection probes, direct graph invariants and file/case ordering
pass. The same complete API/personal suite passes ASan/UBSan with leak checks.
The direct tests retain the unrelated-100k and iterative-20k checks.

Final-source `make test-pa9` and `make test-report-through-pa9` pass; the
required file audit passes with 114 files and no warnings. Both final frozen
experiments pass all 42 checks; their provenance, raw observations and all
calculations independently verify. The compact plan and performance report
record the detailed results. The incoming 928/928 claim is unsupported
by the actual harness: the checked saved log and fresh cumulative report both
count **904/904**, comprising nine passing stages including **111/111 PA9**.
The six additional root ABI fixtures are covered by the explicit 117-case suite.

PA10 must lower resolved declarations, member/nonmember shape, linkage, template
arguments and ABI ordinals directly into these facts; it must not serialize and
reparse them. Normalized lambda ordinals retain the handout's explicit ABI
number convention (its zero spells `E0_`); source-language ordinal assignment
belongs to that later producer. No PA9 handoff remains unaudited or unfinished.
